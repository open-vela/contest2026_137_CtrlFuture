/****************************************************************************
 * app/edgesight/event_log.h
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
 * EdgeSight - Event logging system.
 * Ring buffer in SRAM with periodic flush to SD card.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_EVENT_LOG_H
#define __APP_EDGESIGHT_EVENT_LOG_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define EVENT_LOG_RING_SIZE    64
#define EVENT_LOG_MSG_MAX      80
#define EVENT_LOG_PATH_MAX     64

/****************************************************************************
 * Public Types
 ****************************************************************************/

enum event_level_e
{
  EVENT_LEVEL_DEBUG = 0,
  EVENT_LEVEL_INFO,
  EVENT_LEVEL_WARN,
  EVENT_LEVEL_ALERT,
  EVENT_LEVEL_ERROR
};

struct event_entry_s
{
  uint32_t timestamp_ms;
  uint32_t frame_number;
  uint8_t  level;
  char     message[EVENT_LOG_MSG_MAX];
};

struct event_log_s
{
  struct event_entry_s ring[EVENT_LOG_RING_SIZE];
  uint16_t head;
  uint16_t tail;
  uint16_t count;
  uint16_t dropped;
  uint32_t total_logged;
  uint32_t total_flushed;
  bool     sd_available;
  char     log_path[EVENT_LOG_PATH_MAX];
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize event logging system
 * @param log Log context
 * @param sd_path SD card log directory (NULL = no SD)
 * @return 0 on success
 */

int event_log_init(struct event_log_s *log,
                   const char *sd_path);

/**
 * @brief Log an event
 * @param log Log context
 * @param level Event severity level
 * @param frame Current frame number
 * @param fmt Printf-style format string
 * @return 0 on success, -1 if ring full (dropped)
 */

int event_log_write(struct event_log_s *log,
                    enum event_level_e level,
                    uint32_t frame,
                    const char *fmt, ...);

/**
 * @brief Flush pending events to SD card
 * @param log Log context
 * @return Number of entries flushed, -1 on error
 */

int event_log_flush(struct event_log_s *log);

/**
 * @brief Get logging statistics
 * @param log Log context
 * @param total_logged Output: total events logged
 * @param total_flushed Output: total events written to SD
 * @param dropped Output: events dropped (ring full)
 */

void event_log_stats(const struct event_log_s *log,
                     uint32_t *total_logged,
                     uint32_t *total_flushed,
                     uint16_t *dropped);

/**
 * @brief Dump recent events to console (for debug)
 * @param log Log context
 * @param max_entries Max entries to dump (0 = all in ring)
 */

void event_log_dump(const struct event_log_s *log,
                    uint16_t max_entries);

#endif /* __APP_EDGESIGHT_EVENT_LOG_H */
