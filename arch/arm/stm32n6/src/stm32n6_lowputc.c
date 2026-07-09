/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_lowputc.c
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

#include "arm_internal.h"
#include "stm32n6_lowputc.h"
#include "stm32n6_gpio.h"
#include "hardware/stm32_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* USART register offsets */

#define USART_CR1_OFFSET     0x00
#define USART_CR2_OFFSET     0x04
#define USART_CR3_OFFSET     0x08
#define USART_BRR_OFFSET     0x0c
#define USART_ISR_OFFSET     0x1c
#define USART_TDR_OFFSET     0x28

/* USART_CR1 bits */

#define USART_CR1_UE         (1 << 0)
#define USART_CR1_RE         (1 << 2)
#define USART_CR1_TE         (1 << 3)
#define USART_CR1_OVER8      (1 << 15)
#define USART_CR1_FIFOEN     (1 << 29)

/* USART_ISR bits */

#define USART_ISR_TXE        (1 << 7)

/* Console UART selection */

#ifdef CONFIG_USART1_SERIAL_CONSOLE
#  define CONSOLE_BASE       STM32_USART1_BASE
#  define CONSOLE_BAUD       CONFIG_USART1_BAUD
#endif

/* HSI clock = 64 MHz */

#define STM32_HSI_FREQUENCY  64000000

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_lowsetup
 *
 * Description:
 *   Configure the console USART.
 *
 ****************************************************************************/

void stm32n6_lowsetup(void)
{
#ifdef CONSOLE_BASE
  uint32_t brr;

  /* Configure USART1 pins: PE5=TX AF7, PE6=RX AF7 */

  stm32n6_configgpio(GPIO_USART1_TX);
  stm32n6_configgpio(GPIO_USART1_RX);

  /* Disable USART before configuring */

  putreg32(0, CONSOLE_BASE + USART_CR1_OFFSET);
  putreg32(0, CONSOLE_BASE + USART_CR2_OFFSET);
  putreg32(0, CONSOLE_BASE + USART_CR3_OFFSET);

  /* Configure baud rate: BRR = fck / baud */

  brr = (STM32_HSI_FREQUENCY + (CONSOLE_BAUD / 2)) / CONSOLE_BAUD;
  putreg32(brr, CONSOLE_BASE + USART_BRR_OFFSET);

  /* Enable USART: TX + RX + UE */

  putreg32(USART_CR1_UE | USART_CR1_TE | USART_CR1_RE,
           CONSOLE_BASE + USART_CR1_OFFSET);
#endif
}

/****************************************************************************
 * Name: arm_lowputc
 *
 * Description:
 *   Output one byte on the serial console.
 *
 ****************************************************************************/

void arm_lowputc(char ch)
{
#ifdef CONSOLE_BASE
  while ((getreg32(CONSOLE_BASE + USART_ISR_OFFSET) &
          USART_ISR_TXE) == 0)
    {
    }

  putreg32((uint32_t)ch, CONSOLE_BASE + USART_TDR_OFFSET);
#endif
}
