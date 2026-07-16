/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_iwdg.h
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
 * STM32N6 IWDG driver header.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_STM32N6_STM32N6_IWDG_H
#define __ARCH_ARM_SRC_STM32N6_STM32N6_IWDG_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize IWDG watchdog
 * @param timeout_ms Watchdog timeout in milliseconds
 * @return 0 on success
 */

int stm32n6_iwdg_initialize(uint32_t timeout_ms);

/**
 * @brief Feed (kick) the watchdog to prevent reset
 */

void stm32n6_iwdg_feed(void);

/**
 * @brief Get configured timeout
 * @return Timeout in milliseconds
 */

uint32_t stm32n6_iwdg_get_timeout(void);

#endif /* __ARCH_ARM_SRC_STM32N6_STM32N6_IWDG_H */
