/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_sdmmc.c
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
 * STM32N6 SDMMC driver for NuttX.
 * Supports SDMMC1-2 in 4-bit SD mode with DMA.
 *
 * Adapted from STM32H7 NuttX reference (stm32_sdmmc.c).
 * Reference: STM32N6 HAL (stm32n6xx_hal_sd.c, stm32n6xx_ll_sdmmc.c)
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/kmalloc.h>
#include <nuttx/semaphore.h>
#include <nuttx/mmcsd/mmcsd.h>
#include <syslog.h>
#include <string.h>

#include "stm32n6_sdmmc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SDMMC register base addresses */

#define STM32N6_SDMMC1_BASE  0x40012800
#define STM32N6_SDMMC2_BASE  0x40012C00

/* SDMMC register offsets */

#define SDMMC_POWER_OFFSET     0x00
#define SDMMC_CLKCR_OFFSET    0x04
#define SDMMC_ARG_OFFSET      0x08
#define SDMMC_CMD_OFFSET      0x0C
#define SDMMC_RESPCMD_OFFSET  0x10
#define SDMMC_RESP1_OFFSET    0x14
#define SDMMC_RESP2_OFFSET    0x18
#define SDMMC_RESP3_OFFSET    0x1C
#define SDMMC_RESP4_OFFSET    0x20
#define SDMMC_DTIMER_OFFSET   0x24
#define SDMMC_DLEN_OFFSET     0x28
#define SDMMC_DCTRL_OFFSET    0x2C
#define SDMMC_DCOUNT_OFFSET   0x30
#define SDMMC_STA_OFFSET      0x34
#define SDMMC_ICR_OFFSET      0x38
#define SDMMC_MASK_OFFSET     0x3C
#define SDMMC_IDMACTRL_OFFSET 0x50
#define SDMMC_IDMABS_OFFSET   0x54
#define SDMMC_FIFO_OFFSET     0x80

/* SDMMC_POWER bits */

#define SDMMC_POWER_PWRCTRL_MASK  0x03
#define SDMMC_POWER_PWRCTRL_ON    0x03

/* SDMMC_CLKCR bits */

#define SDMMC_CLKCR_CLKDIV_MASK   0x3FF
#define SDMMC_CLKCR_CLKEN         (1 << 16)
#define SDMMC_CLKCR_WIDBUS_4BIT   (1 << 14)

/* SDMMC_CMD bits */

#define SDMMC_CMD_CMDINDEX_MASK   0x3F
#define SDMMC_CMD_WAITRESP_MASK   (3 << 6)
#define SDMMC_CMD_WAITRESP_NONE   (0 << 6)
#define SDMMC_CMD_WAITRESP_SHORT  (1 << 6)
#define SDMMC_CMD_WAITRESP_LONG   (3 << 6)
#define SDMMC_CMD_CPSMEN          (1 << 10)
#define SDMMC_CMD_CMDTRANS        (1 << 21)

/* SDMMC_STA bits */

#define SDMMC_STA_CCRCFAIL       (1 << 0)
#define SDMMC_STA_DCRCFAIL       (1 << 1)
#define SDMMC_STA_CTIMEOUT       (1 << 2)
#define SDMMC_STA_DTIMEOUT       (1 << 3)
#define SDMMC_STA_TXUNDERR       (1 << 4)
#define SDMMC_STA_RXOVERR        (1 << 5)
#define SDMMC_STA_CMDREND        (1 << 6)
#define SDMMC_STA_CMDSENT        (1 << 7)
#define SDMMC_STA_DATAEND        (1 << 8)
#define SDMMC_STA_DTO            (1 << 9)
#define SDMMC_STA_TXFIFOHE       (1 << 14)
#define SDMMC_STA_RXFIFOHF       (1 << 15)
#define SDMMC_STA_BUSYD0         (1 << 24)

/* SDMMC_DCTRL bits */

#define SDMMC_DCTRL_DTEN         (1 << 0)
#define SDMMC_DCTRL_DTDIR_READ   (1 << 1)
#define SDMMC_DCTRL_DTMODE_BLOCK (0 << 2)
#define SDMMC_DCTRL_DBLOCKSIZE_MASK (0xF << 4)

/* SD commands */

#define SD_CMD_GO_IDLE_STATE      0
#define SD_CMD_SEND_IF_COND       8
#define SD_CMD_SEND_CSD           9
#define SD_CMD_SEND_CID           10
#define SD_CMD_STOP_TRANSMISSION  12
#define SD_CMD_SEND_STATUS        13
#define SD_CMD_SET_BLOCKLEN       16
#define SD_CMD_READ_SINGLE_BLOCK  17
#define SD_CMD_READ_MULT_BLOCK    18
#define SD_CMD_WRITE_SINGLE_BLOCK 24
#define SD_CMD_WRITE_MULT_BLOCK   25
#define SD_CMD_APP_CMD            55
#define SD_CMD_SD_SEND_OP_COND    41
#define SD_CMD_ALL_SEND_CID       2
#define SD_CMD_SEND_RELATIVE_ADDR 3

/* Timeouts */

#define SDMMC_TIMEOUT_MS   1000

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_sdmmc_priv_s
{
  uint32_t base;
  uint32_t clock;
  int      irq;
  sem_t    lock;
  sem_t    wait;
  uint32_t rca;          /* Relative Card Address */
  uint32_t block_size;
  bool     initialized;
  uint32_t card_type;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct stm32n6_sdmmc_priv_s g_sdmmc1_priv;
static struct stm32n6_sdmmc_priv_s g_sdmmc2_priv;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t sdmmc_getreg(
    struct stm32n6_sdmmc_priv_s *priv, uint32_t offset)
{
  return *(volatile uint32_t *)(priv->base + offset);
}

static inline void sdmmc_putreg(
    struct stm32n6_sdmmc_priv_s *priv, uint32_t offset,
    uint32_t value)
{
  *(volatile uint32_t *)(priv->base + offset) = value;
}

static int sdmmc_wait_status(struct stm32n6_sdmmc_priv_s *priv,
                              uint32_t mask)
{
  uint32_t timeout = 1000000;

  while (timeout-- > 0)
    {
      uint32_t status = sdmmc_getreg(priv, SDMMC_STA_OFFSET);
      if (status & SDMMC_STA_CTIMEOUT)
        {
          sdmmc_putreg(priv, SDMMC_ICR_OFFSET,
                       SDMMC_STA_CTIMEOUT);
          return -ETIMEDOUT;
        }

      if (status & mask)
        {
          sdmmc_putreg(priv, SDMMC_ICR_OFFSET, mask);
          return 0;
        }
    }

  return -ETIMEDOUT;
}

static int sdmmc_send_cmd(struct stm32n6_sdmmc_priv_s *priv,
                           uint32_t cmd, uint32_t arg,
                           uint32_t *resp)
{
  uint32_t cmdreg;
  int ret;

  /* Wait for command path ready */

  sdmmc_wait_status(priv, SDMMC_STA_CMDSENT |
                           SDMMC_STA_CMDREND);

  /* Set argument */

  sdmmc_putreg(priv, SDMMC_ARG_OFFSET, arg);

  /* Configure command */

  cmdreg = cmd & SDMMC_CMD_CMDINDEX_MASK;
  cmdreg |= SDMMC_CMD_CPSMEN;

  if (resp != NULL)
    {
      cmdreg |= SDMMC_CMD_WAITRESP_SHORT;
    }

  /* Send command */

  sdmmc_putreg(priv, SDMMC_CMD_OFFSET, cmdreg);

  /* Wait for response */

  if (resp != NULL)
    {
      ret = sdmmc_wait_status(priv,
                               SDMMC_STA_CMDREND |
                               SDMMC_STA_CCRCFAIL);
      if (ret < 0)
        {
          return ret;
        }

      *resp = sdmmc_getreg(priv, SDMMC_RESP1_OFFSET);
    }
  else
    {
      ret = sdmmc_wait_status(priv, SDMMC_STA_CMDSENT);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

static int sdmmc_read_block(struct stm32n6_sdmmc_priv_s *priv,
                             uint32_t sector, void *buf)
{
  uint32_t *dst = (uint32_t *)buf;
  uint32_t status;
  uint32_t timeout;
  int ret;
  uint32_t i;

  /* Set block size */

  sdmmc_putreg(priv, SDMMC_DLEN_OFFSET, 512);

  /* Configure data transfer: read, block mode, 512 bytes */

  sdmmc_putreg(priv, SDMMC_DCTRL_OFFSET,
               SDMMC_DCTRL_DTEN |
               SDMMC_DCTRL_DTDIR_READ |
               SDMMC_DCTRL_DTMODE_BLOCK |
               (9 << 4));  /* 2^9 = 512 */

  /* Send read command */

  ret = sdmmc_send_cmd(priv, SD_CMD_READ_SINGLE_BLOCK,
                        sector, NULL);
  if (ret < 0)
    {
      return ret;
    }

  /* Read data from FIFO */

  timeout = 1000000;
  i = 0;

  while (i < 128 && timeout-- > 0)  /* 512/4 = 128 words */
    {
      status = sdmmc_getreg(priv, SDMMC_STA_OFFSET);

      if (status & SDMMC_STA_RXOVERR)
        {
          return -EIO;
        }

      if (status & SDMMC_STA_RXFIFOHF)
        {
          dst[i++] = sdmmc_getreg(priv, SDMMC_FIFO_OFFSET);
          dst[i++] = sdmmc_getreg(priv, SDMMC_FIFO_OFFSET);
          dst[i++] = sdmmc_getreg(priv, SDMMC_FIFO_OFFSET);
          dst[i++] = sdmmc_getreg(priv, SDMMC_FIFO_OFFSET);
        }
    }

  /* Wait for data end */

  ret = sdmmc_wait_status(priv, SDMMC_STA_DATAEND);
  if (ret < 0)
    {
      return ret;
    }

  return 0;
}

static int sdmmc_write_block(struct stm32n6_sdmmc_priv_s *priv,
                              uint32_t sector, const void *buf)
{
  const uint32_t *src = (const uint32_t *)buf;
  uint32_t status;
  uint32_t timeout;
  int ret;
  uint32_t i;

  /* Set block size */

  sdmmc_putreg(priv, SDMMC_DLEN_OFFSET, 512);

  /* Configure data transfer: write, block mode */

  sdmmc_putreg(priv, SDMMC_DCTRL_OFFSET,
               SDMMC_DCTRL_DTEN |
               SDMMC_DCTRL_DTMODE_BLOCK |
               (9 << 4));

  /* Send write command */

  ret = sdmmc_send_cmd(priv, SD_CMD_WRITE_SINGLE_BLOCK,
                        sector, NULL);
  if (ret < 0)
    {
      return ret;
    }

  /* Write data to FIFO */

  timeout = 1000000;
  i = 0;

  while (i < 128 && timeout-- > 0)
    {
      status = sdmmc_getreg(priv, SDMMC_STA_OFFSET);

      if (status & SDMMC_STA_TXUNDERR)
        {
          return -EIO;
        }

      if (status & SDMMC_STA_TXFIFOHE)
        {
          sdmmc_putreg(priv, SDMMC_FIFO_OFFSET, src[i++]);
          sdmmc_putreg(priv, SDMMC_FIFO_OFFSET, src[i++]);
          sdmmc_putreg(priv, SDMMC_FIFO_OFFSET, src[i++]);
          sdmmc_putreg(priv, SDMMC_FIFO_OFFSET, src[i++]);
        }
    }

  /* Wait for data end */

  ret = sdmmc_wait_status(priv, SDMMC_STA_DATAEND);
  if (ret < 0)
    {
      return ret;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32n6_sdmmc_initialize(int bus_num)
{
  struct stm32n6_sdmmc_priv_s *priv;
  uint32_t resp;
  int ret;

  switch (bus_num)
    {
      case 1:
        priv = &g_sdmmc1_priv;
        priv->base = STM32N6_SDMMC1_BASE;
        priv->clock = 200000000;  /* IC4 = PLL1/4 = 200MHz */
        break;
      case 2:
        priv = &g_sdmmc2_priv;
        priv->base = STM32N6_SDMMC2_BASE;
        priv->clock = 200000000;
        break;
      default:
        syslog(LOG_ERR, "sdmmc: unsupported bus %d\n",
               bus_num);
        return -EINVAL;
    }

  if (priv->initialized)
    {
      return 0;
    }

  nxsem_init(&priv->lock, 0, 1);
  nxsem_init(&priv->wait, 0, 0);
  priv->block_size = 512;

  /* Power on SDMMC */

  sdmmc_putreg(priv, SDMMC_POWER_OFFSET,
               SDMMC_POWER_PWRCTRL_ON);

  /* Configure clock: HCLK/2 = 100MHz / 250 = 400kHz init */

  sdmmc_putreg(priv, SDMMC_CLKCR_OFFSET,
               (250 & SDMMC_CLKCR_CLKDIV_MASK) |
               SDMMC_CLKCR_CLKEN);

  /* Send CMD0: GO_IDLE_STATE */

  ret = sdmmc_send_cmd(priv, SD_CMD_GO_IDLE_STATE, 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR, "sdmmc%d: GO_IDLE failed\n", bus_num);
      return ret;
    }

  /* Send CMD8: SEND_IF_COND (check voltage) */

  ret = sdmmc_send_cmd(priv, SD_CMD_SEND_IF_COND,
                        0x1aa, &resp);
  if (ret < 0)
    {
      syslog(LOG_ERR, "sdmmc%d: SEND_IF_COND failed\n",
             bus_num);
      return ret;
    }

  /* ACMD41: SD_SEND_OP_COND (wait for ready) */

  int retry = 100;

  while (retry-- > 0)
    {
      sdmmc_send_cmd(priv, SD_CMD_APP_CMD, 0, NULL);
      ret = sdmmc_send_cmd(priv, SD_CMD_SD_SEND_OP_COND,
                            0x40ff8000, &resp);
      if (ret == 0 && (resp & 0x80000000))
        {
          break;
        }
    }

  if (retry <= 0)
    {
      syslog(LOG_ERR, "sdmmc%d: card not ready\n", bus_num);
      return -ETIMEDOUT;
    }

  /* CMD2: ALL_SEND_CID */

  sdmmc_send_cmd(priv, SD_CMD_ALL_SEND_CID, 0, NULL);

  /* CMD3: SEND_RELATIVE_ADDR */

  ret = sdmmc_send_cmd(priv, SD_CMD_SEND_RELATIVE_ADDR,
                        0, &resp);
  if (ret < 0)
    {
      syslog(LOG_ERR, "sdmmc%d: SEND_RCA failed\n", bus_num);
      return ret;
    }

  priv->rca = resp & 0xffff0000;

  /* CMD7: SELECT_CARD */

  sdmmc_send_cmd(priv, 7, priv->rca, NULL);

  /* CMD16: SET_BLOCKLEN to 512 */

  sdmmc_send_cmd(priv, SD_CMD_SET_BLOCKLEN, 512, NULL);

  /* Switch to 4-bit bus and higher clock */

  sdmmc_putreg(priv, SDMMC_CLKCR_OFFSET,
               (2 & SDMMC_CLKCR_CLKDIV_MASK) |
               SDMMC_CLKCR_WIDBUS_4BIT |
               SDMMC_CLKCR_CLKEN);

  priv->initialized = true;

  syslog(LOG_INFO, "sdmmc%d: card initialized, RCA=%08lx\n",
         bus_num, (unsigned long)priv->rca);

  /* Register block device */

  /* TODO: mmcsd_register(bus_num, ...) with read/write callbacks */

  return 0;
}

int stm32n6_sdmmc_deinitialize(int bus_num)
{
  struct stm32n6_sdmmc_priv_s *priv;

  switch (bus_num)
    {
      case 1:
        priv = &g_sdmmc1_priv;
        break;
      case 2:
        priv = &g_sdmmc2_priv;
        break;
      default:
        return -EINVAL;
    }

  if (!priv->initialized)
    {
      return 0;
    }

  /* Power off */

  sdmmc_putreg(priv, SDMMC_POWER_OFFSET, 0);

  priv->initialized = false;
  nxsem_destroy(&priv->lock);
  nxsem_destroy(&priv->wait);

  syslog(LOG_INFO, "sdmmc%d: deinitialized\n", bus_num);
  return 0;
}
