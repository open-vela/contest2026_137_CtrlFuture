/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_lptim.c
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

/* ADR-028: Low-power timer (LPTIM1) lower-half driver.
 *
 * This delivers the timing / periodic-interrupt function of ADR-028 via
 * the NuttX timer character framework (/dev/timerN).  LPTIM1 is a 16-bit
 * counter clocked here from the LSI (~32 kHz) so it keeps running from a
 * low-power oscillator independent of the APB bus clock.
 *
 * Only the plain periodic-timer function is exposed.  LPTIM's low-power
 * differentiators -- keeping the counter running through CPU Stop mode to
 * wake the core, and the PWM/one-pulse outputs -- are deliberately left as
 * separate ADR-028 sub-items; this driver treats LPTIM1 as an ordinary
 * periodic timer so the cmocka drivertest_timer suite can exercise it on
 * a second /dev/timerN node with a real, LSI-clocked interrupt.
 *
 * LPTIM differs from the general-purpose timers in its programming model:
 * CFGR and the interrupt-enable register must be written while the timer
 * is disabled; the auto-reload register must be written only after the
 * timer is enabled and each write must wait for the ISR.ARROK handshake;
 * and the periodic event is the auto-reload match (ARRM), not the TIM
 * update flag.  Because the counter clock (LSI) is asynchronous to the
 * APB read bus, the counter is read coherently (two matching reads).
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
#include <nuttx/irq.h>
#include <nuttx/timers/timer.h>
#include <nuttx/spinlock.h>

#include "arm_internal.h"
#include "chip.h"
#include "stm32n6_lptim.h"
#include "hardware/stm32_lptim.h"
#include "hardware/stm32_rcc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* LSI nominal frequency (Hz).  The LPTIM tick equals one LSI period with
 * the prescaler left at divide-by-1, i.e. ~31.25 us.
 */

#define STM32N6_LPTIM_LSI_FREQ  32000

/* 16-bit counter: the largest reload is 0x10000 ticks, so the maximum
 * timeout is that many LSI periods expressed in microseconds.
 */

#define STM32N6_LPTIM_MAXTICKS  0x10000ull
#define STM32N6_LPTIM_MAXTIMEOUT \
  ((STM32N6_LPTIM_MAXTICKS * 1000000ull) / STM32N6_LPTIM_LSI_FREQ)

/* Bounded handshake budgets (microseconds).  LSI enable and the ARROK
 * handshake each take only a few slow-clock cycles; never spin forever.
 */

#define STM32N6_LPTIM_TIMEOUT_US  10000

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_lptim_lowerhalf_s
{
  const struct timer_ops_s *ops;      /* Lower-half ops (must be 1st) */
  uint32_t                  base;     /* LPTIM register base address */
  int                       irq;      /* LPTIM global IRQ number */
  tccb_t                    callback; /* Upper-half timeout callback */
  void                     *arg;      /* Argument for the callback */
  uint32_t                  timeout;  /* Current timeout (microseconds) */
  bool                      started;  /* True when the timer is running */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int  stm32n6_lptim_start(struct timer_lowerhalf_s *lower);
static int  stm32n6_lptim_stop(struct timer_lowerhalf_s *lower);
static int  stm32n6_lptim_getstatus(struct timer_lowerhalf_s *lower,
                                     struct timer_status_s *status);
static int  stm32n6_lptim_settimeout(struct timer_lowerhalf_s *lower,
                                      uint32_t timeout);
static void stm32n6_lptim_setcallback(struct timer_lowerhalf_s *lower,
                                       tccb_t callback, void *arg);
static int  stm32n6_lptim_maxtimeout(struct timer_lowerhalf_s *lower,
                                      uint32_t *maxtimeout);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct timer_ops_s g_stm32n6_lptim_ops =
{
  .start       = stm32n6_lptim_start,
  .stop        = stm32n6_lptim_stop,
  .getstatus   = stm32n6_lptim_getstatus,
  .settimeout  = stm32n6_lptim_settimeout,
  .setcallback = stm32n6_lptim_setcallback,
  .maxtimeout  = stm32n6_lptim_maxtimeout,
};

#ifdef CONFIG_STM32_LPTIM1
static struct stm32n6_lptim_lowerhalf_s g_lptim1_lowerhalf =
{
  .ops  = &g_stm32n6_lptim_ops,
  .base = STM32_LPTIM1_BASE,
  .irq  = STM32_IRQ_LPTIM1,
};
#endif

static spinlock_t g_lptim_lock = SP_UNLOCKED;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_lptim_timeout_to_ticks
 *
 * Description:
 *   Convert a timeout in microseconds to LPTIM counter ticks (LSI periods).
 *   Returns the tick count, or 0 if the request is out of the valid
 *   [1, 0x10000] range.
 *
 ****************************************************************************/

static uint32_t stm32n6_lptim_timeout_to_ticks(uint32_t timeout)
{
  uint64_t ticks;

  ticks = ((uint64_t)timeout * STM32N6_LPTIM_LSI_FREQ) / 1000000ull;
  if (ticks == 0 || ticks > STM32N6_LPTIM_MAXTICKS)
    {
      return 0;
    }

  return (uint32_t)ticks;
}

/****************************************************************************
 * Name: stm32n6_lptim_readcnt
 *
 * Description:
 *   Read the LPTIM counter coherently.  The counter is clocked by the LSI,
 *   which is asynchronous to the APB read bus, so a single read can catch
 *   the register mid-carry; the reference manual requires reading until two
 *   consecutive reads agree.
 *
 ****************************************************************************/

static uint32_t stm32n6_lptim_readcnt(struct stm32n6_lptim_lowerhalf_s *priv)
{
  uint32_t v1;
  uint32_t v2;

  v1 = getreg32(priv->base + STM32_LPTIM_CNT_OFFSET);
  do
    {
      v2 = v1;
      v1 = getreg32(priv->base + STM32_LPTIM_CNT_OFFSET);
    }
  while (v1 != v2);

  return v1 & 0xffff;
}

/****************************************************************************
 * Name: stm32n6_lptim_enableclk
 *
 * Description:
 *   Bring up the LPTIM1 clocking: enable the LSI oscillator, route it to
 *   the LPTIM1 kernel-clock mux, open the APB1 peripheral gate and keep it
 *   alive across CPU Sleep (WFI).  Without the LPEN bit the clock gates
 *   while a task blocks and the periodic interrupt never wakes the core.
 *
 ****************************************************************************/

static int stm32n6_lptim_enableclk(struct stm32n6_lptim_lowerhalf_s *priv)
{
  irqstate_t flags;
  uint32_t regval;
  int i;
  int ret = -ETIMEDOUT;

  UNUSED(priv);

  flags = spin_lock_irqsave(&g_lptim_lock);

  /* Enable LSI (write to the CSR set-alias) and wait for it to stabilize */

  putreg32(RCC_CR_LSION, STM32_RCC_CSR);

  for (i = 0; i < STM32N6_LPTIM_TIMEOUT_US; i++)
    {
      if ((getreg32(STM32_RCC_SR) & RCC_SR_LSIRDY) != 0)
        {
          ret = OK;
          break;
        }

      up_udelay(1);
    }

  if (ret < 0)
    {
      spin_unlock_irqrestore(&g_lptim_lock, flags);
      return ret;
    }

  /* Select LSI as the LPTIM1 kernel clock */

  regval  = getreg32(STM32_RCC_CCIPR12);
  regval &= ~RCC_CCIPR12_LPTIM1SEL_MASK;
  regval |= RCC_CCIPR12_LPTIM1SEL_LSI;
  putreg32(regval, STM32_RCC_CCIPR12);

  /* Open the APB1 peripheral gate and its Sleep-mode keep-alive */

  regval  = getreg32(STM32_RCC_APB1ENR1);
  regval |= RCC_APB1ENR1_LPTIM1EN;
  putreg32(regval, STM32_RCC_APB1ENR1);

  regval  = getreg32(STM32_RCC_APB1LPENR1);
  regval |= RCC_APB1LPENR1_LPTIM1LPEN;
  putreg32(regval, STM32_RCC_APB1LPENR1);

  spin_unlock_irqrestore(&g_lptim_lock, flags);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_lptim_write_arr
 *
 * Description:
 *   Program the auto-reload register while the timer is enabled and wait
 *   for the ISR.ARROK handshake, bounded by a timeout so it can never spin
 *   forever.  ARR must only be written with the timer enabled.
 *
 ****************************************************************************/

static int stm32n6_lptim_write_arr(struct stm32n6_lptim_lowerhalf_s *priv,
                                    uint32_t ticks)
{
  int i;

  /* Clear a stale ARROK, program (ticks - 1), then wait for ARROK */

  putreg32(LPTIM_ICR_ARROKCF, priv->base + STM32_LPTIM_ICR_OFFSET);
  putreg32(ticks - 1, priv->base + STM32_LPTIM_ARR_OFFSET);

  for (i = 0; i < STM32N6_LPTIM_TIMEOUT_US; i++)
    {
      if ((getreg32(priv->base + STM32_LPTIM_ISR_OFFSET) &
           LPTIM_ISR_ARROK) != 0)
        {
          putreg32(LPTIM_ICR_ARROKCF, priv->base + STM32_LPTIM_ICR_OFFSET);
          return OK;
        }

      up_udelay(1);
    }

  return -ETIMEDOUT;
}

/****************************************************************************
 * Name: stm32n6_lptim_interrupt
 *
 * Description:
 *   Auto-reload-match interrupt handler.  Acknowledges ARRM and invokes the
 *   registered upper-half callback.  A non-reloading (or NULL) callback
 *   stops the timer.
 *
 ****************************************************************************/

static int stm32n6_lptim_interrupt(int irq, void *context, void *arg)
{
  struct stm32n6_lptim_lowerhalf_s *priv =
    (struct stm32n6_lptim_lowerhalf_s *)arg;
  uint32_t isr;

  DEBUGASSERT(priv != NULL);

  isr = getreg32(priv->base + STM32_LPTIM_ISR_OFFSET);
  if ((isr & LPTIM_ISR_ARRM) == 0)
    {
      return OK;
    }

  /* Acknowledge the auto-reload match */

  putreg32(LPTIM_ICR_ARRMCF, priv->base + STM32_LPTIM_ICR_OFFSET);

  if (priv->callback != NULL)
    {
      uint32_t next = priv->timeout;

      if (priv->callback(&next, priv->arg))
        {
          /* Honour a possibly-updated interval */

          if (next != priv->timeout)
            {
              stm32n6_lptim_settimeout(
                (struct timer_lowerhalf_s *)priv, next);
            }
        }
      else
        {
          stm32n6_lptim_stop((struct timer_lowerhalf_s *)priv);
        }
    }
  else
    {
      stm32n6_lptim_stop((struct timer_lowerhalf_s *)priv);
    }

  return OK;
}

/****************************************************************************
 * Name: stm32n6_lptim_start
 ****************************************************************************/

static int stm32n6_lptim_start(struct timer_lowerhalf_s *lower)
{
  struct stm32n6_lptim_lowerhalf_s *priv =
    (struct stm32n6_lptim_lowerhalf_s *)lower;
  irqstate_t flags;
  uint32_t ticks;
  int ret;

  DEBUGASSERT(priv != NULL);

  if (priv->started)
    {
      return -EBUSY;
    }

  ticks = stm32n6_lptim_timeout_to_ticks(priv->timeout);
  if (ticks == 0)
    {
      return -EINVAL;
    }

  /* Bring up LSI + kernel clock + APB gate before touching any register */

  ret = stm32n6_lptim_enableclk(priv);
  if (ret < 0)
    {
      tmrerr("ERROR: LPTIM LSI clock enable timeout\n");
      return ret;
    }

  flags = spin_lock_irqsave(&g_lptim_lock);

  /* Configuration and interrupt-enable registers may only be written while
   * the timer is disabled.  Use the internal (kernel) clock with the
   * prescaler at divide-by-1 and enable ARR preload for glitch-free
   * updates, then arm the auto-reload-match interrupt.
   */

  putreg32(0, priv->base + STM32_LPTIM_CR_OFFSET);
  putreg32(LPTIM_CFGR_PRELOAD, priv->base + STM32_LPTIM_CFGR_OFFSET);
  putreg32(LPTIM_DIER_ARRMIE, priv->base + STM32_LPTIM_DIER_OFFSET);

  /* Enable the timer; ARR can only be programmed once enabled */

  putreg32(LPTIM_CR_ENABLE, priv->base + STM32_LPTIM_CR_OFFSET);

  ret = stm32n6_lptim_write_arr(priv, ticks);
  if (ret < 0)
    {
      putreg32(0, priv->base + STM32_LPTIM_CR_OFFSET);
      spin_unlock_irqrestore(&g_lptim_lock, flags);
      tmrerr("ERROR: LPTIM ARR update (ARROK) timeout\n");
      return ret;
    }

  /* Drop any pending match flag, then start counting continuously */

  putreg32(LPTIM_ICR_ARRMCF, priv->base + STM32_LPTIM_ICR_OFFSET);
  modifyreg32(priv->base + STM32_LPTIM_CR_OFFSET, 0, LPTIM_CR_CNTSTRT);

  priv->started = true;

  spin_unlock_irqrestore(&g_lptim_lock, flags);

  up_enable_irq(priv->irq);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_lptim_stop
 ****************************************************************************/

static int stm32n6_lptim_stop(struct timer_lowerhalf_s *lower)
{
  struct stm32n6_lptim_lowerhalf_s *priv =
    (struct stm32n6_lptim_lowerhalf_s *)lower;
  irqstate_t flags;

  DEBUGASSERT(priv != NULL);

  if (!priv->started)
    {
      return -EINVAL;
    }

  flags = spin_lock_irqsave(&g_lptim_lock);

  /* Clearing ENABLE stops and resets the counter.  Disable the interrupt
   * (writable now that the timer is disabled) and drop any pending flag.
   */

  putreg32(0, priv->base + STM32_LPTIM_CR_OFFSET);
  putreg32(0, priv->base + STM32_LPTIM_DIER_OFFSET);
  putreg32(LPTIM_ICR_ARRMCF, priv->base + STM32_LPTIM_ICR_OFFSET);

  priv->started = false;

  spin_unlock_irqrestore(&g_lptim_lock, flags);

  up_disable_irq(priv->irq);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_lptim_getstatus
 ****************************************************************************/

static int stm32n6_lptim_getstatus(struct timer_lowerhalf_s *lower,
                                    struct timer_status_s *status)
{
  struct stm32n6_lptim_lowerhalf_s *priv =
    (struct stm32n6_lptim_lowerhalf_s *)lower;
  uint32_t arr;
  uint32_t cnt;
  uint32_t left;

  DEBUGASSERT(priv != NULL && status != NULL);

  status->flags = 0;
  if (priv->started)
    {
      status->flags |= TCFLAGS_ACTIVE;
    }

  if (priv->callback != NULL)
    {
      status->flags |= TCFLAGS_HANDLER;
    }

  status->timeout = priv->timeout;

  /* Convert the remaining ticks back to microseconds (1 tick = LSI period) */

  arr  = getreg32(priv->base + STM32_LPTIM_ARR_OFFSET) & 0xffff;
  cnt  = stm32n6_lptim_readcnt(priv);
  left = (arr >= cnt) ? (arr - cnt) : 0;

  status->timeleft =
    (uint32_t)(((uint64_t)left * 1000000ull) / STM32N6_LPTIM_LSI_FREQ);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_lptim_settimeout
 ****************************************************************************/

static int stm32n6_lptim_settimeout(struct timer_lowerhalf_s *lower,
                                     uint32_t timeout)
{
  struct stm32n6_lptim_lowerhalf_s *priv =
    (struct stm32n6_lptim_lowerhalf_s *)lower;
  irqstate_t flags;
  uint32_t ticks;
  int ret = OK;

  DEBUGASSERT(priv != NULL);

  ticks = stm32n6_lptim_timeout_to_ticks(timeout);
  if (ticks == 0)
    {
      return -EINVAL;
    }

  flags = spin_lock_irqsave(&g_lptim_lock);

  priv->timeout = timeout;

  /* If the timer is already running, reprogram the auto-reload live (the
   * PRELOAD bit defers it to the next period).  Otherwise start() applies
   * the reload once the timer is enabled.
   */

  if (priv->started)
    {
      ret = stm32n6_lptim_write_arr(priv, ticks);
    }

  spin_unlock_irqrestore(&g_lptim_lock, flags);

  return ret;
}

/****************************************************************************
 * Name: stm32n6_lptim_setcallback
 ****************************************************************************/

static void stm32n6_lptim_setcallback(struct timer_lowerhalf_s *lower,
                                       tccb_t callback, void *arg)
{
  struct stm32n6_lptim_lowerhalf_s *priv =
    (struct stm32n6_lptim_lowerhalf_s *)lower;
  irqstate_t flags;

  DEBUGASSERT(priv != NULL);

  flags = spin_lock_irqsave(&g_lptim_lock);
  priv->callback = callback;
  priv->arg      = arg;
  spin_unlock_irqrestore(&g_lptim_lock, flags);
}

/****************************************************************************
 * Name: stm32n6_lptim_maxtimeout
 ****************************************************************************/

static int stm32n6_lptim_maxtimeout(struct timer_lowerhalf_s *lower,
                                    uint32_t *maxtimeout)
{
  UNUSED(lower);
  DEBUGASSERT(maxtimeout != NULL);

  *maxtimeout = (uint32_t)STM32N6_LPTIM_MAXTIMEOUT;
  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_lptim_initialize
 *
 * Description:
 *   Bind a low-power timer to a character device and register it.
 *
 * Input Parameters:
 *   devpath - Character device path (e.g. "/dev/timer1").
 *   timer   - LPTIM peripheral number (currently only 1 is supported).
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int stm32n6_lptim_initialize(const char *devpath, int timer)
{
  struct stm32n6_lptim_lowerhalf_s *priv;
  void *handle;

  DEBUGASSERT(devpath != NULL);

  switch (timer)
    {
#ifdef CONFIG_STM32_LPTIM1
      case 1:
        priv = &g_lptim1_lowerhalf;
        break;
#endif

      default:
        return -ENODEV;
    }

  /* Attach the match ISR (kept disabled at the NVIC until start) */

  irq_attach(priv->irq, stm32n6_lptim_interrupt, priv);

  handle = timer_register(devpath, (struct timer_lowerhalf_s *)priv);
  if (handle == NULL)
    {
      irq_detach(priv->irq);
      return -EEXIST;
    }

  return OK;
}
