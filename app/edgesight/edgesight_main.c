/****************************************************************************
 * app/edgesight/edgesight_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * EdgeSight - AI Edge Sentinel main application entry point.
 *
 * This is the top-level orchestrator that creates and manages the
 * processing pipeline threads:
 *   - Camera capture + ISP
 *   - NPU inference (person detection + pose estimation)
 *   - Fall detection decision engine
 *   - Display rendering
 *   - Event recording (H.264 + SD card)
 *   - Network alerts (MQTT)
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>

#include "fall_detect.h"
#include "postprocess.h"
#include "camera_hal.h"
#include "npu_hal.h"
#include "display_hal.h"
#include "recorder_hal.h"
#include "network_hal.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define EDGESIGHT_VERSION  "0.1.0"

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* Application global state */

struct edgesight_app_s
{
  /* Fall detector */

  struct fall_detector_s fall_ctx;

  /* HAL contexts */

  struct camera_context_s camera;
  struct npu_context_s npu;
  struct display_context_s display;
  struct recorder_context_s recorder;
  struct network_context_s network;

  /* Pipeline state flags */

  volatile bool running;
  volatile bool camera_ready;
  volatile bool npu_busy;
  volatile bool recording;

  /* Statistics */

  uint32_t frame_count;
  uint32_t detect_count;
  uint32_t fall_count;
  uint32_t inference_ms;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct edgesight_app_s g_app;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief Print application banner
 */

static void edgesight_banner(void)
{
  printf("========================================\n");
  printf("  EdgeSight AI Edge Sentinel v%s\n", EDGESIGHT_VERSION);
  printf("  STM32N647 | NuttX | NPU 600Gops\n");
  printf("========================================\n");
}

/**
 * @brief Initialize all hardware subsystems
 */

static int edgesight_hw_init(void)
{
  printf("[edgesight] Initializing hardware...\n");

  /* TODO: Camera + DCMIPP init */

  /* TODO: NPU init (enable SRAM banks, cache, clock) */

  /* TODO: LTDC + GPU2D display init */

  /* TODO: XSPI Flash init (memory-mapped for model weights) */

  /* TODO: SDMMC + FatFS init */

  /* TODO: Ethernet + lwIP init */

  printf("[edgesight] Hardware init complete.\n");
  return 0;
}

/**
 * @brief Initialize AI models
 */

static int edgesight_ai_init(void)
{
  printf("[edgesight] Loading AI models...\n");

  /* TODO: stai_runtime_init() */

  /* TODO: Load person detection model (YOLO-X nano) */

  /* TODO: Load pose estimation model (MoveNet Lightning) */

  printf("[edgesight] AI models loaded.\n");
  return 0;
}

/**
 * @brief Main processing loop (single-threaded for now)
 *
 * Production version will split into multiple pthreads:
 *   camera_task  -> captures frames
 *   npu_task     -> runs inference
 *   display_task -> renders UI
 *   record_task  -> H.264 encode + SD write
 *   network_task -> MQTT alerts
 */

static void edgesight_loop(struct edgesight_app_s *app)
{
  struct detection_output_s detections;
  struct pose_result_s pose;
  struct fall_output_s fall_out;
  struct npu_inference_result_s npu_result;
  struct display_stats_s stats;
  uint32_t i;

  while (app->running)
    {
      /* Step 1: Capture frame from camera (NN pipe, snapshot mode)
       * camera_hal_start(&app->camera, 1, nn_buffer, CAM_MODE_SNAPSHOT);
       * Wait for frame callback...
       */

      /* Step 2: Run person detection (YOLO-X nano) */

      /* npu_hal_run(&app->npu, NPU_MODEL_DETECT, nn_buffer, &npu_result);
       * postprocess_yolo(npu_result.output_buffers,
       *                  npu_result.output_sizes,
       *                  npu_result.output_count,
       *                  app->npu.info[NPU_MODEL_DETECT].input_width,
       *                  app->npu.info[NPU_MODEL_DETECT].input_height,
       *                  &detections);
       * app->inference_ms = npu_result.inference_time_ms;
       */

      /* Placeholder: no detections until HW is ready */

      detections.count = 0;

      /* Step 3: For each detected person, run pose estimation */

      for (i = 0; i < detections.count; i++)
        {
          /* TODO: Crop person ROI from camera frame */

          /* npu_hal_run(&app->npu, NPU_MODEL_POSE, crop_buf, &npu_result);
           * postprocess_movenet(npu_result.output_buffers[0],
           *                     npu_result.output_sizes[0],
           *                     &detections.detections[i],
           *                     &pose);
           */

          memset(&pose, 0, sizeof(pose));

          /* Step 4: Fall detection */

          if (fall_detect_process(&app->fall_ctx, &pose, &fall_out))
            {
              app->fall_count++;
              printf("[edgesight] FALL DETECTED! count=%lu conf=%.2f "
                     "angle=%.1f\n",
                     (unsigned long)app->fall_count,
                     (double)fall_out.confidence,
                     (double)fall_out.torso_angle);

              /* Trigger event recording */

              if (!app->recording)
                {
                  /* recorder_hal_start(&app->recorder, app->fall_count); */
                  app->recording = true;
                }

              /* Send MQTT alert */

              /* struct alert_message_s alert = {
               *   .level = ALERT_LEVEL_CRITICAL,
               *   .confidence = fall_out.confidence,
               *   .torso_angle = fall_out.torso_angle,
               *   .frame_number = app->frame_count,
               *   .description = "Fall detected"
               * };
               * network_hal_send_alert(&app->network, &alert);
               */

              /* Display alert */

              /* display_hal_show_alert(&app->display, "FALL DETECTED"); */
            }
        }

      /* Step 5: Update display */

      /* display_hal_clear_fg(&app->display);
       * for (i = 0; i < detections.count; i++) {
       *   struct display_bbox_s bbox = {
       *     .x = detections.detections[i].x_center * screen_w,
       *     ...
       *   };
       *   display_hal_draw_bbox(&app->display, &bbox);
       * }
       */

      memset(&stats, 0, sizeof(stats));
      stats.fps = (app->frame_count > 0) ? 30 : 0;
      stats.inference_ms = app->inference_ms;
      stats.persons_detected = detections.count;
      stats.fall_count = app->fall_count;
      stats.alert_active = (app->fall_ctx.state == FALL_STATE_FALLEN);

      /* display_hal_draw_stats(&app->display, &stats); */
      /* display_hal_swap(&app->display); */

      /* Step 6: Feed frame to recorder if active */

      /* if (app->recording) {
       *   recorder_hal_feed_frame(&app->recorder, yuv_frame, frame_size);
       * }
       */

      /* Step 7: Network keepalive */

      /* network_hal_poll(&app->network); */

      app->frame_count++;
      app->detect_count += detections.count;

      /* Yield to other tasks */

      usleep(33000); /* ~30 fps target */
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * @brief EdgeSight application entry point
 */

int main(int argc, char *argv[])
{
  int ret;

  edgesight_banner();

  /* Initialize application state */

  memset(&g_app, 0, sizeof(g_app));
  g_app.running = true;
  fall_detect_init(&g_app.fall_ctx);

  /* Hardware initialization */

  ret = edgesight_hw_init();
  if (ret < 0)
    {
      printf("[edgesight] ERROR: HW init failed: %d\n", ret);
      return EXIT_FAILURE;
    }

  /* AI model initialization */

  ret = edgesight_ai_init();
  if (ret < 0)
    {
      printf("[edgesight] ERROR: AI init failed: %d\n", ret);
      return EXIT_FAILURE;
    }

  printf("[edgesight] Starting main loop...\n");

  /* Main processing loop */

  edgesight_loop(&g_app);

  printf("[edgesight] Shutting down. Frames: %lu Falls: %lu\n",
         (unsigned long)g_app.frame_count,
         (unsigned long)g_app.fall_count);

  return EXIT_SUCCESS;
}
