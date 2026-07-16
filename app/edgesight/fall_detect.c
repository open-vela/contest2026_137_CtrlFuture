/****************************************************************************
 * app/edgesight/fall_detect.c
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
 * EdgeSight - Fall detection algorithm implementation.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "fall_detect.h"
#include <math.h>
#include <string.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief Check if a keypoint has sufficient confidence
 */

static inline bool kp_valid(const struct keypoint_s *kp)
{
  return kp->confidence >= FALL_CONFIDENCE_THRESH;
}

/**
 * @brief Compute midpoint of two keypoints
 */

static inline void kp_midpoint(const struct keypoint_s *a,
                               const struct keypoint_s *b,
                               float *mx, float *my)
{
  *mx = (a->x + b->x) * 0.5f;
  *my = (a->y + b->y) * 0.5f;
}

/**
 * @brief Calculate torso angle from vertical (degrees)
 *
 * Torso is defined as the vector from hip-center to shoulder-center.
 * Vertical is the y-axis (pointing down in image coords).
 * Returns angle in degrees [0..180]. 0 = upright, 90 = horizontal.
 */

static float calc_torso_angle(const struct pose_result_s *pose)
{
  const struct keypoint_s *ls = &pose->kp[KP_LEFT_SHOULDER];
  const struct keypoint_s *rs = &pose->kp[KP_RIGHT_SHOULDER];
  const struct keypoint_s *lh = &pose->kp[KP_LEFT_HIP];
  const struct keypoint_s *rh = &pose->kp[KP_RIGHT_HIP];

  /* Need at least one shoulder and one hip */

  if ((!kp_valid(ls) && !kp_valid(rs)) ||
      (!kp_valid(lh) && !kp_valid(rh)))
    {
      return -1.0f; /* insufficient data */
    }

  float sx;
  float sy;
  float hx;
  float hy;

  /* Shoulder center */

  if (kp_valid(ls) && kp_valid(rs))
    {
      kp_midpoint(ls, rs, &sx, &sy);
    }
  else if (kp_valid(ls))
    {
      sx = ls->x;
      sy = ls->y;
    }
  else
    {
      sx = rs->x;
      sy = rs->y;
    }

  /* Hip center */

  if (kp_valid(lh) && kp_valid(rh))
    {
      kp_midpoint(lh, rh, &hx, &hy);
    }
  else if (kp_valid(lh))
    {
      hx = lh->x;
      hy = lh->y;
    }
  else
    {
      hx = rh->x;
      hy = rh->y;
    }

  /* Vector from hip to shoulder */

  float dx = sx - hx;
  float dy = sy - hy;  /* negative when upright (shoulder above hip) */

  /* Angle from vertical (y-axis pointing down):
   * upright: dx~0, dy<0 -> angle~0
   * lying:   dx!=0, dy~0 -> angle~90
   */

  float angle_rad = atan2f(fabsf(dx), fabsf(dy));
  return angle_rad * (180.0f / 3.14159265f);
}

/**
 * @brief Calculate center-of-gravity height ratio
 *
 * Estimates COG as the average y of hip and shoulder keypoints.
 * Returns ratio of (cog_y - bbox_top) / bbox_height.
 * High ratio (>0.5) suggests person's center is low = possible fall.
 */

static float calc_cog_height_ratio(const struct pose_result_s *pose)
{
  const struct keypoint_s *ls = &pose->kp[KP_LEFT_SHOULDER];
  const struct keypoint_s *rs = &pose->kp[KP_RIGHT_SHOULDER];
  const struct keypoint_s *lh = &pose->kp[KP_LEFT_HIP];
  const struct keypoint_s *rh = &pose->kp[KP_RIGHT_HIP];

  float sum_y = 0.0f;
  int count = 0;

  if (kp_valid(ls))
    {
      sum_y += ls->y;
      count++;
    }

  if (kp_valid(rs))
    {
      sum_y += rs->y;
      count++;
    }

  if (kp_valid(lh))
    {
      sum_y += lh->y;
      count++;
    }

  if (kp_valid(rh))
    {
      sum_y += rh->y;
      count++;
    }

  if (count == 0 || pose->bbox_h < 0.01f)
    {
      return -1.0f;
    }

  float cog_y = sum_y / (float)count;
  return (cog_y - pose->bbox_y) / pose->bbox_h;
}

/**
 * @brief Calculate bounding box aspect ratio (width / height)
 *
 * A standing person has ratio < 1 (tall and narrow).
 * A fallen person has ratio > 1 (wide and short).
 */

static float calc_bbox_ratio(const struct pose_result_s *pose)
{
  if (pose->bbox_h < 0.01f)
    {
      return 0.0f;
    }

  return pose->bbox_w / pose->bbox_h;
}

/**
 * @brief Compute overall fall confidence from individual metrics
 */

static float calc_fall_confidence(float torso_angle, float cog_ratio,
                                  float bbox_ratio)
{
  float conf = 0.0f;
  int factors = 0;

  /* Torso angle contribution */

  if (torso_angle >= 0.0f)
    {
      float t = torso_angle / 90.0f; /* normalize to [0..1] at 90 deg */
      if (t > 1.0f)
        {
          t = 1.0f;
        }

      conf += t;
      factors++;
    }

  /* COG height ratio contribution */

  if (cog_ratio >= 0.0f)
    {
      float c = cog_ratio; /* already [0..1] roughly */
      if (c > 1.0f)
        {
          c = 1.0f;
        }

      conf += c;
      factors++;
    }

  /* Bbox ratio contribution */

  if (bbox_ratio > 0.0f)
    {
      float b = bbox_ratio / 2.0f; /* normalize: ratio=2 -> conf=1 */
      if (b > 1.0f)
        {
          b = 1.0f;
        }

      conf += b;
      factors++;
    }

  if (factors == 0)
    {
      return 0.0f;
    }

  return conf / (float)factors;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void fall_detect_init(struct fall_detector_s *ctx)
{
  memset(ctx, 0, sizeof(struct fall_detector_s));
  ctx->state = FALL_STATE_STANDING;
}

bool fall_detect_process(struct fall_detector_s *ctx,
                         const struct pose_result_s *pose,
                         struct fall_output_s *output)
{
  float torso_angle;
  float cog_ratio;
  float bbox_ratio;
  bool is_suspicious;

  /* Calculate metrics */

  torso_angle = calc_torso_angle(pose);
  cog_ratio = calc_cog_height_ratio(pose);
  bbox_ratio = calc_bbox_ratio(pose);

  /* Store in context */

  ctx->last_torso_angle = torso_angle;
  ctx->last_cog_ratio = cog_ratio;
  ctx->last_bbox_ratio = bbox_ratio;

  /* Determine if current frame is suspicious.
   * A fall is suspected when at least 2 of 3 indicators trigger.
   */

  int triggers = 0;

  if (torso_angle >= FALL_TORSO_ANGLE_THRESH)
    {
      triggers++;
    }

  if (cog_ratio >= FALL_COG_HEIGHT_THRESH)
    {
      triggers++;
    }

  if (bbox_ratio >= FALL_BBOX_RATIO_THRESH)
    {
      triggers++;
    }

  is_suspicious = (triggers >= 2);

  /* State machine */

  switch (ctx->state)
    {
      case FALL_STATE_STANDING:
        if (is_suspicious)
          {
            ctx->state = FALL_STATE_SUSPICIOUS;
            ctx->consecutive_count = 1;
          }
        break;

      case FALL_STATE_SUSPICIOUS:
        if (is_suspicious)
          {
            ctx->consecutive_count++;
            if (ctx->consecutive_count >= FALL_CONSECUTIVE_FRAMES)
              {
                ctx->state = FALL_STATE_FALLEN;
              }
          }
        else
          {
            /* False alarm, go back to standing */

            ctx->state = FALL_STATE_STANDING;
            ctx->consecutive_count = 0;
          }
        break;

      case FALL_STATE_FALLEN:

        /* Stay in fallen state until manually reset */

        break;
    }

  /* Fill output */

  output->state = ctx->state;
  output->torso_angle = torso_angle;
  output->cog_ratio = cog_ratio;
  output->bbox_ratio = bbox_ratio;
  output->confidence = calc_fall_confidence(torso_angle, cog_ratio,
                                            bbox_ratio);

  return (ctx->state == FALL_STATE_FALLEN);
}

void fall_detect_reset(struct fall_detector_s *ctx)
{
  ctx->state = FALL_STATE_STANDING;
  ctx->consecutive_count = 0;
}
