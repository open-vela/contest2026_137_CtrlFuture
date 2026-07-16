/****************************************************************************
 * app/edgesight/test_camera_hal.c
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
 * EdgeSight - Unit test for camera HAL (stub mode).
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>

#include "camera_hal.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_init(void)
{
  struct camera_context_s ctx;
  int ret;

  ret = camera_hal_init(&ctx, 30);
  if (ret < 0)
    {
      printf("  FAIL: init returned %d\n", ret);
      return -1;
    }

  if (!ctx.initialized)
    {
      printf("  FAIL: not marked initialized\n");
      return -1;
    }

  if (ctx.fps != 30)
    {
      printf("  FAIL: fps=%lu (expected 30)\n",
             (unsigned long)ctx.fps);
      return -1;
    }

  printf("  PASS: init (fps=%lu, sensor=%lux%lu)\n",
         (unsigned long)ctx.fps,
         (unsigned long)ctx.sensor_width,
         (unsigned long)ctx.sensor_height);
  return 0;
}

static int test_config_display(void)
{
  struct camera_context_s ctx;
  struct camera_pipe_config_s cfg;

  camera_hal_init(&ctx, 30);

  cfg.width = 800;
  cfg.height = 480;
  cfg.format = CAM_FMT_RGB565;
  cfg.bpp = 2;
  cfg.enable_isp = true;

  int ret = camera_hal_config_display(&ctx, &cfg);
  if (ret < 0)
    {
      printf("  FAIL: config_display returned %d\n", ret);
      return -1;
    }

  if (ctx.display_pipe.width != 800)
    {
      printf("  FAIL: display width=%lu (expected 800)\n",
             (unsigned long)ctx.display_pipe.width);
      return -1;
    }

  printf("  PASS: config display (%lux%lu %s)\n",
         (unsigned long)cfg.width,
         (unsigned long)cfg.height,
         cfg.format == CAM_FMT_RGB565 ? "RGB565" : "other");
  return 0;
}

static int test_config_nn(void)
{
  struct camera_context_s ctx;
  struct camera_pipe_config_s cfg;

  camera_hal_init(&ctx, 30);

  cfg.width = 480;
  cfg.height = 480;
  cfg.format = CAM_FMT_RGB888;
  cfg.bpp = 3;
  cfg.enable_isp = true;

  int ret = camera_hal_config_nn(&ctx, &cfg);
  if (ret < 0)
    {
      printf("  FAIL: config_nn returned %d\n", ret);
      return -1;
    }

  if (ctx.nn_pipe.width != 480)
    {
      printf("  FAIL: nn width=%lu (expected 480)\n",
             (unsigned long)ctx.nn_pipe.width);
      return -1;
    }

  printf("  PASS: config nn (%lux%lu %s)\n",
         (unsigned long)cfg.width,
         (unsigned long)cfg.height,
         cfg.format == CAM_FMT_RGB888 ? "RGB888" : "other");
  return 0;
}

static int test_start_stop(void)
{
  struct camera_context_s ctx;
  int ret;

  camera_hal_init(&ctx, 30);

  ret = camera_hal_start(&ctx, 0, NULL, CAM_MODE_CONTINUOUS);
  if (ret < 0)
    {
      printf("  FAIL: start returned %d\n", ret);
      return -1;
    }

  ret = camera_hal_stop(&ctx, 0);
  if (ret < 0)
    {
      printf("  FAIL: stop returned %d\n", ret);
      return -1;
    }

  printf("  PASS: start/stop cycle\n");
  return 0;
}

static int test_start_without_init(void)
{
  struct camera_context_s ctx;
  int ret;

  memset(&ctx, 0, sizeof(ctx));
  ret = camera_hal_start(&ctx, 0, NULL, CAM_MODE_CONTINUOUS);

  if (ret != -1)
    {
      printf("  FAIL: expected -1, got %d\n", ret);
      return -1;
    }

  printf("  PASS: start without init returns error\n");
  return 0;
}

static int test_callback(void)
{
  struct camera_context_s ctx;

  camera_hal_init(&ctx, 30);

  int ret = camera_hal_set_callback(&ctx, 0, NULL, NULL);
  if (ret < 0)
    {
      printf("  FAIL: set_callback returned %d\n", ret);
      return -1;
    }

  /* Invalid pipe */

  ret = camera_hal_set_callback(&ctx, 2, NULL, NULL);
  if (ret != -1)
    {
      printf("  FAIL: expected -1 for pipe 2, got %d\n", ret);
      return -1;
    }

  printf("  PASS: callback registration\n");
  return 0;
}

static int test_deinit(void)
{
  struct camera_context_s ctx;

  camera_hal_init(&ctx, 30);
  camera_hal_deinit(&ctx);

  if (ctx.initialized)
    {
      printf("  FAIL: still initialized after deinit\n");
      return -1;
    }

  printf("  PASS: deinit clears context\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_camera_hal_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("============================\n");
  printf(" EdgeSight Camera HAL Test\n");
  printf("============================\n\n");

  printf("[1] Init:\n");
  failures += (test_init() != 0);

  printf("\n[2] Config display pipe:\n");
  failures += (test_config_display() != 0);

  printf("\n[3] Config NN pipe:\n");
  failures += (test_config_nn() != 0);

  printf("\n[4] Start/stop:\n");
  failures += (test_start_stop() != 0);

  printf("\n[5] Start without init:\n");
  failures += (test_start_without_init() != 0);

  printf("\n[6] Callback registration:\n");
  failures += (test_callback() != 0);

  printf("\n[7] Deinit:\n");
  failures += (test_deinit() != 0);

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
