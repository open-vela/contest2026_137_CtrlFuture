/****************************************************************************
 * app/edgesight/edgesight_cmd.c
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
 * EdgeSight - NSH command interface implementation.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "edgesight_cmd.h"
#include "config.h"
#include "perf_stats.h"
#include "event_log.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct edgesight_cmd_context_s *g_cmd_ctx;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void cmd_status(void)
{
  if (g_cmd_ctx == NULL || g_cmd_ctx->running == NULL)
    {
      printf("EdgeSight: not initialized\n");
      return;
    }

  printf("=== EdgeSight Status ===\n");
  printf("State: %s\n",
         *g_cmd_ctx->running ? "RUNNING" : "STOPPED");
  printf("Frames: %lu\n",
         (unsigned long)*g_cmd_ctx->frame_count);
  printf("Detections: %lu\n",
         (unsigned long)*g_cmd_ctx->detect_count);
  printf("Falls: %lu\n",
         (unsigned long)*g_cmd_ctx->fall_count);
  printf("Recording: %s\n",
         *g_cmd_ctx->recording ? "YES" : "NO");

  if (g_cmd_ctx->perf != NULL)
    {
      printf("FPS: %lu\n",
             (unsigned long)perf_stats_get_fps(
                 g_cmd_ctx->perf));
    }

  if (g_cmd_ctx->log != NULL)
    {
      uint32_t total;
      uint32_t flushed;
      uint16_t dropped;

      event_log_stats(g_cmd_ctx->log,
                      &total, &flushed, &dropped);
      printf("Log: %lu total, %u dropped\n",
             (unsigned long)total, dropped);
    }
}

static void cmd_config(void)
{
  struct edgesight_config_s cfg;

  config_set_defaults(&cfg);
  config_load_file(&cfg, NULL);
  config_dump(&cfg);
}

static void cmd_log(void)
{
  if (g_cmd_ctx == NULL || g_cmd_ctx->log == NULL)
    {
      printf("No log available\n");
      return;
    }

  event_log_dump(g_cmd_ctx->log, 20);
}

static void cmd_reset(void)
{
  if (g_cmd_ctx == NULL)
    {
      printf("EdgeSight not initialized\n");
      return;
    }

  if (g_cmd_ctx->fall_ctx != NULL)
    {
      fall_detect_reset(g_cmd_ctx->fall_ctx);
      printf("Fall detector reset\n");
    }
}

static void cmd_help(void)
{
  printf("EdgeSight commands:\n");
  printf("  edgesight start    - Start EdgeSight pipeline\n");
  printf("  edgesight status   - Show runtime status\n");
  printf("  edgesight config   - Show configuration\n");
  printf("  edgesight log      - Show recent events\n");
  printf("  edgesight reset    - Reset fall detector\n");
  printf("  edgesight help     - Show this help\n");
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int edgesight_cmd_init(struct edgesight_cmd_context_s *ctx)
{
  g_cmd_ctx = ctx;
  return 0;
}

int edgesight_cmd_execute(const char *cmd)
{
  if (cmd == NULL)
    {
      return -1;
    }

  if (strcmp(cmd, "status") == 0)
    {
      cmd_status();
    }
  else if (strcmp(cmd, "config") == 0)
    {
      cmd_config();
    }
  else if (strcmp(cmd, "log") == 0)
    {
      cmd_log();
    }
  else if (strcmp(cmd, "reset") == 0)
    {
      cmd_reset();
    }
  else if (strcmp(cmd, "help") == 0)
    {
      cmd_help();
    }
  else
    {
      printf("Unknown command: %s\n", cmd);
      cmd_help();
      return -1;
    }

  return 0;
}
