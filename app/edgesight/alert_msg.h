/****************************************************************************
 * app/edgesight/alert_msg.h
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
 * EdgeSight - MQTT alert message formatting.
 * Generates JSON payloads for fall detection alerts.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_ALERT_MSG_H
#define __APP_EDGESIGHT_ALERT_MSG_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ALERT_MSG_MAX          256
#define ALERT_TYPE_FALL        1
#define ALERT_TYPE_RECOVERY    2
#define ALERT_TYPE_HEARTBEAT   3

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct alert_payload_s
{
  uint8_t  type;
  float    confidence;
  float    torso_angle;
  float    cog_height;
  uint32_t frame_number;
  uint32_t timestamp_ms;
  uint32_t uptime_s;
  uint8_t  persons_in_scene;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Format a fall alert as JSON
 * @param buf Output buffer
 * @param buflen Buffer size
 * @param device_id Device identifier string
 * @param payload Alert data
 * @return Length of formatted string, -1 on error
 *
 * Output format:
 * {
 *   "device": "edgesight-001",
 *   "type": "fall",
 *   "ts": 1234567,
 *   "frame": 456,
 *   "confidence": 0.87,
 *   "angle": 62.3,
 *   "cog": 0.32,
 *   "persons": 1,
 *   "uptime": 3600
 * }
 */

int alert_msg_format(char *buf, uint16_t buflen,
                     const char *device_id,
                     const struct alert_payload_s *payload);

/**
 * @brief Format a heartbeat message as JSON
 * @param buf Output buffer
 * @param buflen Buffer size
 * @param device_id Device identifier string
 * @param uptime_s System uptime in seconds
 * @param frame_count Total frames processed
 * @param fall_count Total falls detected
 * @return Length of formatted string
 */

int alert_msg_heartbeat(char *buf, uint16_t buflen,
                        const char *device_id,
                        uint32_t uptime_s,
                        uint32_t frame_count,
                        uint32_t fall_count);

#endif /* __APP_EDGESIGHT_ALERT_MSG_H */
