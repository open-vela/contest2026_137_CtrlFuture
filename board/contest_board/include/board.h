/****************************************************************************
 * vendor/openvela/boards/contest2026_137_board/include/board.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __BOARDS_CONTEST2026_137_BOARD_INCLUDE_BOARD_H
#define __BOARDS_CONTEST2026_137_BOARD_INCLUDE_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#ifndef __ASSEMBLY__
#  include <stdint.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_ARCH_CHIP_STM32N6

/* Clocking *****************************************************************/

/* Clock tree (PLL1 fed from internal HSI):
 *
 *   HSI 64 MHz / M=4 * N=50 = 800 MHz VCO
 *     IC1  /4 = 200 MHz  -> CPU clock (CPUSW)
 *     IC2  /8 = 100 MHz  \
 *     IC6 /12 = 66.7 MHz  > SYSCLK components (SYSSW IC2_IC6_IC11)
 *     IC11 /8 = 100 MHz  /
 *   HPRE /2  = 50 MHz   -> HCLK
 *   PPRE1 /1 = 50 MHz   -> PCLK1
 *   PPRE2 /1 = 50 MHz   -> PCLK2
 */

#define STM32_HSI_FREQUENCY     64000000ul

#define STM32_PLL1_M            4
#define STM32_PLL1_N            50
#define STM32_PLL1_IC1_DIV      4

#define STM32_CPUCLK_FREQUENCY  200000000ul
#define STM32_SYSCLK_FREQUENCY  (STM32_CPUCLK_FREQUENCY / 2)
#define STM32_HCLK_FREQUENCY    (STM32_CPUCLK_FREQUENCY / 4)
#define STM32_PCLK1_FREQUENCY   STM32_HCLK_FREQUENCY
#define STM32_PCLK2_FREQUENCY   STM32_HCLK_FREQUENCY

/* Timer input clock = SYSCLK (TIMPRE=0 default) */

#define STM32_APB1_TIM_FREQUENCY STM32_SYSCLK_FREQUENCY
#define STM32_APB2_TIM_FREQUENCY STM32_SYSCLK_FREQUENCY

/* I/O voltage domains ******************************************************/

#define BOARD_PWR_VDDIO  (PWR_SVMCR3_VDDIO2SV    | PWR_SVMCR3_VDDIO3SV | \
                          PWR_SVMCR3_VDDIO2VRSEL | PWR_SVMCR3_VDDIO3VRSEL)

/* Alternate function pin selections ****************************************/

/* USART1: PE5=TX (AF7), PE6=RX (AF7) — Virtual COM Port via ST-Link */

#define GPIO_USART1_TX   GPIO_USART1_TX_1
#define GPIO_USART1_RX   GPIO_USART1_RX_1

#endif /* CONFIG_ARCH_CHIP_STM32N6 */

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef CONFIG_ARCH_CHIP_STM32N6
void stm32_board_initialize(void);
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __BOARDS_CONTEST2026_137_BOARD_INCLUDE_BOARD_H */
