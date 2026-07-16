/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_sai.c
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
 * STM32N6 SAI driver for NuttX.
 * Serial Audio Interface for microphone input (MEMS mic via I2S).
 *
 * Adapted from STM32F7 NuttX reference (stm32_sai.c).
 * Reference: STM32N6 HAL (stm32n6xx_hal_sai.c)
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/kmalloc.h>
#include <nuttx/semaphore.h>
#include <syslog.h>
#include <string.h>

#include "stm32n6_sai.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SAI1 base address (per CMSIS stm32n647xx.h). The driver's register
 * offsets below (SAI_CR1_OFFSET=0x04, etc.) are relative to this base
 * and already encode the CMSIS Block A control-register placement
 * (SAI_Block_TypeDef starting at SAI1_BASE + 0x04), matching the
 * Renode STM32N6_SAI model's absolute offsets from the sai1 base.
 */

#define STM32N6_SAI1_BASE   0x42005800

/* SAI register offsets (per block: 0x20 spacing) */

#define SAI_CR1_OFFSET      0x04
#define SAI_CR2_OFFSET      0x08
#define SAI_FRCR_OFFSET     0x0C
#define SAI_SLOTR_OFFSET    0x10
#define SAI_IMR_OFFSET      0x14
#define SAI_SR_OFFSET       0x18
#define SAI_CLRFR_OFFSET    0x1C
#define SAI_DR_OFFSET       0x20

/* SAI_CR1 bits */

#define SAI_CR1_SAIEN       (1 << 16)
#define SAI_CR1_DMAEN       (1 << 17)
#define SAI_CR1_MODE_MASK   (3 << 0)
#define SAI_CR1_MODE_RX     (2 << 0)
#define SAI_CR1_DS_MASK     (7 << 5)
#define SAI_CR1_DS_16BIT    (4 << 5)
#define SAI_CR1_DS_24BIT    (6 << 5)
#define SAI_CR1_LSBFIRST    (1 << 12)
#define SAI_CR1_CKSTR       (1 << 9)
#define SAI_CR1_MCKDIV_MASK (0x3F << 20)

/* SAI_FRCR bits (frame control) */

#define SAI_FRCR_FRL_MASK   (0xFF << 0)  /* Frame length */
#define SAI_FRCR_FSALL_MASK (0x3F << 8)  /* FS active level length */

/* SAI_SLOTR bits */

#define SAI_SLOTR_FBOFF_MASK (0x1F << 0)
#define SAI_SLOTR_SLOTSZ_MASK (3 << 6)
#define SAI_SLOTR_NBSLOT_MASK (0xF << 8)
#define SAI_SLOTR_SLOTEN_MASK (0xFFFF << 16)

/* SAI status bits */

#define SAI_SR_FREQ         (1 << 3)
#define SAI_SR_OVRUDR       (1 << 0)

/* Audio constants */

#define SAI_DEFAULT_MCLK_DIV  4  /* MCLK = HCLK / 4 */

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_sai_priv_s
{
  uint32_t base;
  sem_t    lock;
  sem_t    dma_wait;
  bool     initialized;
  struct sai_config_s config;
  sai_buffer_cb_t callback;
  void    *cb_arg;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct stm32n6_sai_priv_s g_sai1a_priv;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t sai_getreg(
    struct stm32n6_sai_priv_s *priv, uint32_t offset)
{
  return *(volatile uint32_t *)(priv->base + offset);
}

static inline void sai_putreg(
    struct stm32n6_sai_priv_s *priv, uint32_t offset,
    uint32_t value)
{
  *(volatile uint32_t *)(priv->base + offset) = value;
}

static int sai_config_cr1(struct stm32n6_sai_priv_s *priv)
{
  uint32_t cr1 = 0;

  /* Mode: receiver */

  cr1 |= SAI_CR1_MODE_RX;

  /* Data size */

  if (priv->config.bits_per_sample == 24)
    {
      cr1 |= SAI_CR1_DS_24BIT;
    }
  else
    {
      cr1 |= SAI_CR1_DS_16BIT;
    }

  /* MCLK divider */

  cr1 |= (SAI_DEFAULT_MCLK_DIV << 20) &
          SAI_CR1_MCKDIV_MASK;

  /* Enable DMA */

  cr1 |= SAI_CR1_DMAEN;

  sai_putreg(priv, SAI_CR1_OFFSET, cr1);
  return 0;
}

static void sai_config_frame(struct stm32n6_sai_priv_s *priv)
{
  uint32_t frcr;
  uint32_t slotr;
  uint32_t frame_len;
  uint32_t slot_count;

  if (priv->config.channels == 2)
    {
      frame_len = priv->config.bits_per_sample * 2;
      slot_count = 2;
    }
  else
    {
      frame_len = priv->config.bits_per_sample;
      slot_count = 1;
    }

  /* Frame length and FS active level */

  frcr = ((frame_len - 1) & 0xff) |
         (((priv->config.bits_per_sample - 1) & 0x3f) << 8);
  sai_putreg(priv, SAI_FRCR_OFFSET, frcr);

  /* Slot configuration */

  slotr = (((priv->config.bits_per_sample - 1) & 0x3) << 6) |
          (((slot_count - 1) & 0xf) << 8) |
          (((1 << slot_count) - 1) << 16);
  sai_putreg(priv, SAI_SLOTR_OFFSET, slotr);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32n6_sai_input_init(const struct sai_config_s *config)
{
  struct stm32n6_sai_priv_s *priv = &g_sai1a_priv;

  if (priv->initialized)
    {
      return 0;
    }

  priv->base = STM32N6_SAI1_BASE;
  priv->config = *config;

  nxsem_init(&priv->lock, 0, 1);
  nxsem_init(&priv->dma_wait, 0, 0);

  /* Disable SAI */

  sai_putreg(priv, SAI_CR1_OFFSET, 0);

  /* Configure CR1 */

  sai_config_cr1(priv);

  /* Configure frame and slot */

  sai_config_frame(priv);

  priv->initialized = true;

  syslog(LOG_INFO, "sai: initialized %luHz %ubit %uch\n",
         (unsigned long)config->sample_rate,
         config->bits_per_sample,
         config->channels);
  return 0;
}

int stm32n6_sai_input_start(sai_buffer_cb_t callback,
                              void *arg)
{
  struct stm32n6_sai_priv_s *priv = &g_sai1a_priv;

  if (!priv->initialized)
    {
      return -ENODEV;
    }

  priv->callback = callback;
  priv->cb_arg = arg;

  /* Enable SAI */

  sai_putreg(priv, SAI_CR1_OFFSET,
             sai_getreg(priv, SAI_CR1_OFFSET) |
             SAI_CR1_SAIEN);

  syslog(LOG_INFO, "sai: capture started\n");
  return 0;
}

int stm32n6_sai_input_stop(void)
{
  struct stm32n6_sai_priv_s *priv = &g_sai1a_priv;

  if (!priv->initialized)
    {
      return -ENODEV;
    }

  /* Disable SAI */

  sai_putreg(priv, SAI_CR1_OFFSET,
             sai_getreg(priv, SAI_CR1_OFFSET) &
             ~SAI_CR1_SAIEN);

  priv->callback = NULL;
  priv->cb_arg = NULL;

  syslog(LOG_INFO, "sai: capture stopped\n");
  return 0;
}

void stm32n6_sai_input_deinit(void)
{
  struct stm32n6_sai_priv_s *priv = &g_sai1a_priv;

  if (!priv->initialized)
    {
      return;
    }

  stm32n6_sai_input_stop();

  sai_putreg(priv, SAI_CR1_OFFSET, 0);

  priv->initialized = false;
  nxsem_destroy(&priv->lock);
  nxsem_destroy(&priv->dma_wait);

  syslog(LOG_INFO, "sai: deinitialized\n");
}
