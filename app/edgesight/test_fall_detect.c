/****************************************************************************
 * app/edgesight/test_fall_detect.c
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
 * EdgeSight - Unit test for fall detection algorithm.
 * Can run on host (QEMU) to verify logic without real hardware.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "fall_detect.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief Create a standing pose (shoulders above hips, upright)
 */

static void make_standing_pose(struct pose_result_s *pose)
{
  memset(pose, 0, sizeof(*pose));

  /* Shoulders at top, hips at middle — upright person */

  pose->kp[KP_LEFT_SHOULDER].x = 0.45f;
  pose->kp[KP_LEFT_SHOULDER].y = 0.25f;
  pose->kp[KP_LEFT_SHOULDER].confidence = 0.9f;

  pose->kp[KP_RIGHT_SHOULDER].x = 0.55f;
  pose->kp[KP_RIGHT_SHOULDER].y = 0.25f;
  pose->kp[KP_RIGHT_SHOULDER].confidence = 0.9f;

  pose->kp[KP_LEFT_HIP].x = 0.45f;
  pose->kp[KP_LEFT_HIP].y = 0.55f;
  pose->kp[KP_LEFT_HIP].confidence = 0.9f;

  pose->kp[KP_RIGHT_HIP].x = 0.55f;
  pose->kp[KP_RIGHT_HIP].y = 0.55f;
  pose->kp[KP_RIGHT_HIP].confidence = 0.9f;

  /* Bbox: tall and narrow (standing) */

  pose->bbox_x = 0.35f;
  pose->bbox_y = 0.1f;
  pose->bbox_w = 0.3f;
  pose->bbox_h = 0.8f;
  pose->det_conf = 0.95f;
}

/**
 * @brief Create a fallen pose (shoulders at same height as hips, horizontal)
 */

static void make_fallen_pose(struct pose_result_s *pose)
{
  memset(pose, 0, sizeof(*pose));

  /* Shoulders and hips at same y level — lying horizontal */

  pose->kp[KP_LEFT_SHOULDER].x = 0.2f;
  pose->kp[KP_LEFT_SHOULDER].y = 0.7f;
  pose->kp[KP_LEFT_SHOULDER].confidence = 0.85f;

  pose->kp[KP_RIGHT_SHOULDER].x = 0.3f;
  pose->kp[KP_RIGHT_SHOULDER].y = 0.72f;
  pose->kp[KP_RIGHT_SHOULDER].confidence = 0.85f;

  pose->kp[KP_LEFT_HIP].x = 0.6f;
  pose->kp[KP_LEFT_HIP].y = 0.7f;
  pose->kp[KP_LEFT_HIP].confidence = 0.85f;

  pose->kp[KP_RIGHT_HIP].x = 0.7f;
  pose->kp[KP_RIGHT_HIP].y = 0.72f;
  pose->kp[KP_RIGHT_HIP].confidence = 0.85f;

  /* Bbox: wide and short (lying down) */

  pose->bbox_x = 0.1f;
  pose->bbox_y = 0.5f;
  pose->bbox_w = 0.8f;
  pose->bbox_h = 0.4f;
  pose->det_conf = 0.9f;
}

/**
 * @brief Create a sitting pose (intermediate)
 */

static void make_sitting_pose(struct pose_result_s *pose)
{
  memset(pose, 0, sizeof(*pose));

  /* Shoulders slightly above hips — roughly 30 degrees from vertical */

  pose->kp[KP_LEFT_SHOULDER].x = 0.42f;
  pose->kp[KP_LEFT_SHOULDER].y = 0.35f;
  pose->kp[KP_LEFT_SHOULDER].confidence = 0.9f;

  pose->kp[KP_RIGHT_SHOULDER].x = 0.52f;
  pose->kp[KP_RIGHT_SHOULDER].y = 0.35f;
  pose->kp[KP_RIGHT_SHOULDER].confidence = 0.9f;

  pose->kp[KP_LEFT_HIP].x = 0.45f;
  pose->kp[KP_LEFT_HIP].y = 0.55f;
  pose->kp[KP_LEFT_HIP].confidence = 0.9f;

  pose->kp[KP_RIGHT_HIP].x = 0.55f;
  pose->kp[KP_RIGHT_HIP].y = 0.55f;
  pose->kp[KP_RIGHT_HIP].confidence = 0.9f;

  /* Bbox: roughly square (sitting) */

  pose->bbox_x = 0.3f;
  pose->bbox_y = 0.2f;
  pose->bbox_w = 0.4f;
  pose->bbox_h = 0.6f;
  pose->det_conf = 0.92f;
}

/**
 * @brief Test: standing person should not trigger fall
 */

static int test_standing_no_fall(void)
{
  struct fall_detector_s ctx;
  struct pose_result_s pose;
  struct fall_output_s output;
  int i;

  fall_detect_init(&ctx);
  make_standing_pose(&pose);

  /* Process multiple frames — should never trigger */

  for (i = 0; i < 10; i++)
    {
      bool fallen = fall_detect_process(&ctx, &pose, &output);
      if (fallen)
        {
          printf("  FAIL: standing pose triggered fall at frame %d\n", i);
          return -1;
        }
    }

  if (output.state != FALL_STATE_STANDING)
    {
      printf("  FAIL: expected STANDING state, got %d\n", output.state);
      return -1;
    }

  printf("  PASS: standing pose correctly not detected as fall\n");
  printf("        torso_angle=%.1f cog=%.2f bbox_ratio=%.2f\n",
         (double)output.torso_angle,
         (double)output.cog_ratio,
         (double)output.bbox_ratio);
  return 0;
}

/**
 * @brief Test: fallen person should trigger after consecutive frames
 */

static int test_fallen_triggers(void)
{
  struct fall_detector_s ctx;
  struct pose_result_s pose;
  struct fall_output_s output;
  bool fallen = false;
  int trigger_frame = -1;
  int i;

  fall_detect_init(&ctx);
  make_fallen_pose(&pose);

  for (i = 0; i < 10; i++)
    {
      fallen = fall_detect_process(&ctx, &pose, &output);
      if (fallen && trigger_frame < 0)
        {
          trigger_frame = i;
        }
    }

  if (!fallen)
    {
      printf("  FAIL: fallen pose not detected\n");
      printf("        torso_angle=%.1f cog=%.2f bbox_ratio=%.2f\n",
             (double)output.torso_angle,
             (double)output.cog_ratio,
             (double)output.bbox_ratio);
      return -1;
    }

  if (trigger_frame != (FALL_CONSECUTIVE_FRAMES - 1))
    {
      printf("  WARN: triggered at frame %d (expected %d)\n",
             trigger_frame, FALL_CONSECUTIVE_FRAMES - 1);
    }

  printf("  PASS: fallen pose detected at frame %d\n", trigger_frame);
  printf("        torso_angle=%.1f cog=%.2f bbox_ratio=%.2f conf=%.2f\n",
         (double)output.torso_angle,
         (double)output.cog_ratio,
         (double)output.bbox_ratio,
         (double)output.confidence);
  return 0;
}

/**
 * @brief Test: sitting should not trigger fall
 */

static int test_sitting_no_fall(void)
{
  struct fall_detector_s ctx;
  struct pose_result_s pose;
  struct fall_output_s output;
  int i;

  fall_detect_init(&ctx);
  make_sitting_pose(&pose);

  for (i = 0; i < 10; i++)
    {
      bool fallen = fall_detect_process(&ctx, &pose, &output);
      if (fallen)
        {
          printf("  FAIL: sitting pose triggered fall at frame %d\n", i);
          printf("        torso_angle=%.1f cog=%.2f bbox_ratio=%.2f\n",
                 (double)output.torso_angle,
                 (double)output.cog_ratio,
                 (double)output.bbox_ratio);
          return -1;
        }
    }

  printf("  PASS: sitting pose correctly not detected as fall\n");
  printf("        torso_angle=%.1f cog=%.2f bbox_ratio=%.2f\n",
         (double)output.torso_angle,
         (double)output.cog_ratio,
         (double)output.bbox_ratio);
  return 0;
}

/**
 * @brief Test: reset after fall should return to standing
 */

static int test_reset_after_fall(void)
{
  struct fall_detector_s ctx;
  struct pose_result_s pose;
  struct fall_output_s output;
  int i;

  fall_detect_init(&ctx);
  make_fallen_pose(&pose);

  /* Trigger fall */

  for (i = 0; i < FALL_CONSECUTIVE_FRAMES + 1; i++)
    {
      fall_detect_process(&ctx, &pose, &output);
    }

  if (output.state != FALL_STATE_FALLEN)
    {
      printf("  FAIL: expected FALLEN state before reset\n");
      return -1;
    }

  /* Reset */

  fall_detect_reset(&ctx);

  /* Feed standing pose — should be back to STANDING */

  make_standing_pose(&pose);
  fall_detect_process(&ctx, &pose, &output);

  if (output.state != FALL_STATE_STANDING)
    {
      printf("  FAIL: expected STANDING after reset, got %d\n",
             output.state);
      return -1;
    }

  printf("  PASS: reset correctly returns to STANDING state\n");
  return 0;
}

/**
 * @brief Test: transient suspicious frames should not trigger
 */

static int test_transient_no_trigger(void)
{
  struct fall_detector_s ctx;
  struct pose_result_s fallen_pose;
  struct pose_result_s standing_pose;
  struct fall_output_s output;

  fall_detect_init(&ctx);
  make_fallen_pose(&fallen_pose);
  make_standing_pose(&standing_pose);

  /* 1 suspicious frame, then back to standing */

  fall_detect_process(&ctx, &fallen_pose, &output);
  fall_detect_process(&ctx, &standing_pose, &output);
  fall_detect_process(&ctx, &standing_pose, &output);

  if (output.state == FALL_STATE_FALLEN)
    {
      printf("  FAIL: transient frame triggered fall\n");
      return -1;
    }

  printf("  PASS: transient suspicious frame correctly filtered\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("==============================\n");
  printf(" EdgeSight Fall Detection Test\n");
  printf("==============================\n\n");

  printf("[1] Standing pose (no fall):\n");
  failures += (test_standing_no_fall() != 0);

  printf("\n[2] Fallen pose (triggers):\n");
  failures += (test_fallen_triggers() != 0);

  printf("\n[3] Sitting pose (no fall):\n");
  failures += (test_sitting_no_fall() != 0);

  printf("\n[4] Reset after fall:\n");
  failures += (test_reset_after_fall() != 0);

  printf("\n[5] Transient frame filtering:\n");
  failures += (test_transient_no_trigger() != 0);

  printf("\n==============================\n");
  if (failures == 0)
    {
      printf(" ALL TESTS PASSED\n");
    }
  else
    {
      printf(" %d TEST(S) FAILED\n", failures);
    }

  printf("==============================\n");

  return failures;
}
