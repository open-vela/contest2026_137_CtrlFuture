/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_dma.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * STM32N6 DMA driver for NuttX.
 * Supports GPDMA (General Purpose DMA) for memory/peripheral transfers.
 *
 * Adapted from STM32H7 NuttX reference (stm32_dma.c).
 * Reference: STM32N6 HAL (stm32n6xx_hal_dma.c)
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

#include "stm32n6_dma.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* GPDMA register base addresses */

#define STM32N6_GPDMA1_BASE  0x40021000
#define STM32N6_GPDMA2_BASE  0x40021400

/* GPDMA channel register offsets (per CMSIS DMA_Channel_TypeDef) */

#define GPDMA_CLBAR_OFFSET     0x00
#define GPDMA_CCIDCFGR_OFFSET  0x04
#define GPDMA_CFCR_OFFSET      0x0C
#define GPDMA_CSR_OFFSET       0x10
#define GPDMA_CCR_OFFSET       0x14
#define GPDMA_CTR1_OFFSET      0x40
#define GPDMA_CTR2_OFFSET      0x44
#define GPDMA_CBR1_OFFSET      0x48
#define GPDMA_CSAR_OFFSET      0x4C
#define GPDMA_CDAR_OFFSET      0x50
#define GPDMA_CTR3_OFFSET      0x54
#define GPDMA_CBR2_OFFSET      0x58
#define GPDMA_CLLR_OFFSET      0xCC

/* GPDMA global register offsets */

#define GPDMA_GISR_OFFSET      0x00
#define GPDMA_SECCFGR2_OFFSET  0x24

/* GPDMA_CCR (Channel Control) bits */

#define GPDMA_CCR_EN            (1 << 0)
#define GPDMA_CCR_RESET         (1 << 1)
#define GPDMA_CCR_SUSP          (1 << 2)
#define GPDMA_CCR_TCIE          (1 << 8)   /* Transfer complete IE */
#define GPDMA_CCR_HTIE          (1 << 9)   /* Half transfer IE */
#define GPDMA_CCR_DTEIE         (1 << 10)  /* Data transfer error IE */
#define GPDMA_CCR_ULEIE         (1 << 11)  /* Update link error IE */
#define GPDMA_CCR_USEIE         (1 << 12)  /* User setting error IE */
#define GPDMA_CCR_TOIE          (1 << 13)  /* Trigger overrun IE */
#define GPDMA_CCR_SWRIOIE       (1 << 14)  /* Software request overrun IE */

/* GPDMA_CTR2 (Transfer Register 2) bits */

#define GPDMA_CTR2_DREQ        (1 << 0)   /* Destination request */
#define GPDMA_CTR2_SWREQ       (1 << 9)   /* Software request */
#define GPDMA_CTR2_DREQ_MASK   (0x7f << 4)
#define GPDMA_CTR2_TCEM_MASK   (3 << 14)  /* Transfer complete event mode */

/* GPDMA_CBR1 (Block Register 1) bits */

#define GPDMA_CBR1_BNDT_MASK   0xFFFF     /* Block number of data bytes */

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_dma_priv_s
{
  uint32_t base;
  uint32_t channel;
  sem_t    lock;
  sem_t    wait;
  bool     initialized;
  uint32_t src_addr;
  uint32_t dst_addr;
  uint32_t block_size;
  bool     complete;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct stm32n6_dma_priv_s g_dma_channels[16];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t dma_getreg(
    struct stm32n6_dma_priv_s *priv, uint32_t offset)
{
  return *(volatile uint32_t *)(priv->base + priv->channel * 0x80 + offset);
}

static inline void dma_putreg(
    struct stm32n6_dma_priv_s *priv, uint32_t offset,
    uint32_t value)
{
  *(volatile uint32_t *)(priv->base + priv->channel * 0x80 + offset) = value;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32n6_dma_init(int channel)
{
  struct stm32n6_dma_priv_s *priv;

  if (channel < 0 || channel >= 16)
    {
      return -EINVAL;
    }

  priv = &g_dma_channels[channel];

  if (priv->initialized)
    {
      return 0;
    }

  priv->base = STM32N6_GPDMA1_BASE;
  priv->channel = channel;

  nxsem_init(&priv->lock, 0, 1);
  nxsem_init(&priv->wait, 0, 0);

  /* Disable channel */

  dma_putreg(priv, GPDMA_CCR_OFFSET, 0);

  priv->initialized = true;

  syslog(LOG_INFO, "dma: channel %d initialized\n", channel);
  return 0;
}

int stm32n6_dma_start(int channel, uint32_t src, uint32_t dst,
                       uint32_t size)
{
  struct stm32n6_dma_priv_s *priv;

  if (channel < 0 || channel >= 16)
    {
      return -EINVAL;
    }

  priv = &g_dma_channels[channel];

  if (!priv->initialized)
    {
      return -ENODEV;
    }

  nxsem_wait(&priv->lock);

  priv->src_addr = src;
  priv->dst_addr = dst;
  priv->block_size = size;
  priv->complete = false;

  /* Set source address */

  dma_putreg(priv, GPDMA_CSAR_OFFSET, src);

  /* Set destination address */

  dma_putreg(priv, GPDMA_CDAR_OFFSET, dst);

  /* Set block size */

  dma_putreg(priv, GPDMA_CBR1_OFFSET,
             size & GPDMA_CBR1_BNDT_MASK);

  /* Configure transfer: memory-to-memory, software request */

  dma_putreg(priv, GPDMA_CTR2_OFFSET, GPDMA_CTR2_SWREQ);

  /* Enable channel with transfer complete interrupt */

  dma_putreg(priv, GPDMA_CCR_OFFSET,
             GPDMA_CCR_EN | GPDMA_CCR_TCIE);

  nxsem_post(&priv->lock);
  return 0;
}

int stm32n6_dma_wait(int channel, int timeout_ms)
{
  struct stm32n6_dma_priv_s *priv;
  int ret;

  if (channel < 0 || channel >= 16)
    {
      return -EINVAL;
    }

  priv = &g_dma_channels[channel];

  if (!priv->initialized)
    {
      return -ENODEV;
    }

  /* Poll for completion (simplified; real impl uses IRQ + sem) */

  int timeout = timeout_ms * 1000;

  while (timeout-- > 0)
    {
      uint32_t cc = dma_getreg(priv, GPDMA_CCR_OFFSET);
      if (!(cc & GPDMA_CCR_EN))
        {
          return 0;
        }
    }

  return -ETIMEDOUT;
}

void stm32n6_dma_deinit(int channel)
{
  struct stm32n6_dma_priv_s *priv;

  if (channel < 0 || channel >= 16)
    {
      return;
    }

  priv = &g_dma_channels[channel];

  if (!priv->initialized)
    {
      return;
    }

  /* Disable channel */

  dma_putreg(priv, GPDMA_CCR_OFFSET, GPDMA_CCR_RESET);

  priv->initialized = false;
  nxsem_destroy(&priv->lock);
  nxsem_destroy(&priv->wait);

  syslog(LOG_INFO, "dma: channel %d deinitialized\n", channel);
}
