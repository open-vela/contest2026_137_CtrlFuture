/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_serial.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/serial/serial.h>

#include "arm_internal.h"
#include "chip.h"
#include "hardware/stm32_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_USART1_SERIAL_CONSOLE
#  define HAVE_CONSOLE 1
#endif

/* USART register offsets */

#define USART_CR1_OFFSET     0x00
#define USART_CR3_OFFSET     0x08
#define USART_BRR_OFFSET     0x0c
#define USART_ISR_OFFSET     0x1c
#define USART_ICR_OFFSET     0x20
#define USART_RDR_OFFSET     0x24
#define USART_TDR_OFFSET     0x28

/* USART_CR1 bits */

#define USART_CR1_UE         (1 << 0)
#define USART_CR1_RE         (1 << 2)
#define USART_CR1_TE         (1 << 3)
#define USART_CR1_RXNEIE     (1 << 5)
#define USART_CR1_TXEIE      (1 << 7)

/* USART_ISR bits */

#define USART_ISR_RXNE       (1 << 5)
#define USART_ISR_TXE        (1 << 7)
#define USART_ISR_ORE        (1 << 3)

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_uart_s
{
  uintptr_t base;
  int       irq;
  uint32_t  baud;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int  stm32n6_setup(struct uart_dev_s *dev);
static void stm32n6_shutdown(struct uart_dev_s *dev);
static int  stm32n6_attach(struct uart_dev_s *dev);
static void stm32n6_detach(struct uart_dev_s *dev);
static int  stm32n6_interrupt(int irq, void *context,
                              void *arg);
static int  stm32n6_ioctl(struct file *filep, int cmd,
                           unsigned long arg);
static int  stm32n6_receive(struct uart_dev_s *dev,
                             unsigned int *status);
static void stm32n6_rxint(struct uart_dev_s *dev, bool enable);
static bool stm32n6_rxavailable(struct uart_dev_s *dev);
static void stm32n6_send(struct uart_dev_s *dev, int ch);
static void stm32n6_txint(struct uart_dev_s *dev, bool enable);
static bool stm32n6_txready(struct uart_dev_s *dev);
static bool stm32n6_txempty(struct uart_dev_s *dev);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct uart_ops_s g_uart_ops =
{
  .setup       = stm32n6_setup,
  .shutdown    = stm32n6_shutdown,
  .attach      = stm32n6_attach,
  .detach      = stm32n6_detach,
  .ioctl       = stm32n6_ioctl,
  .receive     = stm32n6_receive,
  .rxint       = stm32n6_rxint,
  .rxavailable = stm32n6_rxavailable,
  .send        = stm32n6_send,
  .txint       = stm32n6_txint,
  .txready     = stm32n6_txready,
  .txempty     = stm32n6_txempty,
};

#ifdef CONFIG_STM32_USART1

static char g_usart1rxbuffer[64];
static char g_usart1txbuffer[64];

static struct stm32n6_uart_s g_usart1priv =
{
  .base  = STM32_USART1_BASE,
  .irq   = STM32_IRQ_USART1,
  .baud  = CONFIG_USART1_BAUD,
};

static struct uart_dev_s g_usart1port =
{
  .recv  =
    {
      .size   = sizeof(g_usart1rxbuffer),
      .buffer = g_usart1rxbuffer,
    },
  .xmit  =
    {
      .size   = sizeof(g_usart1txbuffer),
      .buffer = g_usart1txbuffer,
    },
  .ops   = &g_uart_ops,
  .priv  = &g_usart1priv,
};

#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int stm32n6_setup(struct uart_dev_s *dev)
{
  /* Already configured in lowsetup */

  return OK;
}

static void stm32n6_shutdown(struct uart_dev_s *dev)
{
  struct stm32n6_uart_s *priv = dev->priv;

  putreg32(0, priv->base + USART_CR1_OFFSET);
}

static int stm32n6_attach(struct uart_dev_s *dev)
{
  struct stm32n6_uart_s *priv = dev->priv;
  int ret;

  ret = irq_attach(priv->irq, stm32n6_interrupt, dev);
  if (ret == OK)
    {
      up_enable_irq(priv->irq);
    }

  return ret;
}

static void stm32n6_detach(struct uart_dev_s *dev)
{
  struct stm32n6_uart_s *priv = dev->priv;

  up_disable_irq(priv->irq);
  irq_detach(priv->irq);
}

static int stm32n6_interrupt(int irq, void *context, void *arg)
{
  struct uart_dev_s *dev = (struct uart_dev_s *)arg;
  struct stm32n6_uart_s *priv = dev->priv;
  uint32_t isr;

  isr = getreg32(priv->base + USART_ISR_OFFSET);

  if (isr & USART_ISR_ORE)
    {
      putreg32(USART_ISR_ORE,
               priv->base + USART_ICR_OFFSET);
    }

  if (isr & USART_ISR_RXNE)
    {
      uart_recvchars(dev);
    }

  if (isr & USART_ISR_TXE)
    {
      uart_xmitchars(dev);
    }

  return OK;
}

static int stm32n6_ioctl(struct file *filep, int cmd,
                          unsigned long arg)
{
  return -ENOTTY;
}

static int stm32n6_receive(struct uart_dev_s *dev,
                            unsigned int *status)
{
  struct stm32n6_uart_s *priv = dev->priv;

  *status = getreg32(priv->base + USART_ISR_OFFSET);
  return (int)getreg32(priv->base + USART_RDR_OFFSET);
}

static void stm32n6_rxint(struct uart_dev_s *dev, bool enable)
{
  struct stm32n6_uart_s *priv = dev->priv;
  uint32_t regval;

  regval = getreg32(priv->base + USART_CR1_OFFSET);

  if (enable)
    {
      regval |= USART_CR1_RXNEIE;
    }
  else
    {
      regval &= ~USART_CR1_RXNEIE;
    }

  putreg32(regval, priv->base + USART_CR1_OFFSET);
}

static bool stm32n6_rxavailable(struct uart_dev_s *dev)
{
  struct stm32n6_uart_s *priv = dev->priv;

  return (getreg32(priv->base + USART_ISR_OFFSET) &
          USART_ISR_RXNE) != 0;
}

static void stm32n6_send(struct uart_dev_s *dev, int ch)
{
  struct stm32n6_uart_s *priv = dev->priv;

  putreg32((uint32_t)ch, priv->base + USART_TDR_OFFSET);
}

static void stm32n6_txint(struct uart_dev_s *dev, bool enable)
{
  struct stm32n6_uart_s *priv = dev->priv;
  uint32_t regval;

  regval = getreg32(priv->base + USART_CR1_OFFSET);

  if (enable)
    {
      regval |= USART_CR1_TXEIE;
      putreg32(regval, priv->base + USART_CR1_OFFSET);
      uart_xmitchars(dev);
    }
  else
    {
      regval &= ~USART_CR1_TXEIE;
      putreg32(regval, priv->base + USART_CR1_OFFSET);
    }
}

static bool stm32n6_txready(struct uart_dev_s *dev)
{
  struct stm32n6_uart_s *priv = dev->priv;

  return (getreg32(priv->base + USART_ISR_OFFSET) &
          USART_ISR_TXE) != 0;
}

static bool stm32n6_txempty(struct uart_dev_s *dev)
{
  return stm32n6_txready(dev);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_earlyserialinit
 *
 * Description:
 *   Early serial initialization (called from __start).
 *
 ****************************************************************************/

#ifdef USE_EARLYSERIALINIT
void arm_earlyserialinit(void)
{
#ifdef CONFIG_STM32_USART1
#  ifdef CONFIG_USART1_SERIAL_CONSOLE
  g_usart1port.isconsole = true;
#  endif
#endif
}
#endif

/****************************************************************************
 * Name: arm_serialinit
 *
 * Description:
 *   Register serial console and serial ports.
 *
 ****************************************************************************/

void arm_serialinit(void)
{
#ifdef CONFIG_STM32_USART1
#  ifdef CONFIG_USART1_SERIAL_CONSOLE
  uart_register("/dev/console", &g_usart1port);
#  endif
  uart_register("/dev/ttyS0", &g_usart1port);
#endif
}

/****************************************************************************
 * Name: up_putc
 *
 * Description:
 *   Output one character on the console.
 *
 ****************************************************************************/

void up_putc(int ch)
{
#ifdef HAVE_CONSOLE
  if (ch == '\n')
    {
      arm_lowputc('\r');
    }

  arm_lowputc((char)ch);
#endif
}
