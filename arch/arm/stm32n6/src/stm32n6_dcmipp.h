/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_dcmipp.h
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
 * STM32N6 DCMIPP camera driver header.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_STM32N6_STM32N6_DCMIPP_H
#define __ARCH_ARM_SRC_STM32N6_STM32N6_DCMIPP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DCMIPP_MODE_CONTINUOUS   0
#define DCMIPP_MODE_SNAPSHOT     1

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize DCMIPP camera subsystem
 * @param width Display pipe width
 * @param height Display pipe height
 * @param fps Target frame rate
 * @return OK on success
 */

int stm32n6_dcmipp_init(uint32_t width, uint32_t height,
                          uint32_t fps);

/**
 * @brief Start camera capture on specified pipe
 * @param pipe 0=display, 1=NN
 * @param buffer Destination buffer
 * @param mode DCMIPP_MODE_CONTINUOUS or DCMIPP_MODE_SNAPSHOT
 * @return OK on success
 */

int stm32n6_dcmipp_start(uint32_t pipe, void *buffer,
                           uint32_t mode);

/**
 * @brief Stop camera capture on specified pipe
 * @param pipe 0=display, 1=NN
 * @return OK on success
 */

int stm32n6_dcmipp_stop(uint32_t pipe);

/**
 * @brief Run ISP auto-exposure/auto-white-balance update
 */

void stm32n6_dcmipp_isp_update(void);

/**
 * @brief Get frame count for specified pipe
 * @param pipe 0=display, 1=NN
 * @return Frame count
 */

uint32_t stm32n6_dcmipp_get_frame_count(uint32_t pipe);

/**
 * @brief Called from DCMIPP ISR when a frame is received
 * @param pipe 0=display, 1=NN
 */

void stm32n6_dcmipp_frame_event(uint32_t pipe);

#endif /* __ARCH_ARM_SRC_STM32N6_STM32N6_DCMIPP_H */
