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

  /* Pipeline state flags */

  volatile bool running;
  volatile bool camera_ready;
  volatile bool npu_busy;

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
  struct pose_result_s pose;
  struct fall_output_s fall_out;

  while (app->running)
    {
      /* Step 1: Capture frame from camera
       * TODO: CMW_CAMERA_Start(DCMIPP_PIPE2, nn_input, SNAPSHOT)
       */

      /* Step 2: Run person detection (YOLO)
       * TODO: stai_network_run(yolo_ctx, STAI_MODE_SYNC)
       */

      /* Step 3: For each detected person, run pose estimation
       * TODO: crop person ROI -> stai_network_run(pose_ctx, SYNC)
       */

      /* Step 4: Fall detection on pose output */

      /* Simulated pose data for skeleton testing */

      memset(&pose, 0, sizeof(pose));

      if (fall_detect_process(&app->fall_ctx, &pose, &fall_out))
        {
          app->fall_count++;
          printf("[edgesight] FALL DETECTED! count=%lu conf=%.2f "
                 "angle=%.1f\n",
                 (unsigned long)app->fall_count,
                 (double)fall_out.confidence,
                 (double)fall_out.torso_angle);

          /* TODO: Trigger recording */

          /* TODO: Send MQTT alert */

          /* TODO: Display red alert on LCD */
        }

      /* Step 5: Update display
       * TODO: GPU2D render detection boxes + stats
       */

      app->frame_count++;

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
