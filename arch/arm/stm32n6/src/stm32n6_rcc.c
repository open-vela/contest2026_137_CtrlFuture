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
#include "hardware/stm32_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* RCC register offsets (subset for minimum boot) */

#define STM32_RCC_CR_OFFSET         0x0000
#define STM32_RCC_CFGR1_OFFSET      0x0018
#define STM32_RCC_APB2ENR_OFFSET    0x026c
#define STM32_RCC_AHB4ENR_OFFSET    0x025c

#define STM32_RCC_CR             (STM32_RCC_BASE + STM32_RCC_CR_OFFSET)
#define STM32_RCC_APB2ENR        (STM32_RCC_BASE + STM32_RCC_APB2ENR_OFFSET)
#define STM32_RCC_AHB4ENR        (STM32_RCC_BASE + STM32_RCC_AHB4ENR_OFFSET)

/* RCC_CR bits */

#define RCC_CR_HSION             (1 << 0)
#define RCC_CR_HSIRDY            (1 << 2)

/* RCC_APB2ENR bits */

#define RCC_APB2ENR_USART1EN     (1 << 4)

/* RCC_AHB4ENR bits */

#define RCC_AHB4ENR_GPIOEEN      (1 << 4)

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline void rcc_enablehsi(void)
{
  uint32_t regval;

  regval  = getreg32(STM32_RCC_CR);
  regval |= RCC_CR_HSION;
  putreg32(regval, STM32_RCC_CR);

  while ((getreg32(STM32_RCC_CR) & RCC_CR_HSIRDY) == 0)
    {
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_clockconfig
 *
 * Description:
 *   Minimum clock setup: enable HSI 64 MHz as system clock.
 *   PLL configuration will be added when more peripherals need it.
 *
 ****************************************************************************/

void stm32n6_clockconfig(void)
{
  uint32_t regval;

  rcc_enablehsi();

  /* Enable GPIOE clock (for USART1 pins PE5/PE6) */

  regval  = getreg32(STM32_RCC_AHB4ENR);
  regval |= RCC_AHB4ENR_GPIOEEN;
  putreg32(regval, STM32_RCC_AHB4ENR);

  /* Enable USART1 clock */

  regval  = getreg32(STM32_RCC_APB2ENR);
  regval |= RCC_APB2ENR_USART1EN;
  putreg32(regval, STM32_RCC_APB2ENR);
}
