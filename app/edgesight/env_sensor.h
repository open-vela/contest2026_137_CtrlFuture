/****************************************************************************
 * app/edgesight/env_sensor.h
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
 * EdgeSight - Environmental sensor driver.
 * Reads temperature, humidity, smoke, and PIR motion sensors.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_ENV_SENSOR_H
#define __APP_EDGESIGHT_ENV_SENSOR_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ENV_SENSOR_TEMP      0
#define ENV_SENSOR_HUMIDITY  1
#define ENV_SENSOR_SMOKE     2
#define ENV_SENSOR_PIR       3
#define ENV_SENSOR_COUNT     4

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct env_sensor_data_s
{
  float    temperature;     /* Celsius */
  float    humidity;        /* Percentage 0-100 */
  float    smoke_level;     /* ADC normalized 0-1 */
  bool     pir_motion;      /* PIR motion detected */
  uint32_t timestamp_ms;
  bool     valid[ENV_SENSOR_COUNT];
};

struct env_sensor_context_s
{
  bool initialized;
  int  i2c_fd;          /* I2C file descriptor for SHT30 */
  int  adc_fd;          /* ADC file descriptor for smoke */
  int  pir_gpio;        /* GPIO pin for PIR */
  struct env_sensor_data_s last_data;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize environmental sensors
 * @param ctx Sensor context
 * @return 0 on success
 */

int env_sensor_init(struct env_sensor_context_s *ctx);

/**
 * @brief Read all environmental sensors
 * @param ctx Sensor context
 * @param data Output sensor data
 * @return 0 on success
 */

int env_sensor_read(struct env_sensor_context_s *ctx,
                    struct env_sensor_data_s *data);

/**
 * @brief Get last valid reading
 * @param ctx Sensor context
 * @return Pointer to last sensor data
 */

const struct env_sensor_data_s *env_sensor_get_last(
    const struct env_sensor_context_s *ctx);

/**
 * @brief Deinitialize sensors
 */

void env_sensor_deinit(struct env_sensor_context_s *ctx);

#endif /* __APP_EDGESIGHT_ENV_SENSOR_H */
