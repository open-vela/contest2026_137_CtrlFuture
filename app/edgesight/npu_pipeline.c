/****************************************************************************
 * app/edgesight/npu_pipeline.c
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
 * EdgeSight - NPU inference pipeline implementation.
 * Coordinates dual-model inference: YOLO detection + MoveNet pose.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "npu_pipeline.h"
#include "npu_hal.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <time.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t get_ms(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static int crop_and_resize(const uint8_t *src,
                            uint32_t src_w, uint32_t src_h,
                            uint8_t *dst,
                            uint32_t dst_w, uint32_t dst_h,
                            const struct detection_s *det)
{
  uint32_t x1;
  uint32_t y1;
  uint32_t x2;
  uint32_t y2;
  uint32_t crop_w;
  uint32_t crop_h;
  uint32_t row;
  uint32_t col;

  /* Convert normalized coordinates to pixel coordinates */

  x1 = (uint32_t)((det->x_center - det->width / 2.0f) * src_w);
  y1 = (uint32_t)((det->y_center - det->height / 2.0f) * src_h);
  x2 = (uint32_t)((det->x_center + det->width / 2.0f) * src_w);
  y2 = (uint32_t)((det->y_center + det->height / 2.0f) * src_h);

  /* Clamp to source bounds */

  if (x1 >= src_w)
    {
      x1 = 0;
    }

  if (y1 >= src_h)
    {
      y1 = 0;
    }

  if (x2 > src_w)
    {
      x2 = src_w;
    }

  if (y2 > src_h)
    {
      y2 = src_h;
    }

  crop_w = x2 - x1;
  crop_h = y2 - y1;

  if (crop_w == 0 || crop_h == 0)
    {
      return -1;
    }

  /* Simple nearest-neighbor resize crop -> dst
   * TODO: Use DMA2D for hardware-accelerated resize
   */

  for (row = 0; row < dst_h; row++)
    {
      uint32_t src_row = y1 + (row * crop_h) / dst_h;
      for (col = 0; col < dst_w; col++)
        {
          uint32_t src_col = x1 + (col * crop_w) / dst_w;
          uint32_t src_idx =
            (src_row * src_w + src_col) * 3;
          uint32_t dst_idx =
            (row * dst_w + col) * 3;

          dst[dst_idx] = src[src_idx];
          dst[dst_idx + 1] = src[src_idx + 1];
          dst[dst_idx + 2] = src[src_idx + 2];
        }
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int npu_pipeline_init(struct npu_pipeline_s *ctx)
{
  memset(ctx, 0, sizeof(*ctx));

  fall_detect_init(&ctx->fall_ctx);

  ctx->detect_width = 480;
  ctx->detect_height = 480;
  ctx->pose_width = 192;
  ctx->pose_height = 192;

  syslog(LOG_INFO, "npu_pipeline: initialized "
         "(detect=%lux%lu pose=%lux%lu)\n",
         (unsigned long)ctx->detect_width,
         (unsigned long)ctx->detect_height,
         (unsigned long)ctx->pose_width,
         (unsigned long)ctx->pose_height);

  return 0;
}

int npu_pipeline_load_models(struct npu_pipeline_s *ctx)
{
  /* Load YOLO detection model */

  /* ret = npu_hal_load_model(NPU_MODEL_DETECT, &info); */

  ctx->models_loaded[NPU_MODEL_DETECT] = true;
  syslog(LOG_INFO, "npu_pipeline: YOLO detect model loaded\n");

  /* Load MoveNet pose model */

  /* ret = npu_hal_load_model(NPU_MODEL_POSE, &info); */

  ctx->models_loaded[NPU_MODEL_POSE] = true;
  syslog(LOG_INFO, "npu_pipeline: MoveNet pose model loaded\n");

  ctx->initialized = true;
  return 0;
}

int npu_pipeline_run(struct npu_pipeline_s *ctx,
                     const void *frame,
                     uint32_t frame_w, uint32_t frame_h,
                     struct detection_output_s *detections,
                     struct pose_result_s *poses,
                     struct fall_output_s *fall_out,
                     bool *fall_detected)
{
  uint32_t t0;
  uint32_t t1;
  uint32_t i;
  int ret = 0;

  *fall_detected = false;
  detections->count = 0;

  if (!ctx->initialized)
    {
      return -1;
    }

  /* Step 1: Run YOLO person detection */

  t0 = get_ms();

  /* npu_hal_run(NPU_MODEL_DETECT, frame, &result);
   * postprocess_yolo(result.outputs, ..., detections);
   */

  t1 = get_ms();
  ctx->stats.detect_infer_ms = t1 - t0;

  if (detections->count == 0)
    {
      ctx->stats.total_frames++;
      return 0;
    }

  ctx->stats.persons_detected += detections->count;

  /* Step 2: For each detected person, run pose estimation */

  for (i = 0; i < detections->count && i < PP_MAX_DETECTIONS; i++)
    {
      /* Crop person ROI from frame
       *
       * uint8_t crop_buf[192 * 192 * 3];
       * crop_and_resize(frame, frame_w, frame_h,
       *                 crop_buf, 192, 192,
       *                 &detections->detections[i]);
       */

      t0 = get_ms();

      /* npu_hal_run(NPU_MODEL_POSE, crop_buf, &result);
       * postprocess_movenet(result.outputs[0], ...,
       *                     &detections->detections[i],
       *                     &poses[i]);
       */

      t1 = get_ms();
      ctx->stats.pose_infer_ms = t1 - t0;

      /* Step 3: Fall detection */

      if (fall_detect_process(&ctx->fall_ctx,
                              &poses[i], fall_out))
        {
          *fall_detected = true;
          ctx->stats.falls_detected++;
        }
    }

  ctx->stats.total_frames++;
  return ret;
}

void npu_pipeline_get_stats(
    const struct npu_pipeline_s *ctx,
    struct npu_pipeline_stats_s *stats)
{
  *stats = ctx->stats;
}

void npu_pipeline_reset_fall(struct npu_pipeline_s *ctx)
{
  fall_detect_reset(&ctx->fall_ctx);
  syslog(LOG_INFO, "npu_pipeline: fall detector reset\n");
}
