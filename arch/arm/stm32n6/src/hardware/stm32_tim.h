/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32_tim.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_TIM_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_TIM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "hardware/stm32_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets (common to all general-purpose timers) ******************/

#define STM32_TIM_CR1_OFFSET     0x0000  /* Control register 1 */
#define STM32_TIM_CR2_OFFSET     0x0004  /* Control register 2 */
#define STM32_TIM_SMCR_OFFSET    0x0008  /* Slave mode control register */
#define STM32_TIM_DIER_OFFSET    0x000c  /* DMA/interrupt enable register */
#define STM32_TIM_SR_OFFSET      0x0010  /* Status register */
#define STM32_TIM_EGR_OFFSET     0x0014  /* Event generation register */
#define STM32_TIM_CNT_OFFSET     0x0024  /* Counter register */
#define STM32_TIM_PSC_OFFSET     0x0028  /* Prescaler register */
#define STM32_TIM_ARR_OFFSET     0x002c  /* Auto-reload register */

/* Register Addresses *******************************************************/

/* TIM2 (32-bit general-purpose timer) on APB1 @ 0x40000000 */

#define STM32_TIM2_CR1           (STM32_TIM2_BASE + STM32_TIM_CR1_OFFSET)
#define STM32_TIM2_CR2           (STM32_TIM2_BASE + STM32_TIM_CR2_OFFSET)
#define STM32_TIM2_SMCR          (STM32_TIM2_BASE + STM32_TIM_SMCR_OFFSET)
#define STM32_TIM2_DIER          (STM32_TIM2_BASE + STM32_TIM_DIER_OFFSET)
#define STM32_TIM2_SR            (STM32_TIM2_BASE + STM32_TIM_SR_OFFSET)
#define STM32_TIM2_EGR           (STM32_TIM2_BASE + STM32_TIM_EGR_OFFSET)
#define STM32_TIM2_CNT           (STM32_TIM2_BASE + STM32_TIM_CNT_OFFSET)
#define STM32_TIM2_PSC           (STM32_TIM2_BASE + STM32_TIM_PSC_OFFSET)
#define STM32_TIM2_ARR           (STM32_TIM2_BASE + STM32_TIM_ARR_OFFSET)

/* Register Bit Definitions *************************************************/

/* Control register 1 (CR1) */

#define TIM_CR1_CEN              (1 << 0)  /* Counter enable */
#define TIM_CR1_UDIS             (1 << 1)  /* Update disable */
#define TIM_CR1_URS              (1 << 2)  /* Update request source */
#define TIM_CR1_OPM              (1 << 3)  /* One-pulse mode */
#define TIM_CR1_ARPE             (1 << 7)  /* Auto-reload preload enable */

/* DMA/interrupt enable register (DIER) */

#define TIM_DIER_UIE             (1 << 0)  /* Update interrupt enable */

/* Status register (SR) */

#define TIM_SR_UIF               (1 << 0)  /* Update interrupt flag */

/* Event generation register (EGR) */

#define TIM_EGR_UG               (1 << 0)  /* Update generation */

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_TIM_H */
