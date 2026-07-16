/****************************************************************************
 * app/edgesight/display_hal.c
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
 * EdgeSight - Display HAL implementation.
 * Wraps LTDC dual-layer + GPU2D/DMA2D rendering for NuttX.
 *
 * Integrates with arch/arm/stm32n6/stm32n6_ltdc.c driver.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "display_hal.h"
#include "memory_map.h"
#include <string.h>
#include <stdio.h>
#include <syslog.h>

#ifdef CONFIG_ARCH_CHIP_STM32N6
#  include "stm32n6_ltdc.h"
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int display_hal_init(struct display_context_s *ctx,
                     const struct display_config_s *cfg)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->config = *cfg;

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* LTDC is already initialized in board_bringup.c via
   * stm32n6_ltdc_init(800, 480, bg_addr, fg_addr0, fg_addr1).
   * Store buffer addresses from memory map.
   */

  ctx->bg_buffer = (void *)BOARD_LCD_BG_ADDR;
#endif

  ctx->fg_write_idx = 0;
  ctx->initialized = true;

  syslog(LOG_INFO, "display: initialized %lux%lu\n",
         (unsigned long)cfg->screen_width,
         (unsigned long)cfg->screen_height);
  return 0;
}

void display_hal_set_bg_buffer(struct display_context_s *ctx,
                               void *buffer)
{
  if (!ctx->initialized)
    {
      return;
    }

  ctx->bg_buffer = buffer;

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Update LTDC Layer 0 framebuffer address */

  stm32n6_ltdc_set_bg_buffer(buffer);
#endif
}

void display_hal_clear_fg(struct display_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Clear foreground to transparent (0x0000) */

  stm32n6_ltdc_clear_fg();
#endif
}

void display_hal_draw_bbox(struct display_context_s *ctx,
                           const struct display_bbox_s *bbox)
{
  if (!ctx->initialized)
    {
      return;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Draw rectangle on foreground layer */

  stm32n6_ltdc_fill_fg_rect(
    bbox->x, bbox->y, bbox->w, 2, bbox->color);
  stm32n6_ltdc_fill_fg_rect(
    bbox->x, bbox->y + bbox->h - 2, bbox->w, 2,
    bbox->color);
  stm32n6_ltdc_fill_fg_rect(
    bbox->x, bbox->y, 2, bbox->h, bbox->color);
  stm32n6_ltdc_fill_fg_rect(
    bbox->x + bbox->w - 2, bbox->y, 2, bbox->h,
    bbox->color);
#endif
}

void display_hal_draw_stats(struct display_context_s *ctx,
                            const struct display_stats_s *stats)
{
  if (!ctx->initialized)
    {
      return;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Draw stats panel background (semi-transparent black bar) */

  stm32n6_ltdc_fill_fg_rect(0, 0,
    ctx->config.screen_width, 24, 0x2104);

  /* TODO: Draw text using font rendering
   * For now, rectangle indicates stats area.
   * Full text rendering requires font bitmap or GPU2D text.
   */
#endif

  (void)stats;
}

void display_hal_show_alert(struct display_context_s *ctx,
                            const char *msg)
{
  if (!ctx->initialized)
    {
      return;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Draw red alert bar at bottom of screen */

  uint32_t y = ctx->config.screen_height - 40;
  stm32n6_ltdc_fill_fg_rect(0, y,
    ctx->config.screen_width, 40, 0xf800);

  /* TODO: Draw alert text on top of red bar */
#endif

  syslog(LOG_WARNING, "display: ALERT: %s\n", msg);
}

void display_hal_swap(struct display_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

#ifdef CONFIG_ARCH_CHIP_STM32N6
  /* Swap foreground double buffer (tear-free) */

  void *safe_buf = stm32n6_ltdc_swap_fg_buffer();
  UNUSED(safe_buf);
#endif

  ctx->fg_write_idx = 1 - ctx->fg_write_idx;
}

void display_hal_deinit(struct display_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  memset(ctx, 0, sizeof(*ctx));
  syslog(LOG_INFO, "display: deinitialized\n");
}
