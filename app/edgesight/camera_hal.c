/****************************************************************************
 * app/edgesight/camera_hal.c
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
 * EdgeSight - Camera HAL implementation.
 * Wraps DCMIPP dual-pipeline + ISP middleware for NuttX.
 *
 * Integrates with arch/arm/stm32n6/stm32n6_dcmipp.c driver.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "camera_hal.h"
#include "memory_map.h"
#include <string.h>
#include <stdio.h>
#include <syslog.h>

#ifdef CONFIG_ARCH_CHIP_STM32N6
#  include "stm32n6_dcmipp.h"
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static camera_frame_cb_t g_callbacks[2];
static void *g_cb_args[2];

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int camera_hal_init(struct camera_context_s *ctx, uint32_t fps)
{
  memset(ctx, 0, sizeof(*ctx));

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Initialize DCMIPP via arch-level driver
   * Display pipe: 800x480 RGB565 continuous
   * NN pipe: 480x480 RGB888 snapshot
   */

  int ret = stm32n6_dcmipp_init(800, 480, fps);
  if (ret < 0)
    {
      syslog(LOG_ERR, "camera: DCMIPP init failed: %d\n", ret);
      return ret;
    }

  ctx->sensor_width = 2592;   /* IMX335 default */
  ctx->sensor_height = 1944;
#else
  ctx->sensor_width = 2592;
  ctx->sensor_height = 1944;
#endif

  ctx->fps = fps;
  ctx->initialized = true;

  syslog(LOG_INFO, "camera: initialized @ %lu fps\n",
         (unsigned long)fps);
  return 0;
}

int camera_hal_config_display(struct camera_context_s *ctx,
                              const struct camera_pipe_config_s *cfg)
{
  if (!ctx->initialized)
    {
      return -1;
    }

  ctx->display_pipe = *cfg;

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Display pipe config is handled by stm32n6_dcmipp_init
   * (800x480 RGB565 continuous)
   */

  syslog(LOG_INFO, "camera: display pipe %lux%lu %s\n",
         (unsigned long)cfg->width,
         (unsigned long)cfg->height,
         cfg->format == CAM_FMT_RGB565 ? "RGB565" : "other");
#endif

  return 0;
}

int camera_hal_config_nn(struct camera_context_s *ctx,
                         const struct camera_pipe_config_s *cfg)
{
  if (!ctx->initialized)
    {
      return -1;
    }

  ctx->nn_pipe = *cfg;

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* NN pipe config is handled by stm32n6_dcmipp_init
   * (480x480 RGB888 snapshot)
   */

  syslog(LOG_INFO, "camera: NN pipe %lux%lu %s\n",
         (unsigned long)cfg->width,
         (unsigned long)cfg->height,
         cfg->format == CAM_FMT_RGB888 ? "RGB888" : "other");
#endif

  return 0;
}

int camera_hal_start(struct camera_context_s *ctx, int pipe,
                     void *buffer, int mode)
{
  if (!ctx->initialized)
    {
      return -1;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  uint32_t dcmipp_mode = (mode == CAM_MODE_CONTINUOUS) ?
                          DCMIPP_MODE_CONTINUOUS :
                          DCMIPP_MODE_SNAPSHOT;
  int ret = stm32n6_dcmipp_start(pipe, buffer, dcmipp_mode);
  if (ret < 0)
    {
      syslog(LOG_ERR, "camera: pipe %d start failed: %d\n",
             pipe, ret);
      return ret;
    }
#endif

  syslog(LOG_INFO, "camera: pipe %d started (%s)\n",
         pipe,
         mode == CAM_MODE_CONTINUOUS ? "continuous" : "snapshot");
  return 0;
}

int camera_hal_stop(struct camera_context_s *ctx, int pipe)
{
  if (!ctx->initialized)
    {
      return -1;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  int ret = stm32n6_dcmipp_stop(pipe);
  if (ret < 0)
    {
      syslog(LOG_ERR, "camera: pipe %d stop failed: %d\n",
             pipe, ret);
      return ret;
    }
#endif

  syslog(LOG_INFO, "camera: pipe %d stopped\n", pipe);
  return 0;
}

void camera_hal_isp_update(struct camera_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Run ISP auto-exposure / auto-white-balance */

  stm32n6_dcmipp_isp_update();
#endif
}

int camera_hal_set_callback(struct camera_context_s *ctx, int pipe,
                            camera_frame_cb_t cb, void *arg)
{
  if (!ctx->initialized || pipe > 1)
    {
      return -1;
    }

  g_callbacks[pipe] = cb;
  g_cb_args[pipe] = arg;
  return 0;
}

void camera_hal_deinit(struct camera_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Stop both pipes */

  stm32n6_dcmipp_stop(0);
  stm32n6_dcmipp_stop(1);
#endif

  memset(ctx, 0, sizeof(*ctx));
  syslog(LOG_INFO, "camera: deinitialized\n");
}
