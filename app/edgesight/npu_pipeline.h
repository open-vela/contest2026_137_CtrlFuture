/****************************************************************************
 * app/edgesight/npu_pipeline.h
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
 * EdgeSight - NPU inference pipeline manager.
 * Coordinates dual-model inference: YOLO detection + MoveNet pose.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_NPU_PIPELINE_H
#define __APP_EDGESIGHT_NPU_PIPELINE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "postprocess.h"
#include "fall_detect.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define NPU_MODEL_DETECT    0
#define NPU_MODEL_POSE      1
#define NPU_MODEL_MAX       2

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* NPU pipeline statistics */

struct npu_pipeline_stats_s
{
  uint32_t detect_infer_ms;
  uint32_t pose_infer_ms;
  uint32_t postprocess_ms;
  uint32_t total_frames;
  uint32_t persons_detected;
  uint32_t falls_detected;
};

/* NPU pipeline context */

struct npu_pipeline_s
{
  bool     initialized;
  bool     models_loaded[NPU_MODEL_MAX];
  uint32_t detect_width;
  uint32_t detect_height;
  uint32_t pose_width;
  uint32_t pose_height;
  struct fall_detector_s fall_ctx;
  struct npu_pipeline_stats_s stats;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize NPU pipeline
 * @param ctx Pipeline context
 * @return 0 on success
 */

int npu_pipeline_init(struct npu_pipeline_s *ctx);

/**
 * @brief Load AI models
 * @param ctx Pipeline context
 * @return 0 on success
 */

int npu_pipeline_load_models(struct npu_pipeline_s *ctx);

/**
 * @brief Run inference pipeline on a frame
 * @param ctx Pipeline context
 * @param frame Input frame buffer (RGB888)
 * @param frame_w Frame width
 * @param frame_h Frame height
 * @param detections Output: detected persons
 * @param poses Output: pose results (one per person)
 * @param fall_out Output: fall detection result
 * @param fall_detected Output: true if fall confirmed
 * @return 0 on success
 */

int npu_pipeline_run(struct npu_pipeline_s *ctx,
                     const void *frame,
                     uint32_t frame_w, uint32_t frame_h,
                     struct detection_output_s *detections,
                     struct pose_result_s *poses,
                     struct fall_output_s *fall_out,
                     bool *fall_detected);

/**
 * @brief Get pipeline statistics
 * @param ctx Pipeline context
 * @param stats Output statistics
 */

void npu_pipeline_get_stats(
    const struct npu_pipeline_s *ctx,
    struct npu_pipeline_stats_s *stats);

/**
 * @brief Reset fall detector
 * @param ctx Pipeline context
 */

void npu_pipeline_reset_fall(struct npu_pipeline_s *ctx);

#endif /* __APP_EDGESIGHT_NPU_PIPELINE_H */
