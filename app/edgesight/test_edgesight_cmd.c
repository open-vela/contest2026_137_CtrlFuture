/****************************************************************************
 * app/edgesight/test_edgesight_cmd.c
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
 * EdgeSight - Unit test for NSH command interface.
 * Verifies command dispatch and execution.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "edgesight_cmd.h"
#include "config.h"
#include "perf_stats.h"
#include "event_log.h"
#include "fall_detect.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_init(void)
{
  struct edgesight_cmd_context_s ctx;
  int ret;

  memset(&ctx, 0, sizeof(ctx));
  ret = edgesight_cmd_init(&ctx);

  if (ret < 0)
    {
      printf("  FAIL: init returned %d\n", ret);
      return -1;
    }

  printf("  PASS: init successful\n");
  return 0;
}

static int test_config_command(void)
{
  int ret;

  ret = edgesight_cmd_execute("config");

  if (ret < 0)
    {
      printf("  FAIL: config returned %d\n", ret);
      return -1;
    }

  printf("  PASS: config command executed\n");
  return 0;
}

static int test_unknown_command(void)
{
  int ret;

  ret = edgesight_cmd_execute("unknown_cmd_xyz");

  if (ret != -1)
    {
      printf("  FAIL: expected -1, got %d\n", ret);
      return -1;
    }

  printf("  PASS: unknown command returns -1\n");
  return 0;
}

static int test_help_command(void)
{
  int ret;

  ret = edgesight_cmd_execute("help");

  if (ret < 0)
    {
      printf("  FAIL: help returned %d\n", ret);
      return -1;
    }

  printf("  PASS: help command executed\n");
  return 0;
}

static int test_status_without_context(void)
{
  int ret;

  /* Status command with initialized but empty context */

  struct edgesight_cmd_context_s ctx;

  memset(&ctx, 0, sizeof(ctx));
  edgesight_cmd_init(&ctx);

  ret = edgesight_cmd_execute("status");

  if (ret < 0)
    {
      printf("  FAIL: status returned %d\n", ret);
      return -1;
    }

  printf("  PASS: status without context handled\n");
  return 0;
}

static int test_log_command(void)
{
  struct event_log_s log;
  struct edgesight_cmd_context_s ctx;

  event_log_init(&log, NULL);
  event_log_write(&log, EVENT_LEVEL_INFO, 1, "test entry");

  memset(&ctx, 0, sizeof(ctx));
  ctx.log = &log;
  edgesight_cmd_init(&ctx);

  int ret = edgesight_cmd_execute("log");

  if (ret < 0)
    {
      printf("  FAIL: log returned %d\n", ret);
      return -1;
    }

  printf("  PASS: log command with entries\n");
  return 0;
}

static int test_reset_command(void)
{
  struct fall_detector_s fall_ctx;
  struct edgesight_cmd_context_s ctx;

  fall_detect_init(&fall_ctx);

  memset(&ctx, 0, sizeof(ctx));
  ctx.fall_ctx = &fall_ctx;
  edgesight_cmd_init(&ctx);

  int ret = edgesight_cmd_execute("reset");

  if (ret < 0)
    {
      printf("  FAIL: reset returned %d\n", ret);
      return -1;
    }

  printf("  PASS: reset command executed\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_cmd_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("============================\n");
  printf(" EdgeSight Command Test\n");
  printf("============================\n\n");

  printf("[1] Init:\n");
  failures += (test_init() != 0);

  printf("\n[2] Config command:\n");
  failures += (test_config_command() != 0);

  printf("\n[3] Unknown command:\n");
  failures += (test_unknown_command() != 0);

  printf("\n[4] Help command:\n");
  failures += (test_help_command() != 0);

  printf("\n[5] Status without context:\n");
  failures += (test_status_without_context() != 0);

  printf("\n[6] Log command:\n");
  failures += (test_log_command() != 0);

  printf("\n[7] Reset command:\n");
  failures += (test_reset_command() != 0);

  printf("\n============================\n");
  if (failures == 0)
    {
      printf(" ALL TESTS PASSED\n");
    }
  else
    {
      printf(" %d TEST(S) FAILED\n", failures);
    }

  printf("============================\n");
  return failures;
}
