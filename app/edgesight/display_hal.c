/****************************************************************************
 * app/edgesight/display_hal.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * EdgeSight - Display HAL implementation.
 * Wraps LTDC dual-layer + GPU2D/DMA2D rendering for NuttX.
 *
 * TODO: Replace stub code with actual LTDC/DMA2D HAL calls when
 * hardware is available.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "display_hal.h"
#include "memory_map.h"
#include <string.h>
#include <stdio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* When building with real hardware:
 * #include "stm32n6xx_hal_ltdc.h"
 * #include "stm32n6xx_hal_dma2d.h"
 * #include "stm32n6xx_hal_gpu2d.h"
 */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int display_hal_init(struct display_context_s *ctx,
                     const struct display_config_s *cfg)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->config = *cfg;

  /* On real hardware:
   *   1. Configure LTDC clocks (IC17 from PLL2)
   *   2. Init LTDC with 2 layers:
   *      Layer0: RGB565, camera background
   *      Layer1: ARGB4444, overlay with color keying
   *   3. Set color key = 0x000000 (transparent black)
   *   4. Configure DMA2D for rectangle fill / copy operations
   *
   * Reference: SCRL_Init() in ObjectDetection example
   */

  ctx->fg_write_idx = 0;
  ctx->initialized = true;

  printf("[display] Initialized %lux%lu (stub)\n",
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

  /* On real hardware:
   *   SCRL_SetAddress_NoReload(buffer, SCRL_LAYER_0);
   *   SCRL_ReloadLayer(SCRL_LAYER_0);
   */
}

void display_hal_clear_fg(struct display_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  /* On real hardware:
   *   UTIL_LCD_FillRect(0, 0, width, height, UTIL_LCD_COLOR_TRANSPARENT);
   *   or use DMA2D fill with transparent color
   */
}

void display_hal_draw_bbox(struct display_context_s *ctx,
                           const struct display_bbox_s *bbox)
{
  if (!ctx->initialized)
    {
      return;
    }

  (void)bbox;

  /* On real hardware:
   *   UTIL_LCD_DrawRect(bbox->x, bbox->y, bbox->w, bbox->h, bbox->color);
   *   UTIL_LCDEx_PrintfAt(bbox->x, bbox->y, LEFT_MODE, bbox->label);
   *   UTIL_LCDEx_PrintfAt(-bbox->x-bbox->w, bbox->y, RIGHT_MODE,
   *                        "%.0f%%", bbox->confidence * 100.0f);
   */
}

void display_hal_draw_stats(struct display_context_s *ctx,
                            const struct display_stats_s *stats)
{
  if (!ctx->initialized)
    {
      return;
    }

  (void)stats;

  /* On real hardware:
   *   UTIL_LCD_SetBackColor(0x40000000);
   *   UTIL_LCDEx_PrintfAt(0, LINE(0), LEFT_MODE, "FPS: %u", stats->fps);
   *   UTIL_LCDEx_PrintfAt(0, LINE(1), LEFT_MODE, "Inf: %ums",
   *                        stats->inference_ms);
   *   UTIL_LCDEx_PrintfAt(0, LINE(0), RIGHT_MODE, "Persons: %u",
   *                        stats->persons_detected);
   *   if (stats->alert_active)
   *     UTIL_LCDEx_PrintfAt(0, LINE(2), CENTER_MODE, "!! FALL !!");
   */
}

void display_hal_show_alert(struct display_context_s *ctx,
                            const char *msg)
{
  if (!ctx->initialized)
    {
      return;
    }

  printf("[display] ALERT: %s\n", msg);

  /* On real hardware:
   *   Fill semi-transparent red background
   *   DMA2D fill with DISPLAY_COLOR_ALERT_BG
   *   UTIL_LCD_SetFont(&Font20);
   *   UTIL_LCDEx_PrintfAt(0, center_y, CENTER_MODE, msg);
   */
}

void display_hal_swap(struct display_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  /* On real hardware:
   *   SCB_CleanDCache_by_Addr(fg_buffer[write_idx], fg_size);
   *   __disable_irq();
   *   SCRL_SetAddress_NoReload(fg_buffer[write_idx], SCRL_LAYER_1);
   *   SCRL_ReloadLayer(SCRL_LAYER_1);
   *   __enable_irq();
   */

  ctx->fg_write_idx = 1 - ctx->fg_write_idx;
}

void display_hal_deinit(struct display_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  memset(ctx, 0, sizeof(*ctx));
  printf("[display] Deinitialized\n");
}
