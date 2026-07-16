/****************************************************************************
 * app/edgesight/sensor_fusion.h
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
 * EdgeSight - Multi-modal sensor fusion engine.
 * Combines vision, audio, and environmental sensors for decision making.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_SENSOR_FUSION_H
#define __APP_EDGESIGHT_SENSOR_FUSION_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define FUSION_INPUT_VISION    0
#define FUSION_INPUT_AUDIO     1
#define FUSION_INPUT_PIR       2
#define FUSION_INPUT_SMOKE     3
#define FUSION_INPUT_TEMP      4
#define FUSION_INPUT_COUNT     5

#define FUSION_ALERT_NONE      0
#define FUSION_ALERT_FALL      1
#define FUSION_ALERT_INTRUSION 2
#define FUSION_ALERT_FIRE      3
#define FUSION_ALERT_SOUND     4

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Individual sensor input */

struct fusion_input_s
{
  uint8_t  type;          /* FUSION_INPUT_xxx */
  float    confidence;    /* 0.0 - 1.0 */
  float    value;         /* Sensor-specific value */
  uint32_t timestamp_ms;
  bool     valid;
};

/* Fusion decision output */

struct fusion_decision_s
{
  uint8_t  alert_type;    /* FUSION_ALERT_xxx */
  float    confidence;    /* Combined confidence */
  uint8_t  sources;       /* Bitmask of contributing sensors */
  char     description[64];
};

/* Fusion context */

struct fusion_context_s
{
  struct fusion_input_s inputs[FUSION_INPUT_COUNT];
  uint32_t last_update_ms;
  uint32_t decision_count;
  uint8_t  last_alert_type;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize sensor fusion engine
 * @param ctx Fusion context
 */

void fusion_init(struct fusion_context_s *ctx);

/**
 * @brief Update a sensor input
 * @param ctx Fusion context
 * @param input Sensor input data
 */

void fusion_update_input(struct fusion_context_s *ctx,
                         const struct fusion_input_s *input);

/**
 * @brief Run fusion decision
 * @param ctx Fusion context
 * @param decision Output decision
 * @return true if alert triggered
 */

bool fusion_decide(struct fusion_context_s *ctx,
                   struct fusion_decision_s *decision);

/**
 * @brief Get human-readable alert type string
 * @param alert_type FUSION_ALERT_xxx
 * @return String name
 */

const char *fusion_alert_str(uint8_t alert_type);

#endif /* __APP_EDGESIGHT_SENSOR_FUSION_H */
