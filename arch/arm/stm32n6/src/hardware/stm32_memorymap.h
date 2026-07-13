/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32_memorymap.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_MEMORYMAP_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_MEMORYMAP_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* STM32N6 Memory Map (non-secure aliases) */

#define STM32_SRAM_BASE        0x34000000ul
#define STM32_PERIPH_BASE      0x40000000ul

/* Bus base addresses */

#define STM32_APB1_BASE        (STM32_PERIPH_BASE + 0x00000000ul)
#define STM32_AHB1_BASE        (STM32_PERIPH_BASE + 0x00020000ul)
#define STM32_APB2_BASE        (STM32_PERIPH_BASE + 0x02000000ul)
#define STM32_AHB2_BASE        (STM32_PERIPH_BASE + 0x02020000ul)
#define STM32_APB3_BASE        (STM32_PERIPH_BASE + 0x04000000ul)
#define STM32_AHB3_BASE        (STM32_PERIPH_BASE + 0x04020000ul)
#define STM32_APB4_BASE        (STM32_PERIPH_BASE + 0x06000000ul)
#define STM32_AHB4_BASE        (STM32_PERIPH_BASE + 0x06020000ul)
#define STM32_APB5_BASE        (STM32_PERIPH_BASE + 0x08000000ul)
#define STM32_AHB5_BASE        (STM32_PERIPH_BASE + 0x08020000ul)

/* APB1 peripherals */

#define STM32_USART2_BASE      (STM32_APB1_BASE + 0x4400)
#define STM32_USART3_BASE      (STM32_APB1_BASE + 0x4800)
#define STM32_UART4_BASE       (STM32_APB1_BASE + 0x4c00)
#define STM32_UART5_BASE       (STM32_APB1_BASE + 0x5000)
#define STM32_UART7_BASE       (STM32_APB1_BASE + 0x7800)
#define STM32_UART8_BASE       (STM32_APB1_BASE + 0x7c00)

/* APB2 peripherals */

#define STM32_USART1_BASE      (STM32_APB2_BASE + 0x1000)
#define STM32_USART6_BASE      (STM32_APB2_BASE + 0x1400)
#define STM32_UART9_BASE       (STM32_APB2_BASE + 0x1800)
#define STM32_USART10_BASE     (STM32_APB2_BASE + 0x1c00)

/* APB4 peripherals */

#define STM32_LPUART1_BASE     (STM32_APB4_BASE + 0x0c00)

/* AHB4 peripherals */

#define STM32_GPIOA_BASE       (STM32_AHB4_BASE + 0x0000)
#define STM32_GPIOB_BASE       (STM32_AHB4_BASE + 0x0400)
#define STM32_GPIOC_BASE       (STM32_AHB4_BASE + 0x0800)
#define STM32_GPIOD_BASE       (STM32_AHB4_BASE + 0x0c00)
#define STM32_GPIOE_BASE       (STM32_AHB4_BASE + 0x1000)
#define STM32_GPIOF_BASE       (STM32_AHB4_BASE + 0x1400)
#define STM32_GPIOG_BASE       (STM32_AHB4_BASE + 0x1800)
#define STM32_GPIOH_BASE       (STM32_AHB4_BASE + 0x1c00)
#define STM32_GPIOI_BASE       (STM32_AHB4_BASE + 0x2000)
#define STM32_GPIOJ_BASE       (STM32_AHB4_BASE + 0x2400)
#define STM32_GPIOZ_BASE       (STM32_AHB4_BASE + 0x4400)
#define STM32_GPION_BASE       (STM32_AHB4_BASE + 0x3400)
#define STM32_GPIOO_BASE       (STM32_AHB4_BASE + 0x3800)
#define STM32_GPIOP_BASE       (STM32_AHB4_BASE + 0x3c00)
#define STM32_GPIOQ_BASE       (STM32_AHB4_BASE + 0x4000)
#define STM32_RCC_BASE         (STM32_AHB4_BASE + 0x8000)

/* AHB3 peripherals */

#define STM32_RNG_BASE         (STM32_AHB3_BASE + 0x0000)

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_MEMORYMAP_H */
