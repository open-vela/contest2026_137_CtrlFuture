/****************************************************************************
 * app/edgesight/test_config.c
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
 * EdgeSight - Unit test for configuration module.
 * Verifies config parsing, defaults, and save/load cycle.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "config.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_defaults(void)
{
  struct edgesight_config_s cfg;

  config_set_defaults(&cfg);

  if (cfg.fall.torso_angle_thresh != 45.0f)
    {
      printf("  FAIL: default torso_angle=%.1f "
             "(expected 45.0)\n",
             (double)cfg.fall.torso_angle_thresh);
      return -1;
    }

  if (cfg.fall.consecutive_frames != 3)
    {
      printf("  FAIL: default frames=%u (expected 3)\n",
             cfg.fall.consecutive_frames);
      return -1;
    }

  if (cfg.record.max_duration_s != 30)
    {
      printf("  FAIL: default duration=%lu (expected 30)\n",
             (unsigned long)cfg.record.max_duration_s);
      return -1;
    }

  if (cfg.network.mqtt_port != 1883)
    {
      printf("  FAIL: default port=%u (expected 1883)\n",
             cfg.network.mqtt_port);
      return -1;
    }

  printf("  PASS: defaults correct\n");
  return 0;
}

static int test_save_load_cycle(void)
{
  struct edgesight_config_s save_cfg;
  struct edgesight_config_s load_cfg;
  char test_path[] = "/tmp/edgesight_test.cfg";
  int ret;

  /* Set non-default values */

  config_set_defaults(&save_cfg);
  save_cfg.fall.torso_angle_thresh = 60.0f;
  save_cfg.fall.consecutive_frames = 5;
  save_cfg.record.max_duration_s = 60;
  save_cfg.network.mqtt_port = 8883;
  strncpy(save_cfg.network.mqtt_broker, "10.0.0.1", 47);

  /* Save */

  ret = config_save_file(&save_cfg, test_path);
  if (ret < 0)
    {
      printf("  FAIL: save_file returned %d\n", ret);
      return -1;
    }

  /* Load into different struct */

  config_set_defaults(&load_cfg);
  ret = config_load_file(&load_cfg, test_path);
  if (ret < 0)
    {
      printf("  FAIL: load_file returned %d\n", ret);
      return -1;
    }

  /* Verify values survived round-trip */

  if (load_cfg.fall.torso_angle_thresh != 60.0f)
    {
      printf("  FAIL: loaded angle=%.1f (expected 60.0)\n",
             (double)load_cfg.fall.torso_angle_thresh);
      return -1;
    }

  if (load_cfg.fall.consecutive_frames != 5)
    {
      printf("  FAIL: loaded frames=%u (expected 5)\n",
             load_cfg.fall.consecutive_frames);
      return -1;
    }

  if (load_cfg.network.mqtt_port != 8883)
    {
      printf("  FAIL: loaded port=%u (expected 8883)\n",
             load_cfg.network.mqtt_port);
      return -1;
    }

  if (strcmp(load_cfg.network.mqtt_broker, "10.0.0.1") != 0)
    {
      printf("  FAIL: loaded broker=%s\n",
             load_cfg.network.mqtt_broker);
      return -1;
    }

  printf("  PASS: save/load cycle verified\n");
  return 0;
}

static int test_load_missing_file(void)
{
  struct edgesight_config_s cfg;
  int ret;

  config_set_defaults(&cfg);
  ret = config_load_file(&cfg, "/tmp/nonexistent_file.cfg");

  if (ret != -1)
    {
      printf("  FAIL: expected -1, got %d\n", ret);
      return -1;
    }

  /* Config should remain unchanged */

  if (cfg.fall.torso_angle_thresh != 45.0f)
    {
      printf("  FAIL: config was modified by missing file\n");
      return -1;
    }

  printf("  PASS: missing file handled gracefully\n");
  return 0;
}

static int test_partial_config(void)
{
  FILE *fp;
  char test_path[] = "/tmp/edgesight_partial.cfg";
  struct edgesight_config_s cfg;
  int ret;

  /* Write a partial config (only override one setting) */

  fp = fopen(test_path, "w");
  if (fp == NULL)
    {
      printf("  FAIL: cannot create test file\n");
      return -1;
    }

  fprintf(fp, "fall.angle_thresh=30.0\n");
  fclose(fp);

  /* Load with defaults, then override */

  config_set_defaults(&cfg);
  ret = config_load_file(&cfg, test_path);
  if (ret < 0)
    {
      printf("  FAIL: load_file returned %d\n", ret);
      return -1;
    }

  /* Overridden value should be updated */

  if (cfg.fall.torso_angle_thresh != 30.0f)
    {
      printf("  FAIL: angle=%.1f (expected 30.0)\n",
             (double)cfg.fall.torso_angle_thresh);
      return -1;
    }

  /* Other values should remain at defaults */

  if (cfg.fall.consecutive_frames != 3)
    {
      printf("  FAIL: frames=%u (expected 3)\n",
             cfg.fall.consecutive_frames);
      return -1;
    }

  printf("  PASS: partial config override works\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_config_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("==========================\n");
  printf(" EdgeSight Config Test\n");
  printf("==========================\n\n");

  printf("[1] Default values:\n");
  failures += (test_defaults() != 0);

  printf("\n[2] Save/load cycle:\n");
  failures += (test_save_load_cycle() != 0);

  printf("\n[3] Missing file handling:\n");
  failures += (test_load_missing_file() != 0);

  printf("\n[4] Partial config override:\n");
  failures += (test_partial_config() != 0);

  printf("\n==========================\n");
  if (failures == 0)
    {
      printf(" ALL TESTS PASSED\n");
    }
  else
    {
      printf(" %d TEST(S) FAILED\n", failures);
    }

  printf("==========================\n");
  return failures;
}
