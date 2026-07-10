/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_rcc.c
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
#include "stm32n6_rcc.h"
#include "hardware/stm32_rcc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Default clock frequencies (HSI mode, no PLL) */

#define STM32_HSI_FREQUENCY      64000000ul  /* 64 MHz internal RC */

/* PLL1 configuration: HSE 32MHz -> 800MHz VCO
 * DIVM=1 (prescaler=1), DIVN=25 (mult=25), DIVP=1 (no division)
 * VCO = 32MHz / 1 * 25 = 800MHz
 * PLL1P = 800MHz / 1 = 800MHz
 * Conservative default: AHB DIV4 -> 200MHz SYSCLK
 */

#define PLL1_DIVM               1
#define PLL1_DIVN               25
#define PLL1_DIVP               1

/* AHB prescaler: SYSCLK / DIV4 for conservative 200MHz */

#define RCC_CFGR1_HPRE_DIV4     (9 << 4)   /* AHB = SYSCLK / 4 */

/* Timeout for clock ready flags (100ms at ~64MHz loop rate) */

#define CLOCK_READY_TIMEOUT     (100 * CONFIG_BOARD_LOOPSPERMSEC)

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline void rcc_enablehsi(void)
{
  uint32_t regval;

  /* Set HSION in CR (bit 3) */

  regval  = getreg32(STM32_RCC_CR);
  regval |= RCC_CR_HSION;
  putreg32(regval, STM32_RCC_CR);

  /* Wait for HSIRDY in SR (bit 3) */

  while ((getreg32(STM32_RCC_SR) & RCC_SR_HSIRDY) == 0)
    {
    }
}

#ifdef CONFIG_STM32N6_USE_HSE
static inline int rcc_enablehse(void)
{
  uint32_t regval;
  volatile int timeout;

  /* Set HSEON in CR (bit 4) */

  regval  = getreg32(STM32_RCC_CR);
  regval |= RCC_CR_HSEON;
  putreg32(regval, STM32_RCC_CR);

  /* Wait for HSERDY in SR (bit 4) with timeout */

  timeout = CLOCK_READY_TIMEOUT;

  while ((getreg32(STM32_RCC_SR) & RCC_SR_HSERDY) == 0)
    {
      if (--timeout <= 0)
        {
          return -1;
        }
    }

  return 0;
}
#endif

#ifdef CONFIG_STM32N6_USE_PLL1
static inline int rcc_configpll1(void)
{
  uint32_t regval;
  volatile int timeout;

  /* Configure PLL1 source and prescaler: HSE, DIVM */

  regval = (RCC_PLL1CFGR1_PLL1SRC_HSE) |
           (PLL1_DIVM << RCC_PLL1CFGR1_DIVM1_SHIFT);
  putreg32(regval, STM32_RCC_PLL1CFGR1);

  /* Configure PLL1 multiplier: DIVN */

  regval = (PLL1_DIVN - 1) << RCC_PLL1CFGR2_DIVN1_SHIFT;
  putreg32(regval, STM32_RCC_PLL1CFGR2);

  /* Configure PLL1 output divider: DIVP */

  regval = (PLL1_DIVP - 1) << RCC_PLL1CFGR3_DIVP1_SHIFT;
  putreg32(regval, STM32_RCC_PLL1CFGR3);

  /* Enable PLL1 */

  regval  = getreg32(STM32_RCC_CR);
  regval |= RCC_CR_PLL1ON;
  putreg32(regval, STM32_RCC_CR);

  /* Wait for PLL1RDY with timeout */

  timeout = CLOCK_READY_TIMEOUT;

  while ((getreg32(STM32_RCC_SR) & RCC_SR_PLL1RDY) == 0)
    {
      if (--timeout <= 0)
        {
          return -1;
        }
    }

  return 0;
}

static inline void rcc_switchsysclk(void)
{
  uint32_t regval;

  /* Set system clock source to PLL1 */

  regval  = getreg32(STM32_RCC_CFGR1);
  regval &= ~RCC_CFGR1_SW_MASK;
  regval |= RCC_CFGR1_SW_PLL1;

  /* Set AHB prescaler for conservative 200MHz (800MHz / 4) */

  regval &= ~(0xf << 4);
  regval |= RCC_CFGR1_HPRE_DIV4;
  putreg32(regval, STM32_RCC_CFGR1);
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_clockconfig
 *
 * Description:
 *   Configure system clock. If PLL1 is enabled via Kconfig, this sets up
 *   HSE -> PLL1 -> 800MHz with AHB DIV4 (200MHz conservative default).
 *   Otherwise falls back to HSI 64MHz.
 *
 ****************************************************************************/

void stm32n6_clockconfig(void)
{
  uint32_t regval;

  /* Always start with HSI as fallback */

  rcc_enablehsi();

#ifdef CONFIG_STM32N6_USE_HSE
  /* Enable HSE and wait for ready */

  if (rcc_enablehse() < 0)
    {
      /* HSE failed, stay on HSI */

      goto enable_peripherals;
    }
#endif

#ifdef CONFIG_STM32N6_USE_PLL1
  /* Configure and enable PLL1, switch system clock */

  if (rcc_configpll1() < 0)
    {
      /* PLL1 failed, stay on HSI */

      goto enable_peripherals;
    }

  rcc_switchsysclk();
#endif

#if defined(CONFIG_STM32N6_USE_HSE) || defined(CONFIG_STM32N6_USE_PLL1)
enable_peripherals:
#endif

  /* Enable GPIOE clock (for USART1 pins PE5/PE6) */

  regval  = getreg32(STM32_RCC_AHB4ENR);
  regval |= RCC_AHB4ENR_GPIOEEN;
  putreg32(regval, STM32_RCC_AHB4ENR);

  /* Enable USART1 clock */

  regval  = getreg32(STM32_RCC_APB2ENR);
  regval |= RCC_APB2ENR_USART1EN;
  putreg32(regval, STM32_RCC_APB2ENR);
}

/****************************************************************************
 * Name: stm32n6_get_sysclk
 *
 * Description:
 *   Return the current SYSCLK frequency in Hz.
 *
 ****************************************************************************/

uint32_t stm32n6_get_sysclk(void)
{
#ifdef CONFIG_STM32N6_USE_PLL1
  return 800000000ul / 4;  /* PLL1 800MHz / AHB DIV4 = 200MHz */
#else
  return STM32_HSI_FREQUENCY;
#endif
}

/****************************************************************************
 * Name: stm32n6_get_hclk
 *
 * Description:
 *   Return the AHB bus (HCLK) frequency in Hz.
 *
 ****************************************************************************/

uint32_t stm32n6_get_hclk(void)
{
  return stm32n6_get_sysclk();  /* Same as SYSCLK for now */
}

/****************************************************************************
 * Name: stm32n6_get_pclk1
 *
 * Description:
 *   Return the APB1 bus frequency in Hz.
 *
 ****************************************************************************/

uint32_t stm32n6_get_pclk1(void)
{
  return stm32n6_get_hclk();  /* APB1 = HCLK for now */
}

/****************************************************************************
 * Name: stm32n6_get_pclk2
 *
 * Description:
 *   Return the APB2 bus frequency in Hz.
 *
 ****************************************************************************/

uint32_t stm32n6_get_pclk2(void)
{
  return stm32n6_get_hclk();  /* APB2 = HCLK for now */
}
