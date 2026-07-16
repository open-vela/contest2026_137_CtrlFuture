/****************************************************************************
 * app/edgesight/test_display_hal.c
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
 * EdgeSight - Unit test for display HAL (stub mode).
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>

#include "display_hal.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_init(void)
{
  struct display_context_s ctx;
  struct display_config_s cfg;
  int ret;

  cfg.screen_width = 800;
  cfg.screen_height = 480;
  cfg.bg_format = 0;
  cfg.fg_format = 0;

  ret = display_hal_init(&ctx, &cfg);
  if (ret < 0)
    {
      printf("  FAIL: init returned %d\n", ret);
      return -1;
    }

  if (!ctx.initialized)
    {
      printf("  FAIL: not marked initialized\n");
      return -1;
    }

  printf("  PASS: init (%lux%lu)\n",
         (unsigned long)cfg.screen_width,
         (unsigned long)cfg.screen_height);
  return 0;
}

static int test_clear_fg(void)
{
  struct display_context_s ctx;
  struct display_config_s cfg;

  cfg.screen_width = 800;
  cfg.screen_height = 480;
  cfg.bg_format = 0;
  cfg.fg_format = 0;

  display_hal_init(&ctx, &cfg);

  /* Should not crash */

  display_hal_clear_fg(&ctx);

  printf("  PASS: clear_fg\n");
  return 0;
}

static int test_draw_bbox(void)
{
  struct display_context_s ctx;
  struct display_config_s cfg;
  struct display_bbox_s bbox;

  cfg.screen_width = 800;
  cfg.screen_height = 480;
  cfg.bg_format = 0;
  cfg.fg_format = 0;

  display_hal_init(&ctx, &cfg);

  bbox.x = 100;
  bbox.y = 100;
  bbox.w = 50;
  bbox.h = 80;
  bbox.color = 0x07e0;
  bbox.label = "person";
  bbox.confidence = 0.95f;

  display_hal_draw_bbox(&ctx, &bbox);

  printf("  PASS: draw_bbox\n");
  return 0;
}

static int test_draw_stats(void)
{
  struct display_context_s ctx;
  struct display_config_s cfg;
  struct display_stats_s stats;

  cfg.screen_width = 800;
  cfg.screen_height = 480;
  cfg.bg_format = 0;
  cfg.fg_format = 0;

  display_hal_init(&ctx, &cfg);

  memset(&stats, 0, sizeof(stats));
  stats.fps = 30;
  stats.inference_ms = 15;
  stats.persons_detected = 2;
  stats.fall_count = 0;
  stats.alert_active = false;

  display_hal_draw_stats(&ctx, &stats);

  printf("  PASS: draw_stats (fps=%lu persons=%lu)\n",
         (unsigned long)stats.fps,
         (unsigned long)stats.persons_detected);
  return 0;
}

static int test_show_alert(void)
{
  struct display_context_s ctx;
  struct display_config_s cfg;

  cfg.screen_width = 800;
  cfg.screen_height = 480;
  cfg.bg_format = 0;
  cfg.fg_format = 0;

  display_hal_init(&ctx, &cfg);

  display_hal_show_alert(&ctx, "FALL DETECTED");

  printf("  PASS: show_alert\n");
  return 0;
}

static int test_swap_buffer(void)
{
  struct display_context_s ctx;
  struct display_config_s cfg;

  cfg.screen_width = 800;
  cfg.screen_height = 480;
  cfg.bg_format = 0;
  cfg.fg_format = 0;

  display_hal_init(&ctx, &cfg);

  uint32_t old_idx = ctx.fg_write_idx;
  display_hal_swap(&ctx);

  if (ctx.fg_write_idx == old_idx)
    {
      printf("  FAIL: swap did not toggle index\n");
      return -1;
    }

  display_hal_swap(&ctx);
  if (ctx.fg_write_idx != old_idx)
    {
      printf("  FAIL: double swap did not restore\n");
      return -1;
    }

  printf("  PASS: swap_buffer toggles index\n");
  return 0;
}

static int test_deinit(void)
{
  struct display_context_s ctx;
  struct display_config_s cfg;

  cfg.screen_width = 800;
  cfg.screen_height = 480;
  cfg.bg_format = 0;
  cfg.fg_format = 0;

  display_hal_init(&ctx, &cfg);
  display_hal_deinit(&ctx);

  if (ctx.initialized)
    {
      printf("  FAIL: still initialized after deinit\n");
      return -1;
    }

  printf("  PASS: deinit clears context\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_display_hal_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("============================\n");
  printf(" EdgeSight Display HAL Test\n");
  printf("============================\n\n");

  printf("[1] Init:\n");
  failures += (test_init() != 0);

  printf("\n[2] Clear foreground:\n");
  failures += (test_clear_fg() != 0);

  printf("\n[3] Draw bounding box:\n");
  failures += (test_draw_bbox() != 0);

  printf("\n[4] Draw stats panel:\n");
  failures += (test_draw_stats() != 0);

  printf("\n[5] Show alert:\n");
  failures += (test_show_alert() != 0);

  printf("\n[6] Swap buffer:\n");
  failures += (test_swap_buffer() != 0);

  printf("\n[7] Deinit:\n");
  failures += (test_deinit() != 0);

  printf("\n============================\n");
  if (failures == 0)
    {
      printf(" ALL TESTS PASSED\n");
    }
  else
    {
      printf(" %d TEST(S) FAILED\n", failures);
    }

  printf("============================\n");
  return failures;
}
