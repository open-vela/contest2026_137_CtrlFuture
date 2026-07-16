/****************************************************************************
 * app/edgesight/recorder_hal.c
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
 * EdgeSight - Recorder HAL implementation.
 * Wraps H.264 VENC hardware encoder + SD card for NuttX.
 *
 * TODO: Replace stub code with actual h264encapi/VENC calls when
 * hardware is available.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "recorder_hal.h"
#include <string.h>
#include <stdio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* When building with real hardware:
 * #include "h264encapi.h"
 * #include "stm32n6xx_ll_venc.h"
 * #include "ewl.h"
 */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int recorder_hal_init(struct recorder_context_s *ctx,
                      const struct recorder_config_s *cfg)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->config = *cfg;

  /* On real hardware:
   *   1. Initialize VENC clocks
   *   2. Configure H264 encoder (h264encapi):
   *      H264EncCfg cfg;
   *      cfg.width = VENC_WIDTH;
   *      cfg.height = VENC_HEIGHT;
   *      cfg.frameRateNum = fps;
   *      cfg.frameRateDenom = 1;
   *      H264EncInit(&cfg, &encoder);
   *   3. Set rate control:
   *      H264EncSetRateCtrl(encoder, &rcCfg);
   *   4. Set coding control (GOP, slice mode):
   *      H264EncSetCodingCtrl(encoder, &codingCfg);
   *
   * Reference: VENC_RTSP_Server/venc_app.c encoder_prepare()
   */

  ctx->output_fd = -1;
  ctx->state = REC_STATE_IDLE;
  ctx->initialized = true;

  printf("[recorder] Initialized: %ukbps %ufps GOP=%u (stub)\n",
         (unsigned)cfg->bitrate_kbps,
         (unsigned)cfg->fps,
         (unsigned)cfg->gop_size);
  return 0;
}

int recorder_hal_start(struct recorder_context_s *ctx,
                       uint32_t event_id)
{
  if (!ctx->initialized || ctx->state == REC_STATE_RECORDING)
    {
      return -1;
    }

  /* On real hardware:
   *   1. Create file: sprintf(path, "%s/event_%04u_%08lu.h264",
   *                           output_dir, event_id, timestamp);
   *   2. Open file: ctx->output_fd = open(path, O_WRONLY|O_CREAT);
   *   3. Encode SPS/PPS header:
   *      H264EncStrmStart(encoder, &encIn, &encOut);
   *      write(fd, encOut.pOutBuf, encOut.streamSize);
   *   4. Start DCMIPP capture for VENC input
   */

  ctx->state = REC_STATE_RECORDING;
  ctx->frame_count = 0;

  printf("[recorder] Recording started: event #%lu (stub)\n",
         (unsigned long)event_id);
  return 0;
}

int recorder_hal_feed_frame(struct recorder_context_s *ctx,
                            const void *frame, uint32_t size)
{
  if (!ctx->initialized || ctx->state != REC_STATE_RECORDING)
    {
      return -1;
    }

  (void)frame;
  (void)size;

  /* On real hardware:
   *   1. Set input buffer address:
   *      encIn.busLuma = (uintptr_t)frame;
   *      encIn.timeIncrement = 1;
   *   2. Determine frame type:
   *      encIn.codingType = (frame_count % gop == 0) ?
   *                          H264ENC_INTRA_FRAME :
   *                          H264ENC_PREDICTED_FRAME;
   *   3. Encode:
   *      H264EncStrmEncode(encoder, &encIn, &encOut, ...);
   *   4. Write to SD:
   *      write(fd, encOut.pOutBuf, encOut.streamSize);
   *
   * Reference: VENC_RTSP_Server/venc_app.c encode_frame()
   */

  ctx->frame_count++;

  /* Check duration limit */

  if (ctx->config.max_duration_s > 0 &&
      ctx->frame_count >= ctx->config.fps * ctx->config.max_duration_s)
    {
      printf("[recorder] Max duration reached, stopping\n");
      recorder_hal_stop(ctx);
    }

  return 0;
}

int recorder_hal_stop(struct recorder_context_s *ctx)
{
  if (!ctx->initialized || ctx->state != REC_STATE_RECORDING)
    {
      return -1;
    }

  /* On real hardware:
   *   1. Encode end-of-stream:
   *      H264EncStrmEnd(encoder, &encIn, &encOut);
   *      write(fd, encOut.pOutBuf, encOut.streamSize);
   *   2. Close file: close(ctx->output_fd);
   *   3. Stop DCMIPP VENC capture
   */

  ctx->state = REC_STATE_IDLE;

  printf("[recorder] Stopped: %lu frames encoded (stub)\n",
         (unsigned long)ctx->frame_count);
  return 0;
}

void recorder_hal_get_stats(const struct recorder_context_s *ctx,
                            struct recorder_stats_s *stats)
{
  memset(stats, 0, sizeof(*stats));
  stats->state = ctx->state;
  stats->frames_encoded = ctx->frame_count;
}

void recorder_hal_deinit(struct recorder_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  if (ctx->state == REC_STATE_RECORDING)
    {
      recorder_hal_stop(ctx);
    }

  /* On real hardware:
   *   H264EncRelease(encoder);
   */

  memset(ctx, 0, sizeof(*ctx));
  printf("[recorder] Deinitialized\n");
}
