/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_ltdc.c
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
 * STM32N6 LTDC (LCD-TFT Display Controller) driver.
 *
 * Supports dual-layer overlay:
 *   Layer 0: Camera background (RGB565)
 *   Layer 1: Detection overlay (ARGB4444, transparent color keying)
 *
 * Reference: STM32N6 Getting Started ObjectDetection
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/video/fb.h>
#include <nuttx/kmalloc.h>
#include <nuttx/irq.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>

#include "stm32n6_ltdc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LTDC_LAYER_BG     0
#define LTDC_LAYER_FG     1
#define LTDC_LAYER_COUNT  2

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_ltdc_dev_s
{
  struct fb_vtable_s  vtable;     /* Framebuffer vtable (must be first) */
  sem_t               locksem;    /* Device lock */
  uint32_t            width;      /* Display width */
  uint32_t            height;     /* Display height */
  uint32_t            bg_fmt;     /* Background pixel format */
  uint32_t            fg_fmt;     /* Foreground pixel format */
  void               *bg_buffer;  /* Background framebuffer */
  void               *fg_buffer;  /* Foreground framebuffer */
  void               *fg_buffer2; /* Foreground second buffer */
  uint32_t            fg_idx;     /* Current foreground buffer index */
  uint32_t            fg_size;    /* Foreground buffer size */
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct stm32n6_ltdc_dev_s *g_ltdc_dev;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int ltdc_getvideoinfo(struct fb_vtable_s *vtable,
                              struct fb_videoinfo_s *vinfo)
{
  struct stm32n6_ltdc_dev_s *priv =
    (struct stm32n6_ltdc_dev_s *)vtable;

  vinfo->fmt = priv->bg_fmt;
  vinfo->xres = priv->width;
  vinfo->yres = priv->height;
  vinfo->nplanes = 1;

  return OK;
}

static int ltdc_getplaneinfo(struct fb_vtable_s *vtable,
                              int planeno,
                              struct fb_planeinfo_s *pinfo)
{
  struct stm32n6_ltdc_dev_s *priv =
    (struct stm32n6_ltdc_dev_s *)vtable;

  if (planeno != 0)
    {
      return -EINVAL;
    }

  pinfo->fbmem = priv->bg_buffer;
  pinfo->fblen = priv->width * priv->height * 2;
  pinfo->stride = priv->width * 2;
  pinfo->display = 0;
  pinfo->bpp = 16;

  return OK;
}

static int ltdc_open(struct fb_vtable_s *vtable)
{
  syslog(LOG_INFO, "ltdc: opened\n");
  return OK;
}

static int ltdc_close(struct fb_vtable_s *vtable)
{
  syslog(LOG_INFO, "ltdc: closed\n");
  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_ltdc_init
 *
 * Description:
 *   Initialize LTDC display controller.
 *   Registers /dev/fb0 framebuffer device.
 *
 *   Configuration:
 *     - Layer 0 (background): Camera feed, RGB565, continuous update
 *     - Layer 1 (foreground): Detection overlay, ARGB4444, transparent
 *     - Color keying: 0x000000 (black = transparent)
 *     - Double-buffered foreground for tear-free overlay updates
 *
 * Input Parameters:
 *   width   - Display width (e.g. 800)
 *   height  - Display height (e.g. 480)
 *   bg_buf  - Background framebuffer (camera output)
 *   fg_buf1 - Foreground framebuffer 1 (overlay)
 *   fg_buf2 - Foreground framebuffer 2 (overlay, double-buffer)
 *
 * Returned Value:
 *   OK on success, negated errno on failure.
 *
 ****************************************************************************/

int stm32n6_ltdc_init(uint32_t width, uint32_t height,
                        void *bg_buf, void *fg_buf1,
                        void *fg_buf2)
{
  struct stm32n6_ltdc_dev_s *priv;
  int ret;

  /* Allocate device structure */

  priv = kmm_zalloc(sizeof(struct stm32n6_ltdc_dev_s));
  if (priv == NULL)
    {
      syslog(LOG_ERR, "ltdc: out of memory\n");
      return -ENOMEM;
    }

  priv->width = width;
  priv->height = height;
  priv->bg_fmt = FB_FMT_RGB16_565;
  priv->fg_fmt = FB_FMT_RGB16_565;  /* ARGB4444 packed as 16-bit */
  priv->bg_buffer = bg_buf;
  priv->fg_buffer = fg_buf1;
  priv->fg_buffer2 = fg_buf2;
  priv->fg_idx = 0;
  priv->fg_size = width * height * 2;

  nxsem_init(&priv->locksem, 0, 1);

  /* Initialize vtable */

  priv->vtable.getvideoinfo = ltdc_getvideoinfo;
  priv->vtable.getplaneinfo = ltdc_getplaneinfo;
  priv->vtable.open = ltdc_open;
  priv->vtable.close = ltdc_close;

  /* Register framebuffer device */

  ret = register_framebuffer("/dev/fb0", &priv->vtable);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ltdc: register failed: %d\n", ret);
      nxsem_destroy(&priv->locksem);
      kmm_free(priv);
      return ret;
    }

  g_ltdc_dev = priv;

  syslog(LOG_INFO, "ltdc: registered /dev/fb0 (%lux%lu "
         "RGB565+ARGB4444)\n",
         (unsigned long)width, (unsigned long)height);

  /* TODO: Configure LTDC hardware registers
   *
   * 1. LTDC timing parameters (HSYNC, VSYNC, HBP, VBP, etc.)
   * 2. Layer 0: bg_buf, RGB565, full screen
   * 3. Layer 1: fg_buf1, ARGB4444, full screen, color key 0x000000
   * 4. Enable LTDC
   *
   * Reference: HAL_LTDC_Init() + HAL_LTDC_ConfigLayer()
   */

  return OK;
}

/****************************************************************************
 * Name: stm32n6_ltdc_set_bg_buffer
 *
 * Description:
 *   Update background layer buffer address.
 *   Used to swap camera frame buffers.
 *
 ****************************************************************************/

int stm32n6_ltdc_set_bg_buffer(void *buffer)
{
  struct stm32n6_ltdc_dev_s *priv = g_ltdc_dev;

  if (priv == NULL)
    {
      return -ENODEV;
    }

  priv->bg_buffer = buffer;

  /* TODO: Update LTDC Layer 0 framebuffer address
   * HAL_LTDC_SetAddress_NoReload(&hltdc, (uint32_t)buffer, LTDC_LAYER_BG);
   * HAL_LTDC_ReloadLayer(&hltdc, LTDC_RELOAD_IMMEDIATE, LTDC_LAYER_BG);
   */

  return OK;
}

/****************************************************************************
 * Name: stm32n6_ltdc_swap_fg_buffer
 *
 * Description:
 *   Swap foreground double buffer for tear-free overlay update.
 *   Returns pointer to the buffer that is now safe to write.
 *
 ****************************************************************************/

void *stm32n6_ltdc_swap_fg_buffer(void)
{
  struct stm32n6_ltdc_dev_s *priv = g_ltdc_dev;
  void *safe_buffer;

  if (priv == NULL)
    {
      return NULL;
    }

  /* Swap buffers */

  if (priv->fg_idx == 0)
    {
      priv->fg_idx = 1;
      safe_buffer = priv->fg_buffer2;
    }
  else
    {
      priv->fg_idx = 0;
      safe_buffer = priv->fg_buffer;
    }

  /* TODO: Update LTDC Layer 1 framebuffer address
   * void *display_buf = (priv->fg_idx == 0) ?
   *                     priv->fg_buffer : priv->fg_buffer2;
   * HAL_LTDC_SetAddress_NoReload(&hltdc,
   *                              (uint32_t)display_buf,
   *                              LTDC_LAYER_FG);
   * HAL_LTDC_ReloadLayer(&hltdc, LTDC_RELOAD_IMMEDIATE,
   *                      LTDC_LAYER_FG);
   */

  return safe_buffer;
}

/****************************************************************************
 * Name: stm32n6_ltdc_fill_fg_rect
 *
 * Description:
 *   Fill a rectangle on the foreground layer.
 *   Used for drawing bounding boxes and stats panel.
 *
 ****************************************************************************/

int stm32n6_ltdc_fill_fg_rect(uint32_t x, uint32_t y,
                                uint32_t w, uint32_t h,
                                uint16_t color)
{
  struct stm32n6_ltdc_dev_s *priv = g_ltdc_dev;
  uint16_t *buf;
  uint32_t row;
  uint32_t col;

  if (priv == NULL)
    {
      return -ENODEV;
    }

  /* Clamp to screen bounds */

  if (x >= priv->width || y >= priv->height)
    {
      return -EINVAL;
    }

  if (x + w > priv->width)
    {
      w = priv->width - x;
    }

  if (y + h > priv->height)
    {
      h = priv->height - y;
    }

  /* Get current foreground buffer (the one being displayed) */

  buf = (priv->fg_idx == 0) ?
        (uint16_t *)priv->fg_buffer :
        (uint16_t *)priv->fg_buffer2;

  /* Fill rectangle */

  for (row = 0; row < h; row++)
    {
      uint16_t *line = &buf[(y + row) * priv->width + x];
      for (col = 0; col < w; col++)
        {
          line[col] = color;
        }
    }

  return OK;
}

/****************************************************************************
 * Name: stm32n6_ltdc_clear_fg
 *
 * Description:
 *   Clear foreground layer to transparent (0x0000).
 *
 ****************************************************************************/

void stm32n6_ltdc_clear_fg(void)
{
  struct stm32n6_ltdc_dev_s *priv = g_ltdc_dev;

  if (priv == NULL)
    {
      return;
    }

  /* Get the safe buffer (not currently displayed) */

  void *buf = (priv->fg_idx == 0) ?
              priv->fg_buffer2 : priv->fg_buffer;

  memset(buf, 0, priv->fg_size);
}

/****************************************************************************
 * Name: stm32n6_ltdc_get_fg_buffer
 *
 * Description:
 *   Get pointer to the safe-to-write foreground buffer.
 *
 ****************************************************************************/

void *stm32n6_ltdc_get_fg_buffer(void)
{
  struct stm32n6_ltdc_dev_s *priv = g_ltdc_dev;

  if (priv == NULL)
    {
      return NULL;
    }

  return (priv->fg_idx == 0) ?
         priv->fg_buffer2 : priv->fg_buffer;
}
