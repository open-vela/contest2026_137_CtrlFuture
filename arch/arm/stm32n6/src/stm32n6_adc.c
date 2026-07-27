/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_adc.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version
 * 2.0 (the "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 *
 ****************************************************************************/

/* ADR-029: ADC driver for the STM32N647 on-chip internal channels.
 *
 * This delivers the ADC function of ADR-029 through the NuttX ADC
 * character framework (/dev/adc0).  It targets the chip's internal analog
 * sources that need no external wiring -- specifically the internal voltage
 * reference (VREFINT, channel 17) -- so the cmocka drivertest_adc suite can
 * exercise a real conversion on hardware.
 *
 * Conversions are software-triggered and read back by polling: ANIOC_TRIGGER
 * starts one regular conversion, the handler busy-polls EOC, reads the data
 * register and hands the sample to the upper half.  Because the whole
 * conversion completes synchronously inside the ioctl (no blocking wait, so
 * the core never enters WFI mid-conversion), the ADC needs neither an EOC
 * interrupt nor the sleep-clock (LPEN) handling that the timers require.
 *
 * The ADC kernel clock (RCC ADC12SEL) is left at its reset default of HCLK,
 * which is already running on this board, so no RCC clock-mux setup is
 * needed -- only the AHB1 peripheral clock gate is opened.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>
#include <nuttx/spinlock.h>

#include "arm_internal.h"
#include "chip.h"
#include "stm32n6_adc.h"
#include "hardware/stm32_adc.h"
#include "hardware/stm32_rcc.h"
#include "hardware/stm32_pwr.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Polling budgets (microseconds).  A single internal-channel conversion at
 * the longest sample time is ~26 us with the ADC on a 32 MHz HCLK, so these
 * give a wide margin while still guaranteeing forward progress (never a
 * dead wait -- Embedded Programming Rule 2).
 */

#define ADC_TIMEOUT_US       10000  /* Regulator/calibration/ready/EOC */
#define ADC_REGUL_STAB_US    10     /* Voltage-regulator stabilization */
#define ADC_VREFINT_STAB_US  12     /* VREFINT internal-path stabilization */

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_adc_s
{
  const struct adc_callback_s *cb;      /* Upper-half receive callback */
  uint32_t                     base;    /* ADC instance register base */
  uint32_t                     cmnbase; /* ADC common-block register base */
  uint8_t                      channel; /* Single regular channel to sample */
  bool                         ready;   /* True once the ADC is enabled */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int  stm32n6_adc_bind(struct adc_dev_s *dev,
                             const struct adc_callback_s *callback);
static void stm32n6_adc_reset(struct adc_dev_s *dev);
static int  stm32n6_adc_setup(struct adc_dev_s *dev);
static void stm32n6_adc_shutdown(struct adc_dev_s *dev);
static void stm32n6_adc_rxint(struct adc_dev_s *dev, bool enable);
static int  stm32n6_adc_ioctl(struct adc_dev_s *dev, int cmd,
                              unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct adc_ops_s g_stm32n6_adc_ops =
{
  .ao_bind     = stm32n6_adc_bind,
  .ao_reset    = stm32n6_adc_reset,
  .ao_setup    = stm32n6_adc_setup,
  .ao_shutdown = stm32n6_adc_shutdown,
  .ao_rxint    = stm32n6_adc_rxint,
  .ao_ioctl    = stm32n6_adc_ioctl,
};

static struct stm32n6_adc_s g_adc1priv =
{
  .base    = STM32_ADC1_BASE,
  .cmnbase = STM32_ADC12_COMMON_BASE,
  .channel = ADC_CHANNEL_VREFINT,
};

static struct adc_dev_s g_adc1dev =
{
  .ad_ops  = &g_stm32n6_adc_ops,
  .ad_priv = &g_adc1priv,
};

static spinlock_t g_adc_lock = SP_UNLOCKED;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_adc_enableclk
 *
 * Description:
 *   Open the AHB1 peripheral clock gate feeding ADC1/ADC2.  Without this a
 *   register write is silently dropped and reads return zero.
 *
 ****************************************************************************/

static void stm32n6_adc_enableclk(void)
{
  irqstate_t flags;
  uint32_t regval;

  flags = spin_lock_irqsave(&g_adc_lock);

  regval  = getreg32(STM32_RCC_AHB1ENR);
  regval |= RCC_AHB1ENR_ADC12EN;
  putreg32(regval, STM32_RCC_AHB1ENR);

  spin_unlock_irqrestore(&g_adc_lock, flags);
}

/****************************************************************************
 * Name: stm32n6_adc_wait_set
 *
 * Description:
 *   Poll a status-register bit until the hardware sets it, bounded by
 *   ADC_TIMEOUT_US.  Used for the ready (ADRDY) and EOC handshakes.
 *
 ****************************************************************************/

static int stm32n6_adc_wait_set(uint32_t reg, uint32_t bit)
{
  int i;

  for (i = 0; i < ADC_TIMEOUT_US; i++)
    {
      if ((getreg32(reg) & bit) != 0)
        {
          return OK;
        }

      up_udelay(1);
    }

  return -ETIMEDOUT;
}

/****************************************************************************
 * Name: stm32n6_adc_bind
 ****************************************************************************/

static int stm32n6_adc_bind(struct adc_dev_s *dev,
                            const struct adc_callback_s *callback)
{
  struct stm32n6_adc_s *priv = (struct stm32n6_adc_s *)dev->ad_priv;

  DEBUGASSERT(priv != NULL);
  priv->cb = callback;
  return OK;
}

/****************************************************************************
 * Name: stm32n6_adc_reset
 *
 * Description:
 *   Return the ADC to a known idle state: stop any conversion, disable the
 *   ADC and put it back into deep power-down.
 *
 ****************************************************************************/

static void stm32n6_adc_reset(struct adc_dev_s *dev)
{
  struct stm32n6_adc_s *priv = (struct stm32n6_adc_s *)dev->ad_priv;
  irqstate_t flags;
  uint32_t cr;

  DEBUGASSERT(priv != NULL);

  flags = spin_lock_irqsave(&g_adc_lock);

  cr = getreg32(priv->base + STM32_ADC_CR_OFFSET);
  if ((cr & ADC_CR_ADEN) != 0)
    {
      /* Request disable and re-enter deep power-down */

      putreg32(ADC_CR_ADDIS, priv->base + STM32_ADC_CR_OFFSET);
    }

  putreg32(ADC_CR_DEEPPWD, priv->base + STM32_ADC_CR_OFFSET);
  priv->ready = false;

  spin_unlock_irqrestore(&g_adc_lock, flags);
}

/****************************************************************************
 * Name: stm32n6_adc_setup
 *
 * Description:
 *   Power up, calibrate and enable the ADC, then configure a one-channel
 *   regular sequence sampling the internal VREFINT source.  Called the
 *   first time /dev/adc0 is opened.
 *
 ****************************************************************************/

static int stm32n6_adc_setup(struct adc_dev_s *dev)
{
  struct stm32n6_adc_s *priv = (struct stm32n6_adc_s *)dev->ad_priv;
  irqstate_t flags;
  uint32_t regval;
  int ret;

  DEBUGASSERT(priv != NULL);

  if (priv->ready)
    {
      return OK;
    }

  stm32n6_adc_enableclk();

  flags = spin_lock_irqsave(&g_adc_lock);

  /* Close the VDDA18ADC analog-supply switch (SVMCR3.ASV).  Without a valid
   * analog supply the ADC still runs and end-of-conversion still asserts,
   * but every conversion reads back exactly zero -- so this must be enabled
   * before powering up the ADC.
   */

  regval  = getreg32(STM32_PWR_SVMCR3);
  regval |= PWR_SVMCR3_ASV;
  putreg32(regval, STM32_PWR_SVMCR3);

  /* Exit deep power-down.  On the STM32N6 clearing DEEPPWD is what enables
   * the ADC voltage regulator; there is no separate ADVREGEN bit.  Give the
   * regulator its stabilization time before enabling the ADC.
   */

  regval  = getreg32(priv->base + STM32_ADC_CR_OFFSET);
  regval &= ~ADC_CR_DEEPPWD;
  putreg32(regval, priv->base + STM32_ADC_CR_OFFSET);

  up_udelay(ADC_REGUL_STAB_US);

  /* No calibration is performed.  Unlike the STM32H7/F3 "set ADCAL with
   * ADEN=0 and wait for self-clear" flow, the STM32N6 calibrates via an
   * averaged offset measurement that runs with the ADC already enabled; a
   * blind ADCAL here would never clear.  Calibration only trims a static
   * offset, which is irrelevant to reading VREFINT and to the drivertest
   * (it only needs a live, jittering conversion), so it is skipped.
   *
   * Enable the ADC: clear a stale ADRDY, then set ADEN and wait for ADRDY.
   * ADEN is re-asserted on each poll: hardware may clear it if it is set
   * too soon after power-up, and re-writing it is harmless once ready.
   */

  putreg32(ADC_ISR_ADRDY, priv->base + STM32_ADC_ISR_OFFSET);

  regval  = getreg32(priv->base + STM32_ADC_CR_OFFSET);
  regval |= ADC_CR_ADEN;
  putreg32(regval, priv->base + STM32_ADC_CR_OFFSET);

  ret = -ETIMEDOUT;
    {
      int i;
      for (i = 0; i < ADC_TIMEOUT_US; i++)
        {
          if ((getreg32(priv->base + STM32_ADC_ISR_OFFSET) &
               ADC_ISR_ADRDY) != 0)
            {
              ret = OK;
              break;
            }

          if ((getreg32(priv->base + STM32_ADC_CR_OFFSET) &
               ADC_CR_ADEN) == 0)
            {
              modifyreg32(priv->base + STM32_ADC_CR_OFFSET, 0, ADC_CR_ADEN);
            }

          up_udelay(1);
        }
    }

  if (ret < 0)
    {
      spin_unlock_irqrestore(&g_adc_lock, flags);
      aerr("ERROR: ADC enable (ADRDY) timeout\n");
      return ret;
    }

  putreg32(ADC_ISR_ADRDY, priv->base + STM32_ADC_ISR_OFFSET);

  /* Route the internal VREFINT path to the ADC input mux */

  regval  = getreg32(priv->cmnbase + STM32_ADC_CCR_OFFSET);
  regval |= ADC_CCR_VREFEN;
  putreg32(regval, priv->cmnbase + STM32_ADC_CCR_OFFSET);

  /* Longest sample time for the slow internal reference (channel 17) */

  regval  = getreg32(priv->base + STM32_ADC_SMPR2_OFFSET);
  regval &= ~ADC_SMPR2_SMP17_MASK;
  regval |= (ADC_SMP_MAX << ADC_SMPR2_SMP17_SHIFT);
  putreg32(regval, priv->base + STM32_ADC_SMPR2_OFFSET);

  /* Preselect the channel: without its PCSEL bit the input is not wired to
   * the ADC mux and the data register reads back zero.
   */

  putreg32(ADC_PCSEL_CH(priv->channel),
           priv->base + STM32_ADC_PCSEL_OFFSET);

  /* One-conversion regular sequence: L = 0 (1 conversion), SQ1 = channel */

  putreg32(((uint32_t)priv->channel << ADC_SQR1_SQ1_SHIFT),
           priv->base + STM32_ADC_SQR1_OFFSET);

  priv->ready = true;

  spin_unlock_irqrestore(&g_adc_lock, flags);

  /* Let the internal reference settle before the first conversion */

  up_udelay(ADC_VREFINT_STAB_US);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_adc_shutdown
 ****************************************************************************/

static void stm32n6_adc_shutdown(struct adc_dev_s *dev)
{
  stm32n6_adc_reset(dev);
}

/****************************************************************************
 * Name: stm32n6_adc_rxint
 *
 * Description:
 *   Enable/disable RX interrupts.  This driver is poll-based (no EOC
 *   interrupt is used), so there is nothing to toggle.
 *
 ****************************************************************************/

static void stm32n6_adc_rxint(struct adc_dev_s *dev, bool enable)
{
  UNUSED(dev);
  UNUSED(enable);
}

/****************************************************************************
 * Name: stm32n6_adc_convert
 *
 * Description:
 *   Software-trigger one regular conversion, poll for completion, read the
 *   result and forward it to the upper half.
 *
 ****************************************************************************/

static int stm32n6_adc_convert(struct stm32n6_adc_s *priv)
{
  irqstate_t flags;
  uint32_t data;
  int ret;

  if (!priv->ready)
    {
      return -EAGAIN;
    }

  flags = spin_lock_irqsave(&g_adc_lock);

  /* Start one regular conversion */

  modifyreg32(priv->base + STM32_ADC_CR_OFFSET, 0, ADC_CR_ADSTART);

  /* Poll for end-of-conversion (bounded) */

  ret = stm32n6_adc_wait_set(priv->base + STM32_ADC_ISR_OFFSET, ADC_ISR_EOC);
  if (ret < 0)
    {
      spin_unlock_irqrestore(&g_adc_lock, flags);
      aerr("ERROR: ADC EOC timeout\n");
      return ret;
    }

  /* Reading DR clears EOC */

  data = getreg32(priv->base + STM32_ADC_DR_OFFSET) & ADC_DR_RDATA_MASK;

  spin_unlock_irqrestore(&g_adc_lock, flags);

  if (priv->cb != NULL && priv->cb->au_receive != NULL)
    {
      priv->cb->au_receive(&g_adc1dev, priv->channel, (int32_t)data);
    }

  return OK;
}

/****************************************************************************
 * Name: stm32n6_adc_ioctl
 ****************************************************************************/

static int stm32n6_adc_ioctl(struct adc_dev_s *dev, int cmd,
                             unsigned long arg)
{
  struct stm32n6_adc_s *priv = (struct stm32n6_adc_s *)dev->ad_priv;
  int ret;

  DEBUGASSERT(priv != NULL);

  switch (cmd)
    {
      case ANIOC_TRIGGER:
        ret = stm32n6_adc_convert(priv);
        break;

      case ANIOC_GET_NCHANNELS:
        ret = 1;
        break;

      default:
        ret = -ENOTTY;
        break;
    }

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_adc_initialize
 *
 * Description:
 *   Register an ADC character device backed by the STM32N6 ADC internal
 *   channels.
 *
 * Input Parameters:
 *   devpath - Character device path (e.g. "/dev/adc0").
 *   intf    - ADC peripheral number (currently only 1 is supported).
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int stm32n6_adc_initialize(const char *devpath, int intf)
{
  struct adc_dev_s *dev;
  int ret;

  DEBUGASSERT(devpath != NULL);

  switch (intf)
    {
      case 1:
        dev = &g_adc1dev;
        break;

      default:
        return -ENODEV;
    }

  ret = adc_register(devpath, dev);
  if (ret < 0)
    {
      aerr("ERROR: adc_register(%s) failed: %d\n", devpath, ret);
    }

  return ret;
}
