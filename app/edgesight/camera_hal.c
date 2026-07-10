/****************************************************************************
 * app/edgesight/camera_hal.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * EdgeSight - Camera HAL implementation.
 * Wraps DCMIPP dual-pipeline + ISP middleware for NuttX.
 *
 * TODO: Replace stub code with actual CMW_CAMERA API calls when
 * hardware is available.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "camera_hal.h"
#include "memory_map.h"
#include <string.h>
#include <stdio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* When building with real hardware, include ST camera middleware:
 * #include "cmw_camera.h"
 * #include "stm32n6xx_hal_dcmipp.h"
 */

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

  /* On real hardware:
   *   CMW_CameraInit_t cam_conf;
   *   cam_conf.width = 0;   (sensor default)
   *   cam_conf.height = 0;
   *   cam_conf.fps = fps;
   *   cam_conf.mirror_flip = CAMERA_FLIP;
   *   ret = CMW_CAMERA_Init(&cam_conf, NULL);
   *   ctx->sensor_width = cam_conf.width;
   *   ctx->sensor_height = cam_conf.height;
   */

  ctx->fps = fps;
  ctx->sensor_width = 2592;   /* IMX335 default */
  ctx->sensor_height = 1944;
  ctx->initialized = true;

  printf("[camera] Initialized @ %lu fps (stub)\n",
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

  /* On real hardware:
   *   CMW_DCMIPP_Conf_t dcmipp_conf = {0};
   *   dcmipp_conf.output_width = cfg->width;
   *   dcmipp_conf.output_height = cfg->height;
   *   dcmipp_conf.output_format = DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1;
   *   dcmipp_conf.output_bpp = cfg->bpp;
   *   dcmipp_conf.mode = CMW_Aspect_ratio_crop;
   *   dcmipp_conf.enable_gamma_conversion = 0;
   *   CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE1, &dcmipp_conf, &pitch);
   */

  ctx->display_pipe = *cfg;

  printf("[camera] Display pipe configured: %lux%lu %s\n",
         (unsigned long)cfg->width,
         (unsigned long)cfg->height,
         cfg->format == CAM_FMT_RGB565 ? "RGB565" : "other");
  return 0;
}

int camera_hal_config_nn(struct camera_context_s *ctx,
                         const struct camera_pipe_config_s *cfg)
{
  if (!ctx->initialized)
    {
      return -1;
    }

  /* On real hardware:
   *   CMW_DCMIPP_Conf_t dcmipp_conf = {0};
   *   dcmipp_conf.output_width = cfg->width;
   *   dcmipp_conf.output_height = cfg->height;
   *   dcmipp_conf.output_format = DCMIPP_PIXEL_PACKER_FORMAT_RGB888_1;
   *   dcmipp_conf.output_bpp = cfg->bpp;
   *   dcmipp_conf.mode = CMW_Aspect_ratio_crop;
   *   CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE2, &dcmipp_conf, &pitch);
   */

  ctx->nn_pipe = *cfg;

  printf("[camera] NN pipe configured: %lux%lu %s\n",
         (unsigned long)cfg->width,
         (unsigned long)cfg->height,
         cfg->format == CAM_FMT_RGB888 ? "RGB888" : "other");
  return 0;
}

int camera_hal_start(struct camera_context_s *ctx, int pipe,
                     void *buffer, int mode)
{
  if (!ctx->initialized)
    {
      return -1;
    }

  (void)buffer;

  /* On real hardware:
   *   uint32_t cmw_mode = (mode == CAM_MODE_CONTINUOUS) ?
   *                         CMW_MODE_CONTINUOUS : CMW_MODE_SNAPSHOT;
   *   CMW_CAMERA_Start(pipe == 0 ? DCMIPP_PIPE1 : DCMIPP_PIPE2,
   *                    buffer, cmw_mode);
   */

  printf("[camera] Pipe %d started (%s)\n",
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

  /* On real hardware:
   *   CMW_CAMERA_Suspend(pipe == 0 ? DCMIPP_PIPE1 : DCMIPP_PIPE2);
   */

  printf("[camera] Pipe %d stopped\n", pipe);
  return 0;
}

void camera_hal_isp_update(struct camera_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  /* On real hardware:
   *   CMW_CAMERA_Run();  (updates AE/AWB/ISP parameters)
   */
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

  /* On real hardware:
   *   CMW_CAMERA_DeInit();
   */

  memset(ctx, 0, sizeof(*ctx));
  printf("[camera] Deinitialized\n");
}
