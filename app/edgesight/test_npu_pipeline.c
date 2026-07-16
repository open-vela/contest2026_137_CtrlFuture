/****************************************************************************
 * app/edgesight/test_npu_pipeline.c
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
 * EdgeSight - Unit test for NPU pipeline manager.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>

#include "npu_pipeline.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_init(void)
{
  struct npu_pipeline_s ctx;
  int ret;

  ret = npu_pipeline_init(&ctx);
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

  if (ctx.detect_width != 480 || ctx.detect_height != 480)
    {
      printf("  FAIL: detect size %lux%lu\n",
             (unsigned long)ctx.detect_width,
             (unsigned long)ctx.detect_height);
      return -1;
    }

  if (ctx.pose_width != 192 || ctx.pose_height != 192)
    {
      printf("  FAIL: pose size %lux%lu\n",
             (unsigned long)ctx.pose_width,
             (unsigned long)ctx.pose_height);
      return -1;
    }

  printf("  PASS: init (detect=%lux%lu pose=%lux%lu)\n",
         (unsigned long)ctx.detect_width,
         (unsigned long)ctx.detect_height,
         (unsigned long)ctx.pose_width,
         (unsigned long)ctx.pose_height);
  return 0;
}

static int test_load_models(void)
{
  struct npu_pipeline_s ctx;
  int ret;

  npu_pipeline_init(&ctx);
  ret = npu_pipeline_load_models(&ctx);

  if (ret < 0)
    {
      printf("  FAIL: load_models returned %d\n", ret);
      return -1;
    }

  if (!ctx.models_loaded[NPU_MODEL_DETECT])
    {
      printf("  FAIL: detect model not loaded\n");
      return -1;
    }

  if (!ctx.models_loaded[NPU_MODEL_POSE])
    {
      printf("  FAIL: pose model not loaded\n");
      return -1;
    }

  printf("  PASS: models loaded (detect + pose)\n");
  return 0;
}

static int test_run_without_init(void)
{
  struct npu_pipeline_s ctx;
  struct detection_output_s detections;
  struct pose_result_s poses[PP_MAX_DETECTIONS];
  struct fall_output_s fall_out;
  bool fall_detected;
  int ret;

  memset(&ctx, 0, sizeof(ctx));
  ret = npu_pipeline_run(&ctx, NULL, 0, 0,
                          &detections, poses, &fall_out,
                          &fall_detected);

  if (ret != -1)
    {
      printf("  FAIL: expected -1, got %d\n", ret);
      return -1;
    }

  printf("  PASS: run without init returns error\n");
  return 0;
}

static int test_run_demo_mode(void)
{
  struct npu_pipeline_s ctx;
  struct detection_output_s detections;
  struct pose_result_s poses[PP_MAX_DETECTIONS];
  struct fall_output_s fall_out;
  bool fall_detected;
  int ret;

  npu_pipeline_init(&ctx);
  npu_pipeline_load_models(&ctx);

  ret = npu_pipeline_run(&ctx, NULL, 0, 0,
                          &detections, poses, &fall_out,
                          &fall_detected);

  if (ret < 0)
    {
      printf("  FAIL: run returned %d\n", ret);
      return -1;
    }

  /* Demo mode: no actual NPU, should return 0 detections */

  if (detections.count != 0)
    {
      printf("  FAIL: expected 0 detections in demo mode, "
             "got %lu\n", (unsigned long)detections.count);
      return -1;
    }

  if (fall_detected)
    {
      printf("  FAIL: no fall should be detected in demo\n");
      return -1;
    }

  if (ctx.stats.total_frames != 1)
    {
      printf("  FAIL: frame count %lu (expected 1)\n",
             (unsigned long)ctx.stats.total_frames);
      return -1;
    }

  printf("  PASS: demo mode (0 detections, no fall)\n");
  return 0;
}

static int test_stats(void)
{
  struct npu_pipeline_s ctx;
  struct npu_pipeline_stats_s stats;

  npu_pipeline_init(&ctx);
  npu_pipeline_load_models(&ctx);

  /* Run a few frames */

  int i;

  for (i = 0; i < 5; i++)
    {
      npu_pipeline_run(&ctx, NULL, 0, 0, NULL, NULL,
                        NULL, NULL);
    }

  npu_pipeline_get_stats(&ctx, &stats);

  if (stats.total_frames != 5)
    {
      printf("  FAIL: total_frames %lu (expected 5)\n",
             (unsigned long)stats.total_frames);
      return -1;
    }

  if (stats.persons_detected != 0)
    {
      printf("  FAIL: persons_detected %lu (expected 0)\n",
             (unsigned long)stats.persons_detected);
      return -1;
    }

  printf("  PASS: stats (frames=%lu persons=%lu falls=%lu)\n",
         (unsigned long)stats.total_frames,
         (unsigned long)stats.persons_detected,
         (unsigned long)stats.falls_detected);
  return 0;
}

static int test_reset_fall(void)
{
  struct npu_pipeline_s ctx;

  npu_pipeline_init(&ctx);

  /* Reset should not crash */

  npu_pipeline_reset_fall(&ctx);

  printf("  PASS: fall reset\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_npu_pipeline_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("=============================\n");
  printf(" EdgeSight NPU Pipeline Test\n");
  printf("=============================\n\n");

  printf("[1] Init:\n");
  failures += (test_init() != 0);

  printf("\n[2] Load models:\n");
  failures += (test_load_models() != 0);

  printf("\n[3] Run without init:\n");
  failures += (test_run_without_init() != 0);

  printf("\n[4] Run demo mode:\n");
  failures += (test_run_demo_mode() != 0);

  printf("\n[5] Statistics:\n");
  failures += (test_stats() != 0);

  printf("\n[6] Reset fall:\n");
  failures += (test_reset_fall() != 0);

  printf("\n=============================\n");
  if (failures == 0)
    {
      printf(" ALL TESTS PASSED\n");
    }
  else
    {
      printf(" %d TEST(S) FAILED\n", failures);
    }

  printf("=============================\n");
  return failures;
}
