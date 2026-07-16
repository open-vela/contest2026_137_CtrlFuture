/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_i2c.c
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
 * STM32N6 I2C driver for NuttX.
 * Supports I2C1-4 in master mode, interrupt-driven.
 *
 * Adapted from STM32H7 NuttX reference (stm32_i2c.c).
 * Reference: STM32N6 HAL (stm32n6xx_hal_i2c.c)
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/kmalloc.h>
#include <nuttx/semaphore.h>
#include <nuttx/i2c/i2c_master.h>
#include <syslog.h>
#include <string.h>

#include "stm32n6_i2c.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* I2C instance count */

#define STM32N6_I2C_COUNT   4

/* I2C register base addresses (per CMSIS stm32n647xx.h) */

#define STM32N6_I2C1_BASE   0x40005400
#define STM32N6_I2C2_BASE   0x40005800
#define STM32N6_I2C3_BASE   0x40005C00
#define STM32N6_I2C4_BASE   0x46001C00

/* I2C register offsets (STM32N6 compatible with H7) */

#define I2C_CR1_OFFSET      0x00
#define I2C_CR2_OFFSET      0x04
#define I2C_OAR1_OFFSET     0x08
#define I2C_OAR2_OFFSET     0x0C
#define I2C_TIMINGR_OFFSET  0x10
#define I2C_TIMEOUTR_OFFSET 0x14
#define I2C_ISR_OFFSET      0x18
#define I2C_ICR_OFFSET      0x1C
#define I2C_PECR_OFFSET     0x20
#define I2C_RXDR_OFFSET     0x24
#define I2C_TXDR_OFFSET     0x28

/* I2C_CR1 bits */

#define I2C_CR1_PE          (1 << 0)
#define I2C_CR1_TXIE        (1 << 1)
#define I2C_CR1_RXIE        (1 << 2)
#define I2C_CR1_STOPIE      (1 << 5)
#define I2C_CR1_TCIE        (1 << 6)
#define I2C_CR1_ERRIE       (1 << 7)
#define I2C_CR1_ANFOFF      (1 << 12)
#define I2C_CR1_DNF_MASK    (0x0F << 8)
#define I2C_CR1_SWRST       (1 << 13)

/* I2C_CR2 bits */

#define I2C_CR2_SADD_MASK   (0x3FF << 0)
#define I2C_CR2_RD_WRN      (1 << 10)
#define I2C_CR2_START        (1 << 13)
#define I2C_CR2_STOP         (1 << 14)
#define I2C_CR2_NACK         (1 << 15)
#define I2C_CR2_NBYTES_MASK  (0xFF << 16)
#define I2C_CR2_RELOAD       (1 << 24)
#define I2C_CR2_AUTOEND      (1 << 25)

/* I2C_ISR bits */

#define I2C_ISR_TXE         (1 << 0)
#define I2C_ISR_TXIS        (1 << 1)
#define I2C_ISR_RXNE        (1 << 2)
#define I2C_ISR_ADDR        (1 << 3)
#define I2C_ISR_NACKF       (1 << 4)
#define I2C_ISR_STOPF       (1 << 5)
#define I2C_ISR_TC          (1 << 6)
#define I2C_ISR_TCR         (1 << 7)
#define I2C_ISR_BERR        (1 << 8)
#define I2C_ISR_ARLO        (1 << 9)
#define I2C_ISR_OVR         (1 << 10)
#define I2C_ISR_BUSY        (1 << 15)

/* I2C timing presets for different speeds */

#define I2C_TIMING_100KHZ   0x10C0F5B7  /* 100 kHz @ 64 MHz HSI */
#define I2C_TIMING_400KHZ   0x00E03D5A  /* 400 kHz @ 64 MHz HSI */

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_i2c_config_s
{
  uint32_t base;
  uint32_t clock;
  int      irq;
  int      sda_pin;
  int      scl_pin;
  int      sda_af;
  int      scl_af;
};

struct stm32n6_i2c_priv_s
{
  struct i2c_master_s dev;
  const struct stm32n6_i2c_config_s *config;
  sem_t   lock;
  sem_t   wait;
  uint32_t frequency;
  uint32_t status;
  int      error;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct stm32n6_i2c_config_s g_i2c1_config =
{
  .base = STM32N6_I2C1_BASE,
  .clock = 64000000,
  .irq = 0,  /* TODO: fill IRQ number */
  .sda_pin = 0,
  .scl_pin = 0,
  .sda_af = 4,
  .scl_af = 4,
};

static const struct stm32n6_i2c_config_s g_i2c2_config =
{
  .base = STM32N6_I2C2_BASE,
  .clock = 64000000,
  .irq = 0,
  .sda_pin = 0,
  .scl_pin = 0,
  .sda_af = 4,
  .scl_af = 4,
};

static const struct stm32n6_i2c_config_s g_i2c3_config =
{
  .base = STM32N6_I2C3_BASE,
  .clock = 64000000,
  .irq = 0,
  .sda_pin = 0,
  .scl_pin = 0,
  .sda_af = 4,
  .scl_af = 4,
};

static const struct stm32n6_i2c_config_s g_i2c4_config =
{
  .base = STM32N6_I2C4_BASE,
  .clock = 64000000,
  .irq = 0,
  .sda_pin = 0,
  .scl_pin = 0,
  .sda_af = 4,
  .scl_af = 4,
};

static struct stm32n6_i2c_priv_s g_i2c1_priv;
static struct stm32n6_i2c_priv_s g_i2c2_priv;
static struct stm32n6_i2c_priv_s g_i2c3_priv;
static struct stm32n6_i2c_priv_s g_i2c4_priv;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t stm32n6_i2c_getreg(
    struct stm32n6_i2c_priv_s *priv, uint32_t offset)
{
  return *(volatile uint32_t *)(priv->config->base + offset);
}

static inline void stm32n6_i2c_putreg(
    struct stm32n6_i2c_priv_s *priv, uint32_t offset,
    uint32_t value)
{
  *(volatile uint32_t *)(priv->config->base + offset) = value;
}

static int stm32n6_i2c_wait_isr(
    struct stm32n6_i2c_priv_s *priv, uint32_t mask,
    uint32_t *status)
{
  uint32_t timeout = 100000;

  while (timeout-- > 0)
    {
      *status = stm32n6_i2c_getreg(priv, I2C_ISR_OFFSET);
      if (*status & I2C_ISR_NACKF)
        {
          return -ENXIO;
        }

      if (*status & I2C_ISR_BERR)
        {
          priv->error = -EIO;
          return -EIO;
        }

      if (*status & I2C_ISR_ARLO)
        {
          priv->error = -EBUSY;
          return -EBUSY;
        }

      if (*status & mask)
        {
          return 0;
        }
    }

  priv->error = -ETIMEDOUT;
  return -ETIMEDOUT;
}

static int stm32n6_i2c_transfer(
    struct i2c_master_s *dev, struct i2c_msg_s *msgs,
    int count)
{
  struct stm32n6_i2c_priv_s *priv =
    (struct stm32n6_i2c_priv_s *)dev;
  int i;
  int ret;

  nxsem_wait(&priv->lock);

  for (i = 0; i < count; i++)
    {
      struct i2c_msg_s *msg = &msgs[i];
      uint32_t cr2;
      uint32_t status;
      uint32_t j;

      /* Configure address and direction */

      cr2 = (msg->addr << 1) & I2C_CR2_SADD_MASK;

      if (msg->flags & I2C_M_READ)
        {
          cr2 |= I2C_CR2_RD_WRN;
        }

      cr2 |= (msg->length << 16) & I2C_CR2_NBYTES_MASK;

      /* Auto-end if last message */

      if (i == count - 1)
        {
          cr2 |= I2C_CR2_AUTOEND;
        }

      cr2 |= I2C_CR2_START;

      stm32n6_i2c_putreg(priv, I2C_CR2_OFFSET, cr2);

      /* Transfer data */

      if (msg->flags & I2C_M_READ)
        {
          for (j = 0; j < msg->length; j++)
            {
              ret = stm32n6_i2c_wait_isr(priv,
                                          I2C_ISR_RXNE, &status);
              if (ret < 0)
                {
                  goto errout;
                }

              msg->buffer[j] =
                stm32n6_i2c_getreg(priv,
                                   I2C_RXDR_OFFSET) & 0xff;
            }
        }
      else
        {
          for (j = 0; j < msg->length; j++)
            {
              ret = stm32n6_i2c_wait_isr(priv,
                                          I2C_ISR_TXIS |
                                          I2C_ISR_TC,
                                          &status);
              if (ret < 0)
                {
                  goto errout;
                }

              stm32n6_i2c_putreg(priv, I2C_TXDR_OFFSET,
                                 msg->buffer[j]);
            }
        }

      /* Wait for stop if last message */

      if (i == count - 1)
        {
          ret = stm32n6_i2c_wait_isr(priv,
                                      I2C_ISR_STOPF,
                                      &status);
          if (ret < 0)
            {
              goto errout;
            }
        }
    }

  nxsem_post(&priv->lock);
  return count;

errout:
  stm32n6_i2c_putreg(priv, I2C_CR1_OFFSET, 0);
  stm32n6_i2c_putreg(priv, I2C_CR1_OFFSET, I2C_CR1_PE);
  nxsem_post(&priv->lock);
  return priv->error;
}

static int stm32n6_i2c_setfrequency(
    struct stm32n6_i2c_priv_s *priv, uint32_t frequency)
{
  if (frequency <= 100000)
    {
      stm32n6_i2c_putreg(priv, I2C_TIMINGR_OFFSET,
                          I2C_TIMING_100KHZ);
    }
  else
    {
      stm32n6_i2c_putreg(priv, I2C_TIMINGR_OFFSET,
                          I2C_TIMING_400KHZ);
    }

  priv->frequency = frequency;
  return 0;
}

static int stm32n6_i2c_reset(struct i2c_master_s *dev)
{
  struct stm32n6_i2c_priv_s *priv =
    (struct stm32n6_i2c_priv_s *)dev;

  /* Software reset */

  stm32n6_i2c_putreg(priv, I2C_CR1_OFFSET, I2C_CR1_SWRST);
  stm32n6_i2c_putreg(priv, I2C_CR1_OFFSET, 0);
  stm32n6_i2c_putreg(priv, I2C_CR1_OFFSET, I2C_CR1_PE);

  return OK;
}

static void stm32n6_i2c_init_priv(
    struct stm32n6_i2c_priv_s *priv,
    const struct stm32n6_i2c_config_s *config)
{
  priv->config = config;
  nxsem_init(&priv->lock, 0, 1);
  nxsem_init(&priv->wait, 0, 0);
  priv->dev.ops = NULL; /* Set by caller */
  priv->frequency = 100000;
  priv->error = 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

struct i2c_master_s *stm32n6_i2cbus_initialize(int bus_num)
{
  struct stm32n6_i2c_priv_s *priv;
  const struct stm32n6_i2c_config_s *config;
  static const struct i2c_ops_s g_i2c_ops =
  {
    .transfer = stm32n6_i2c_transfer,
    .reset    = stm32n6_i2c_reset,
  };

  switch (bus_num)
    {
      case 1:
        priv = &g_i2c1_priv;
        config = &g_i2c1_config;
        break;
      case 2:
        priv = &g_i2c2_priv;
        config = &g_i2c2_config;
        break;
      case 3:
        priv = &g_i2c3_priv;
        config = &g_i2c3_config;
        break;
      case 4:
        priv = &g_i2c4_priv;
        config = &g_i2c4_config;
        break;
      default:
        syslog(LOG_ERR, "i2c: unsupported bus %d\n",
               bus_num);
        return NULL;
    }

  if (priv->config != NULL)
    {
      /* Already initialized */

      return &priv->dev;
    }

  stm32n6_i2c_init_priv(priv, config);
  priv->dev.ops = &g_i2c_ops;

  /* Enable I2C peripheral */

  stm32n6_i2c_putreg(priv, I2C_CR1_OFFSET, I2C_CR1_PE);

  /* Set default 100kHz timing */

  stm32n6_i2c_setfrequency(priv, 100000);

  syslog(LOG_INFO, "i2c%d: initialized @ 100kHz\n", bus_num);
  return &priv->dev;
}

int stm32n6_i2cbus_uninitialize(struct i2c_master_s *dev)
{
  struct stm32n6_i2c_priv_s *priv =
    (struct stm32n6_i2c_priv_s *)dev;

  /* Disable I2C peripheral */

  stm32n6_i2c_putreg(priv, I2C_CR1_OFFSET, 0);

  priv->config = NULL;
  nxsem_destroy(&priv->lock);
  nxsem_destroy(&priv->wait);

  return OK;
}
