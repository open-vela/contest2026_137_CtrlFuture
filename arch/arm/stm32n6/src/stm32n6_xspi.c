/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_xspi.c
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
 * STM32N6 XSPI driver for NuttX.
 * Supports XSPI1-2 in memory-mapped mode for NOR Flash.
 * Used for model weight storage and code execution (XIP).
 *
 * Adapted from STM32H7 NuttX reference (stm32_qspi.c).
 * Reference: STM32N6 HAL (stm32n6xx_hal_xspi.c)
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

#include "stm32n6_xspi.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* XSPI register base addresses (per CMSIS stm32n647xx.h) */

#define STM32N6_XSPI1_BASE  0x48025000
#define STM32N6_XSPI2_BASE  0x4802A000

/* XSPI register offsets */

#define XSPI_CR_OFFSET      0x00
#define XSPI_DCR1_OFFSET    0x08
#define XSPI_DCR2_OFFSET    0x0C
#define XSPI_DCR3_OFFSET    0x10
#define XSPI_DCR4_OFFSET    0x14
#define XSPI_SR_OFFSET      0x20
#define XSPI_FCR_OFFSET     0x24
#define XSPI_DLR_OFFSET     0x40
#define XSPI_AR_OFFSET      0x48
#define XSPI_DR_OFFSET      0x50
#define XSPI_CCR_OFFSET     0x100
#define XSPI_TCR_OFFSET     0x108
#define XSPI_IR_OFFSET      0x110
#define XSPI_ABR_OFFSET     0x120
#define XSPI_LPTR_OFFSET    0x130
#define XSPI_WPCCR_OFFSET   0x140
#define XSPI_WPTCR_OFFSET   0x148
#define XSPI_WPIR_OFFSET    0x150
#define XSPI_WPABR_OFFSET   0x160
#define XSPI_WCCR_OFFSET    0x180
#define XSPI_WTCR_OFFSET    0x188
#define XSPI_WIR_OFFSET     0x190
#define XSPI_WABR_OFFSET    0x1A0
#define XSPI_HLCR_OFFSET    0x200

/* XSPI_CR bits */

#define XSPI_CR_EN           (1 << 0)
#define XSPI_CR_ABORT        (1 << 1)
#define XSPI_CR_DMAEN        (1 << 2)
#define XSPI_CR_TCEN         (1 << 3)
#define XSPI_CR_MMWEN        (1 << 6)

/* XSPI_SR bits */

#define XSPI_SR_TCF          (1 << 1)
#define XSPI_SR_TEF          (1 << 0)
#define XSPI_SR_BUSY         (1 << 5)

/* XSPI_DCR1 bits */

#define XSPI_DCR1_DEVSIZE_MASK (0x1F << 16)
#define XSPI_DCR1_MTYP_MASK   (7 << 24)
#define XSPI_DCR1_MTYP_OCTAL  (2 << 24)
#define XSPI_DCR1_CKMODE      (1 << 28)

/* XSPI_CCR bits */

#define XSPI_CCR_IMODE_MASK   (7 << 0)
#define XSPI_CCR_IDTR         (1 << 3)
#define XSPI_CCR_ISIZE_MASK   (3 << 4)
#define XSPI_CCR_ADMODE_MASK  (7 << 8)
#define XSPI_CCR_ADDTR        (1 << 11)
#define XSPI_CCR_ADSIZE_MASK  (3 << 12)
#define XSPI_CCR_DMODE_MASK   (7 << 16)
#define XSPI_CCR_DDTR         (1 << 19)

/* Flash commands (Macronix MX25UM51245G) */

#define NOR_CMD_READ_ID       0x9F
#define NOR_CMD_READ_STATUS   0x05
#define NOR_CMD_WRITE_ENABLE  0x06
#define NOR_CMD_SECTOR_ERASE  0x21
#define NOR_CMD_PAGE_PROGRAM  0x12
#define NOR_CMD_OCTAL_READ    0x0B

/* Memory-mapped mode base address */

#define XSPI2_MMAP_BASE      0x70000000

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_xspi_priv_s
{
  uint32_t base;
  sem_t    lock;
  bool     initialized;
  uint32_t flash_size;
  uint32_t sector_size;
  uint32_t page_size;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct stm32n6_xspi_priv_s g_xspi1_priv;
static struct stm32n6_xspi_priv_s g_xspi2_priv;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t xspi_getreg(
    struct stm32n6_xspi_priv_s *priv, uint32_t offset)
{
  return *(volatile uint32_t *)(priv->base + offset);
}

static inline void xspi_putreg(
    struct stm32n6_xspi_priv_s *priv, uint32_t offset,
    uint32_t value)
{
  *(volatile uint32_t *)(priv->base + offset) = value;
}

static int xspi_wait_tc(struct stm32n6_xspi_priv_s *priv)
{
  uint32_t timeout = 1000000;

  while (timeout-- > 0)
    {
      uint32_t sr = xspi_getreg(priv, XSPI_SR_OFFSET);
      if (sr & XSPI_SR_TEF)
        {
          xspi_putreg(priv, XSPI_FCR_OFFSET,
                       XSPI_SR_TEF);
          return -EIO;
        }

      if (sr & XSPI_SR_TCF)
        {
          xspi_putreg(priv, XSPI_FCR_OFFSET,
                       XSPI_SR_TCF);
          return 0;
        }
    }

  return -ETIMEDOUT;
}

static int xspi_wait_idle(struct stm32n6_xspi_priv_s *priv)
{
  uint32_t timeout = 1000000;

  while (timeout-- > 0)
    {
      if (!(xspi_getreg(priv, XSPI_SR_OFFSET) &
            XSPI_SR_BUSY))
        {
          return 0;
        }
    }

  return -ETIMEDOUT;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32n6_xspi_initialize(int bus_num)
{
  struct stm32n6_xspi_priv_s *priv;

  switch (bus_num)
    {
      case 1:
        priv = &g_xspi1_priv;
        priv->base = STM32N6_XSPI1_BASE;
        break;
      case 2:
        priv = &g_xspi2_priv;
        priv->base = STM32N6_XSPI2_BASE;
        break;
      default:
        syslog(LOG_ERR, "xspi: unsupported bus %d\n",
               bus_num);
        return -EINVAL;
    }

  if (priv->initialized)
    {
      return 0;
    }

  nxsem_init(&priv->lock, 0, 1);
  priv->flash_size = 0x1000000;  /* 16MB default */
  priv->sector_size = 4096;
  priv->page_size = 256;

  /* Disable XSPI */

  xspi_putreg(priv, XSPI_CR_OFFSET, 0);

  /* Configure device: OCTAL mode, 256MB flash, clock mode 0 */

  xspi_putreg(priv, XSPI_DCR1_OFFSET,
               XSPI_DCR1_MTYP_OCTAL |
               (24 << 16) |  /* 2^24 = 16MB */
               XSPI_DCR1_CKMODE);

  /* Configure CS timing */

  xspi_putreg(priv, XSPI_DCR2_OFFSET, 0);
  xspi_putreg(priv, XSPI_DCR3_OFFSET, 0);

  /* Memory-mapped mode timeout */

  xspi_putreg(priv, XSPI_LPTR_OFFSET, 0x400);

  /* Enable XSPI */

  xspi_putreg(priv, XSPI_CR_OFFSET, XSPI_CR_EN);

  priv->initialized = true;

  syslog(LOG_INFO, "xspi%d: initialized, mapped @ "
         "0x%08lx\n",
         bus_num,
         bus_num == 2 ? (unsigned long)XSPI2_MMAP_BASE :
         (unsigned long)0x70000000);

  return 0;
}

int stm32n6_xspi_enable_mmap(int bus_num)
{
  struct stm32n6_xspi_priv_s *priv;

  switch (bus_num)
    {
      case 1:
        priv = &g_xspi1_priv;
        break;
      case 2:
        priv = &g_xspi2_priv;
        break;
      default:
        return -EINVAL;
    }

  if (!priv->initialized)
    {
      return -ENODEV;
    }

  /* Enable memory-mapped mode */

  xspi_putreg(priv, XSPI_CR_OFFSET,
               XSPI_CR_EN | XSPI_CR_MMWEN);

  syslog(LOG_INFO, "xspi%d: memory-mapped mode enabled\n",
         bus_num);
  return 0;
}

int stm32n6_xspi_read(int bus_num, uint32_t offset,
                       void *buf, uint32_t len)
{
  struct stm32n6_xspi_priv_s *priv;
  uint32_t mmap_base;

  switch (bus_num)
    {
      case 1:
        priv = &g_xspi1_priv;
        mmap_base = 0x70000000;
        break;
      case 2:
        priv = &g_xspi2_priv;
        mmap_base = XSPI2_MMAP_BASE;
        break;
      default:
        return -EINVAL;
    }

  if (!priv->initialized)
    {
      return -ENODEV;
    }

  if (offset + len > priv->flash_size)
    {
      return -EINVAL;
    }

  /* Memory-mapped read */

  memcpy(buf, (void *)(mmap_base + offset), len);
  return len;
}

void stm32n6_xspi_deinit(int bus_num)
{
  struct stm32n6_xspi_priv_s *priv;

  switch (bus_num)
    {
      case 1:
        priv = &g_xspi1_priv;
        break;
      case 2:
        priv = &g_xspi2_priv;
        break;
      default:
        return;
    }

  if (!priv->initialized)
    {
      return;
    }

  xspi_putreg(priv, XSPI_CR_OFFSET, 0);
  priv->initialized = false;
  nxsem_destroy(&priv->lock);

  syslog(LOG_INFO, "xspi%d: deinitialized\n", bus_num);
}
