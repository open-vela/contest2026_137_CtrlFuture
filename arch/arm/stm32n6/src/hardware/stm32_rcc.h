/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32_rcc.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_RCC_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_RCC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define STM32_RCC_CR_OFFSET         0x0000  /* Clock control */
#define STM32_RCC_SR_OFFSET         0x0004  /* Clock status */
#define STM32_RCC_CFGR1_OFFSET      0x0018  /* Clock configuration 1 */

/* PLL1 configuration */

#define STM32_RCC_PLL1CFGR1_OFFSET  0x0200  /* PLL1 config 1 (DIVM1, SRC) */
#define STM32_RCC_PLL1CFGR2_OFFSET  0x0204  /* PLL1 config 2 (DIVN1) */
#define STM32_RCC_PLL1CFGR3_OFFSET  0x0208  /* PLL1 config 3 (DIVP/Q/R) */

/* PLL2 configuration */

#define STM32_RCC_PLL2CFGR1_OFFSET  0x020c  /* PLL2 config 1 */
#define STM32_RCC_PLL2CFGR2_OFFSET  0x0210  /* PLL2 config 2 */
#define STM32_RCC_PLL2CFGR3_OFFSET  0x0214  /* PLL2 config 3 */

/* PLL3 configuration */

#define STM32_RCC_PLL3CFGR1_OFFSET  0x0218  /* PLL3 config 1 */
#define STM32_RCC_PLL3CFGR2_OFFSET  0x021c  /* PLL3 config 2 */
#define STM32_RCC_PLL3CFGR3_OFFSET  0x0220  /* PLL3 config 3 */

/* PLL4 configuration */

#define STM32_RCC_PLL4CFGR1_OFFSET  0x0224  /* PLL4 config 1 */
#define STM32_RCC_PLL4CFGR2_OFFSET  0x0228  /* PLL4 config 2 */
#define STM32_RCC_PLL4CFGR3_OFFSET  0x022c  /* PLL4 config 3 */

/* Bus clock enable registers */

#define STM32_RCC_AHB1ENR_OFFSET    0x0250  /* AHB1 periph clock enable */
#define STM32_RCC_AHB2ENR_OFFSET    0x0254  /* AHB2 periph clock enable */
#define STM32_RCC_AHB3ENR_OFFSET    0x0258  /* AHB3 periph clock enable */
#define STM32_RCC_AHB4ENR_OFFSET    0x025c  /* AHB4 periph clock enable */
#define STM32_RCC_AHB5ENR_OFFSET    0x0260  /* AHB5 periph clock enable */
#define STM32_RCC_APB1ENR_OFFSET    0x0264  /* APB1 periph clock enable */
#define STM32_RCC_APB2ENR_OFFSET    0x026c  /* APB2 periph clock enable */
#define STM32_RCC_APB4ENR_OFFSET    0x0278  /* APB4 periph clock enable */
#define STM32_RCC_APB5ENR_OFFSET    0x027c  /* APB5 periph clock enable */

/* Bus clock enable register addresses */

#define STM32_RCC_CR       (STM32_RCC_BASE + STM32_RCC_CR_OFFSET)
#define STM32_RCC_SR       (STM32_RCC_BASE + STM32_RCC_SR_OFFSET)
#define STM32_RCC_CFGR1    (STM32_RCC_BASE + STM32_RCC_CFGR1_OFFSET)

#define STM32_RCC_PLL1CFGR1 (STM32_RCC_BASE + STM32_RCC_PLL1CFGR1_OFFSET)
#define STM32_RCC_PLL1CFGR2 (STM32_RCC_BASE + STM32_RCC_PLL1CFGR2_OFFSET)
#define STM32_RCC_PLL1CFGR3 (STM32_RCC_BASE + STM32_RCC_PLL1CFGR3_OFFSET)

#define STM32_RCC_PLL2CFGR1 (STM32_RCC_BASE + STM32_RCC_PLL2CFGR1_OFFSET)
#define STM32_RCC_PLL2CFGR2 (STM32_RCC_BASE + STM32_RCC_PLL2CFGR2_OFFSET)
#define STM32_RCC_PLL2CFGR3 (STM32_RCC_BASE + STM32_RCC_PLL2CFGR3_OFFSET)

#define STM32_RCC_PLL4CFGR1 (STM32_RCC_BASE + STM32_RCC_PLL4CFGR1_OFFSET)
#define STM32_RCC_PLL4CFGR2 (STM32_RCC_BASE + STM32_RCC_PLL4CFGR2_OFFSET)
#define STM32_RCC_PLL4CFGR3 (STM32_RCC_BASE + STM32_RCC_PLL4CFGR3_OFFSET)

#define STM32_RCC_AHB1ENR  (STM32_RCC_BASE + STM32_RCC_AHB1ENR_OFFSET)
#define STM32_RCC_AHB2ENR  (STM32_RCC_BASE + STM32_RCC_AHB2ENR_OFFSET)
#define STM32_RCC_AHB3ENR  (STM32_RCC_BASE + STM32_RCC_AHB3ENR_OFFSET)
#define STM32_RCC_AHB4ENR  (STM32_RCC_BASE + STM32_RCC_AHB4ENR_OFFSET)
#define STM32_RCC_AHB5ENR  (STM32_RCC_BASE + STM32_RCC_AHB5ENR_OFFSET)
#define STM32_RCC_APB1ENR  (STM32_RCC_BASE + STM32_RCC_APB1ENR_OFFSET)
#define STM32_RCC_APB2ENR  (STM32_RCC_BASE + STM32_RCC_APB2ENR_OFFSET)
#define STM32_RCC_APB4ENR  (STM32_RCC_BASE + STM32_RCC_APB4ENR_OFFSET)
#define STM32_RCC_APB5ENR  (STM32_RCC_BASE + STM32_RCC_APB5ENR_OFFSET)

/* RCC_CR bits (CMSIS stm32n647xx.h) */

#define RCC_CR_HSION             (1 << 3)   /* HSI enable */
#define RCC_CR_HSIRDY            (1 << 2)   /* HSI ready */
#define RCC_CR_HSEON             (1 << 4)   /* HSE enable */
#define RCC_CR_PLL1ON            (1 << 8)   /* PLL1 enable */
#define RCC_CR_PLL2ON            (1 << 9)   /* PLL2 enable */
#define RCC_CR_PLL3ON            (1 << 10)  /* PLL3 enable */
#define RCC_CR_PLL4ON            (1 << 11)  /* PLL4 enable */

/* RCC_SR bits */

#define RCC_SR_HSIRDY            (1 << 3)   /* HSI ready flag */
#define RCC_SR_HSERDY            (1 << 4)   /* HSE ready flag */
#define RCC_SR_PLL1RDY           (1 << 8)   /* PLL1 ready flag */
#define RCC_SR_PLL2RDY           (1 << 9)   /* PLL2 ready flag */
#define RCC_SR_PLL3RDY           (1 << 10)  /* PLL3 ready flag */
#define RCC_SR_PLL4RDY           (1 << 11)  /* PLL4 ready flag */

/* RCC_CFGR1 bits: System clock mux SW[2:0] (bits 0-2) */

#define RCC_CFGR1_SW_MASK        (7 << 0)
#define RCC_CFGR1_SW_HSI         (0 << 0)   /* HSI as system clock */
#define RCC_CFGR1_SW_HSE         (1 << 0)   /* HSE as system clock */
#define RCC_CFGR1_SW_PLL1        (2 << 0)   /* PLL1 as system clock */

/* PLL1CFGR1 bits: PLL1 prescaler DIVM1[5:0] (bits 0-5), SRC[1:0] (8-9) */

#define RCC_PLL1CFGR1_DIVM1_MASK   (0x3f << 0)
#define RCC_PLL1CFGR1_DIVM1_SHIFT  0
#define RCC_PLL1CFGR1_PLL1SRC_MASK (3 << 8)
#define RCC_PLL1CFGR1_PLL1SRC_HSI  (0 << 8)
#define RCC_PLL1CFGR1_PLL1SRC_HSE  (1 << 8)

/* PLL1CFGR2 bits: PLL1 multiplier DIVN1[8:0] (bits 0-8) */

#define RCC_PLL1CFGR2_DIVN1_MASK   (0x1ff << 0)
#define RCC_PLL1CFGR2_DIVN1_SHIFT  0

/* PLL1CFGR3 bits: PLL1 output dividers */

#define RCC_PLL1CFGR3_DIVP1_MASK   (0x7f << 0)
#define RCC_PLL1CFGR3_DIVP1_SHIFT  0
#define RCC_PLL1CFGR3_DIVQ1_MASK   (0x7f << 8)
#define RCC_PLL1CFGR3_DIVQ1_SHIFT  8
#define RCC_PLL1CFGR3_DIVR1_MASK   (0x7f << 16)
#define RCC_PLL1CFGR3_DIVR1_SHIFT  16

/* PLL2CFGR1 bits */

#define RCC_PLL2CFGR1_DIVM2_MASK   (0x3f << 0)
#define RCC_PLL2CFGR1_DIVM2_SHIFT  0
#define RCC_PLL2CFGR1_PLL2SRC_MASK (3 << 8)
#define RCC_PLL2CFGR1_PLL2SRC_HSI  (0 << 8)
#define RCC_PLL2CFGR1_PLL2SRC_HSE  (1 << 8)

/* PLL2CFGR2 bits */

#define RCC_PLL2CFGR2_DIVN2_MASK   (0x1ff << 0)
#define RCC_PLL2CFGR2_DIVN2_SHIFT  0

/* PLL2CFGR3 bits */

#define RCC_PLL2CFGR3_DIVP2_MASK   (0x7f << 0)
#define RCC_PLL2CFGR3_DIVP2_SHIFT  0

/* PLL4CFGR1 bits */

#define RCC_PLL4CFGR1_DIVM4_MASK   (0x3f << 0)
#define RCC_PLL4CFGR1_DIVM4_SHIFT  0
#define RCC_PLL4CFGR1_PLL4SRC_MASK (3 << 8)
#define RCC_PLL4CFGR1_PLL4SRC_HSI  (0 << 8)
#define RCC_PLL4CFGR1_PLL4SRC_HSE  (1 << 8)

/* PLL4CFGR2 bits */

#define RCC_PLL4CFGR2_DIVN4_MASK   (0x1ff << 0)
#define RCC_PLL4CFGR2_DIVN4_SHIFT  0

/* PLL4CFGR3 bits */

#define RCC_PLL4CFGR3_DIVP4_MASK   (0x7f << 0)
#define RCC_PLL4CFGR3_DIVP4_SHIFT  0

/* AHB4ENR bits: GPIO port enables */

#define RCC_AHB4ENR_GPIOAEN      (1 << 0)
#define RCC_AHB4ENR_GPIOBEN      (1 << 1)
#define RCC_AHB4ENR_GPIOCEN      (1 << 2)
#define RCC_AHB4ENR_GPIODEN      (1 << 3)
#define RCC_AHB4ENR_GPIOEEN      (1 << 4)
#define RCC_AHB4ENR_GPIOFEN      (1 << 5)
#define RCC_AHB4ENR_GPIOGEN      (1 << 6)
#define RCC_AHB4ENR_GPIOHEN      (1 << 7)
#define RCC_AHB4ENR_GPIONEN      (1 << 12)
#define RCC_AHB4ENR_GPIOOEN      (1 << 13)
#define RCC_AHB4ENR_GPIOPEN      (1 << 14)
#define RCC_AHB4ENR_GPIOQEN      (1 << 15)
#define RCC_AHB4ENR_RCCEN        (1 << 27)

/* APB2ENR bits: USART1 enable */

#define RCC_APB2ENR_USART1EN     (1 << 4)

/* AHB3ENR bits: RNG enable */

#define RCC_AHB3ENR_RNGEN       (1 << 0)

/* AHB1ENR bits: DMA enables */

#define RCC_AHB1ENR_GPDMA1EN    (1 << 0)

/* APB1ENR bits: peripheral enables */

#define RCC_APB1ENR_USART2EN    (1 << 17)
#define RCC_APB1ENR_USART3EN    (1 << 18)
#define RCC_APB1ENR_UART4EN     (1 << 19)
#define RCC_APB1ENR_UART5EN     (1 << 20)
#define RCC_APB1ENR_I2C1EN      (1 << 21)
#define RCC_APB1ENR_I2C2EN      (1 << 22)
#define RCC_APB1ENR_IWDGEN      (1 << 24)
#define RCC_APB1ENR_RTCEN       (1 << 26)

/* AHB5ENR bits: XSPI/SDMMC enables */

#define RCC_AHB5ENR_XSPI1EN     (1 << 0)
#define RCC_AHB5ENR_XSPI2EN     (1 << 1)
#define RCC_AHB5ENR_SDMMC1EN    (1 << 4)

/* APB4ENR bits: EXTI enable */

#define RCC_APB4ENR_EXTIEN      (1 << 0)

/* APB2ENR additional bits */

#define RCC_APB2ENR_USART6EN    (1 << 5)
#define RCC_APB2ENR_UART9EN     (1 << 7)
#define RCC_APB2ENR_USART10EN   (1 << 8)

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_RCC_H */
