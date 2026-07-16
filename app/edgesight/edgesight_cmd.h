/****************************************************************************
 * app/edgesight/edgesight_cmd.h
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
 * EdgeSight - NSH command interface header.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_CMD_H
#define __APP_EDGESIGHT_CMD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "fall_detect.h"
#include "perf_stats.h"
#include "event_log.h"

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct edgesight_cmd_context_s
{
  volatile bool *running;
  volatile bool *recording;
  uint32_t *frame_count;
  uint32_t *detect_count;
  uint32_t *fall_count;
  struct fall_detector_s *fall_ctx;
  struct perf_stats_s *perf;
  struct event_log_s *log;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int edgesight_cmd_init(struct edgesight_cmd_context_s *ctx);

int edgesight_cmd_execute(const char *cmd);

#endif /* __APP_EDGESIGHT_CMD_H */
