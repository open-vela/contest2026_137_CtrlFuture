/****************************************************************************
 * arch/arm/src/stm32n6/hardware/stm32_lptim.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_LPTIM_H
#define __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_LPTIM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "hardware/stm32_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register offsets *********************************************************/

#define STM32_LPTIM_ISR_OFFSET    0x0000  /* Interrupt and status register */
#define STM32_LPTIM_ICR_OFFSET    0x0004  /* Interrupt clear register */
#define STM32_LPTIM_DIER_OFFSET   0x0008  /* Interrupt enable register */
#define STM32_LPTIM_CFGR_OFFSET   0x000c  /* Configuration register */
#define STM32_LPTIM_CR_OFFSET     0x0010  /* Control register */
#define STM32_LPTIM_CCR1_OFFSET   0x0014  /* Capture/compare register 1 */
#define STM32_LPTIM_ARR_OFFSET    0x0018  /* Autoreload register */
#define STM32_LPTIM_CNT_OFFSET    0x001c  /* Counter register */

/* Register addresses *******************************************************/

#define STM32_LPTIM1_ISR   (STM32_LPTIM1_BASE + STM32_LPTIM_ISR_OFFSET)
#define STM32_LPTIM1_ICR   (STM32_LPTIM1_BASE + STM32_LPTIM_ICR_OFFSET)
#define STM32_LPTIM1_DIER  (STM32_LPTIM1_BASE + STM32_LPTIM_DIER_OFFSET)
#define STM32_LPTIM1_CFGR  (STM32_LPTIM1_BASE + STM32_LPTIM_CFGR_OFFSET)
#define STM32_LPTIM1_CR    (STM32_LPTIM1_BASE + STM32_LPTIM_CR_OFFSET)
#define STM32_LPTIM1_ARR   (STM32_LPTIM1_BASE + STM32_LPTIM_ARR_OFFSET)
#define STM32_LPTIM1_CNT   (STM32_LPTIM1_BASE + STM32_LPTIM_CNT_OFFSET)

/* Register bit definitions *************************************************/

/* Interrupt and status register (ISR) */

#define LPTIM_ISR_CC1IF   (1 << 0)   /* Capture/compare 1 interrupt flag */
#define LPTIM_ISR_ARRM    (1 << 1)   /* Autoreload match */
#define LPTIM_ISR_ARROK   (1 << 4)   /* Autoreload register update OK */

/* Interrupt clear register (ICR) */

#define LPTIM_ICR_ARRMCF  (1 << 1)   /* Autoreload match clear flag */
#define LPTIM_ICR_ARROKCF (1 << 4)   /* Autoreload update OK clear flag */

/* Interrupt enable register (DIER) */

#define LPTIM_DIER_ARRMIE (1 << 1)   /* Autoreload match interrupt enable */

/* Configuration register (CFGR) */

#define LPTIM_CFGR_CKSEL    (1 << 0)   /* Clock selector (0 = internal) */
#define LPTIM_CFGR_PRESC_SHIFT 9       /* Clock prescaler PRESC[2:0] */
#define LPTIM_CFGR_PRESC_MASK  (0x7 << LPTIM_CFGR_PRESC_SHIFT)
#define LPTIM_CFGR_PRELOAD  (1 << 22)  /* ARR/CCR update mode (1 = at UE) */

/* Control register (CR) */

#define LPTIM_CR_ENABLE   (1 << 0)   /* LPTIM enable */
#define LPTIM_CR_SNGSTRT  (1 << 1)   /* Start in single mode */
#define LPTIM_CR_CNTSTRT  (1 << 2)   /* Start in continuous mode */

#endif /* __ARCH_ARM_SRC_STM32N6_HARDWARE_STM32_LPTIM_H */
