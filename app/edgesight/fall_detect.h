/****************************************************************************
 * app/edgesight/fall_detect.h
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
 * EdgeSight - Fall detection algorithm based on pose keypoints.
 *
 * Uses 17 COCO keypoints from MoveNet/YOLOv8-Pose output to determine
 * if a person has fallen by analyzing:
 *   1. Torso angle relative to vertical
 *   2. Center-of-gravity height ratio
 *   3. Bounding box aspect ratio
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_FALL_DETECT_H
#define __APP_EDGESIGHT_FALL_DETECT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* COCO 17-keypoint indices */

#define KP_NOSE             0
#define KP_LEFT_EYE        1
#define KP_RIGHT_EYE       2
#define KP_LEFT_EAR        3
#define KP_RIGHT_EAR       4
#define KP_LEFT_SHOULDER   5
#define KP_RIGHT_SHOULDER  6
#define KP_LEFT_ELBOW      7
#define KP_RIGHT_ELBOW     8
#define KP_LEFT_WRIST      9
#define KP_RIGHT_WRIST     10
#define KP_LEFT_HIP        11
#define KP_RIGHT_HIP       12
#define KP_LEFT_KNEE       13
#define KP_RIGHT_KNEE      14
#define KP_LEFT_ANKLE      15
#define KP_RIGHT_ANKLE     16

#define KP_COUNT           17

/* Detection thresholds (tunable) */

#define FALL_TORSO_ANGLE_THRESH   55.0f  /* degrees from vertical */
#define FALL_COG_HEIGHT_THRESH    0.55f  /* ratio: cog_y / bbox_height */
#define FALL_BBOX_RATIO_THRESH    1.2f   /* width / height > this */
#define FALL_CONFIDENCE_THRESH    0.3f   /* min keypoint confidence */
#define FALL_CONSECUTIVE_FRAMES   3      /* frames to confirm fall */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Single keypoint with normalized coordinates [0,1] and confidence */

struct keypoint_s
{
  float x;          /* normalized x [0..1] */
  float y;          /* normalized y [0..1], 0=top */
  float confidence; /* detection confidence [0..1] */
};

/* Pose detection result */

struct pose_result_s
{
  struct keypoint_s kp[KP_COUNT];
  float bbox_x;     /* bounding box top-left x [0..1] */
  float bbox_y;     /* bounding box top-left y [0..1] */
  float bbox_w;     /* bounding box width [0..1] */
  float bbox_h;     /* bounding box height [0..1] */
  float det_conf;   /* overall detection confidence */
};

/* Fall detection state (per tracked person) */

enum fall_state_e
{
  FALL_STATE_STANDING = 0,
  FALL_STATE_SUSPICIOUS,
  FALL_STATE_FALLEN,
};

/* Fall detector context */

struct fall_detector_s
{
  enum fall_state_e state;
  uint8_t consecutive_count;  /* consecutive frames in suspicious */
  float last_torso_angle;
  float last_cog_ratio;
  float last_bbox_ratio;
};

/* Fall detection output */

struct fall_output_s
{
  enum fall_state_e state;
  float torso_angle;     /* current torso angle (degrees) */
  float cog_ratio;       /* center of gravity height ratio */
  float bbox_ratio;      /* bbox width/height ratio */
  float confidence;      /* fall detection confidence [0..1] */
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize fall detector context
 * @param ctx Pointer to fall detector context
 */

void fall_detect_init(struct fall_detector_s *ctx);

/**
 * @brief Process one frame of pose data
 * @param ctx    Fall detector context (maintains state across frames)
 * @param pose   Pose estimation result for this frame
 * @param output Output fall detection result
 * @return true if fall detected (state == FALL_STATE_FALLEN)
 */

bool fall_detect_process(struct fall_detector_s *ctx,
                         const struct pose_result_s *pose,
                         struct fall_output_s *output);

/**
 * @brief Reset fall detector state (e.g., after alert acknowledged)
 * @param ctx Pointer to fall detector context
 */

void fall_detect_reset(struct fall_detector_s *ctx);

#endif /* __APP_EDGESIGHT_FALL_DETECT_H */
