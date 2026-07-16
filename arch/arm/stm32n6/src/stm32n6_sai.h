/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_sai.h
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
 * STM32N6 SAI driver header.
 * Serial Audio Interface for microphone input and speaker output.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_STM32N6_STM32N6_SAI_H
#define __ARCH_ARM_SRC_STM32N6_STM32N6_SAI_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* SAI audio format */

struct sai_config_s
{
  uint32_t sample_rate;    /* e.g. 16000, 44100, 48000 */
  uint8_t bits_per_sample; /* 16 or 24 */
  uint8_t  channels;       /* 1=mono, 2=stereo */
  uint8_t  block_count;    /* DMA block count (2=double buffer) */
  uint16_t block_samples;  /* Samples per block */
};

/* SAI buffer callback */

typedef void (*sai_buffer_cb_t)(void *buffer, uint32_t size,
                                 void *arg);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize SAI for audio input
 * @param config Audio configuration
 * @return 0 on success
 */

int stm32n6_sai_input_init(const struct sai_config_s *config);

/**
 * @brief Start audio capture
 * @param callback Buffer-ready callback
 * @param arg Callback argument
 * @return 0 on success
 */

int stm32n6_sai_input_start(sai_buffer_cb_t callback,
                              void *arg);

/**
 * @brief Stop audio capture
 * @return 0 on success
 */

int stm32n6_sai_input_stop(void);

/**
 * @brief Deinitialize SAI input
 */

void stm32n6_sai_input_deinit(void);

#endif /* __ARCH_ARM_SRC_STM32N6_STM32N6_SAI_H */
