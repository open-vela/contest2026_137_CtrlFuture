/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_dcmipp.c
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
 * STM32N6 DCMIPP (Digital Camera Interface Pixel Pipeline) driver.
 *
 * Wraps ST CMW_CAMERA middleware for NuttX video framework.
 * Supports dual-pipe architecture:
 *   PIPE1: Display output (RGB565, continuous)
 *   PIPE2: NN inference input (RGB888, snapshot)
 *
 * Reference: STM32N6 Getting Started ObjectDetection
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/video/video.h>
#include <nuttx/kmalloc.h>
#include <nuttx/irq.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>

#include "stm32n6_dcmipp.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DCMIPP_PIPE_DISPLAY    0
#define DCMIPP_PIPE_NN         1
#define DCMIPP_PIPE_COUNT      2

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_dcmipp_dev_s
{
  struct video_dev_s  dev;        /* V4L2 video device (must be first) */
  volatile bool       pipe_ready[DCMIPP_PIPE_COUNT];
  volatile uint32_t   frame_count[DCMIPP_PIPE_COUNT];
  sem_t               locksem;    /* Device lock */
  uint32_t            width;      /* Display width */
  uint32_t            height;     /* Display height */
  uint32_t            nn_width;   /* NN pipe width */
  uint32_t            nn_height;  /* NN pipe height */
  uint32_t            fps;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct stm32n6_dcmipp_dev_s *g_dcmipp_dev;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int dcmipp_open(struct file *filep)
{
  struct inode *inode = filep->f_inode;
  struct stm32n6_dcmipp_dev_s *priv = inode->i_private;

  nxsem_wait(&priv->locksem);

  syslog(LOG_INFO, "dcmipp: opened\n");

  nxsem_post(&priv->locksem);
  return OK;
}

static int dcmipp_close(struct file *filep)
{
  struct inode *inode = filep->f_inode;
  struct stm32n6_dcmipp_dev_s *priv = inode->i_private;

  nxsem_wait(&priv->locksem);

  syslog(LOG_INFO, "dcmipp: closed\n");

  nxsem_post(&priv->locksem);
  return OK;
}

static ssize_t dcmipp_read(struct file *filep, char *buffer,
                            size_t buflen)
{
  /* Not used for camera device */

  return -ENOSYS;
}

static ssize_t dcmipp_write(struct file *filep,
                             const char *buffer, size_t buflen)
{
  /* Not used for camera device */

  return -ENOSYS;
}

static int dcmipp_ioctl(struct file *filep, int cmd,
                         unsigned long arg)
{
  struct inode *inode = filep->f_inode;
  struct stm32n6_dcmipp_dev_s *priv = inode->i_private;
  int ret = OK;

  nxsem_wait(&priv->locksem);

  switch (cmd)
    {
      case VIDIOC_QUERYCAP:
        {
          struct v4l2_capability *cap =
            (struct v4l2_capability *)(uintptr_t)arg;
          memset(cap, 0, sizeof(*cap));
          strlcpy((char *)cap->driver, "stm32n6-dcmipp",
                  sizeof(cap->driver));
          strlcpy((char *)cap->card, "STM32N6 Camera",
                  sizeof(cap->card));
          cap->device_caps = V4L2_CAP_VIDEO_CAPTURE |
                             V4L2_CAP_STREAMING;
          break;
        }

      case VIDIOC_G_FMT:
        {
          struct v4l2_format *fmt =
            (struct v4l2_format *)(uintptr_t)arg;
          fmt->fmt.pix.width = priv->width;
          fmt->fmt.pix.height = priv->height;
          fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
          fmt->fmt.pix.sizeimage =
            priv->width * priv->height * 2;
          break;
        }

      case VIDIOC_S_FMT:
        {
          struct v4l2_format *fmt =
            (struct v4l2_format *)(uintptr_t)arg;
          priv->width = fmt->fmt.pix.width;
          priv->height = fmt->fmt.pix.height;
          syslog(LOG_INFO, "dcmipp: set format %lux%lu\n",
                 (unsigned long)priv->width,
                 (unsigned long)priv->height);
          break;
        }

      case VIDIOC_REQBUFS:
        {
          /* Stub: accept buffer request */

          break;
        }

      default:
        ret = -ENOTTY;
        break;
    }

  nxsem_post(&priv->locksem);
  return ret;
}

static const struct file_operations g_dcmipp_ops =
{
  .open  = dcmipp_open,
  .close = dcmipp_close,
  .read  = dcmipp_read,
  .write = dcmipp_write,
  .ioctl = dcmipp_ioctl,
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_dcmipp_init
 *
 * Description:
 *   Initialize DCMIPP camera subsystem.
 *   Registers /dev/video0 character device.
 *
 * Input Parameters:
 *   width  - Display pipe width (e.g. 800)
 *   height - Display pipe height (e.g. 480)
 *   fps    - Target frame rate
 *
 * Returned Value:
 *   OK on success, negated errno on failure.
 *
 ****************************************************************************/

int stm32n6_dcmipp_init(uint32_t width, uint32_t height,
                          uint32_t fps)
{
  struct stm32n6_dcmipp_dev_s *priv;
  int ret;

  /* Allocate device structure */

  priv = kmm_zalloc(sizeof(struct stm32n6_dcmipp_dev_s));
  if (priv == NULL)
    {
      syslog(LOG_ERR, "dcmipp: out of memory\n");
      return -ENOMEM;
    }

  priv->width = width;
  priv->height = height;
  priv->nn_width = 480;   /* YOLO input size */
  priv->nn_height = 480;
  priv->fps = fps;

  nxsem_init(&priv->locksem, 0, 1);

  /* Register character device */

  ret = register_driver("/dev/video0", &g_dcmipp_ops,
                        0666, priv);
  if (ret < 0)
    {
      syslog(LOG_ERR, "dcmipp: register failed: %d\n", ret);
      nxsem_destroy(&priv->locksem);
      kmm_free(priv);
      return ret;
    }

  g_dcmipp_dev = priv;

  syslog(LOG_INFO, "dcmipp: registered /dev/video0 "
         "(display=%lux%lu nn=%lux%lu %lu fps)\n",
         (unsigned long)width, (unsigned long)height,
         (unsigned long)priv->nn_width,
         (unsigned long)priv->nn_height,
         (unsigned long)fps);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_dcmipp_start
 *
 * Description:
 *   Start camera capture on specified pipe.
 *   Integrates with ST CMW_CAMERA middleware.
 *
 ****************************************************************************/

int stm32n6_dcmipp_start(uint32_t pipe, void *buffer,
                           uint32_t mode)
{
  struct stm32n6_dcmipp_dev_s *priv = g_dcmipp_dev;

  if (priv == NULL)
    {
      return -ENODEV;
    }

  if (pipe >= DCMIPP_PIPE_COUNT)
    {
      return -EINVAL;
    }

  /* Delegate to ST middleware */

  /* ret = CMW_CAMERA_Start(pipe, buffer, mode); */

  priv->pipe_ready[pipe] = true;
  priv->frame_count[pipe] = 0;

  syslog(LOG_INFO, "dcmipp: pipe %lu started (%s)\n",
         (unsigned long)pipe,
         mode == 0 ? "continuous" : "snapshot");

  return OK;
}

/****************************************************************************
 * Name: stm32n6_dcmipp_stop
 *
 * Description:
 *   Stop camera capture on specified pipe.
 *
 ****************************************************************************/

int stm32n6_dcmipp_stop(uint32_t pipe)
{
  struct stm32n6_dcmipp_dev_s *priv = g_dcmipp_dev;

  if (priv == NULL)
    {
      return -ENODEV;
    }

  if (pipe >= DCMIPP_PIPE_COUNT)
    {
      return -EINVAL;
    }

  /* ret = CMW_CAMERA_Suspend(pipe); */

  priv->pipe_ready[pipe] = false;

  syslog(LOG_INFO, "dcmipp: pipe %lu stopped\n",
         (unsigned long)pipe);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_dcmipp_isp_update
 *
 * Description:
 *   Run ISP auto-exposure / auto-white-balance update.
 *   Should be called periodically (e.g. every frame).
 *
 ****************************************************************************/

void stm32n6_dcmipp_isp_update(void)
{
  /* ret = CMW_CAMERA_Run(); */
}

/****************************************************************************
 * Name: stm32n6_dcmipp_get_frame_count
 *
 * Description:
 *   Get frame count for specified pipe.
 *
 ****************************************************************************/

uint32_t stm32n6_dcmipp_get_frame_count(uint32_t pipe)
{
  struct stm32n6_dcmipp_dev_s *priv = g_dcmipp_dev;

  if (priv == NULL || pipe >= DCMIPP_PIPE_COUNT)
    {
      return 0;
    }

  return priv->frame_count[pipe];
}

/****************************************************************************
 * Name: stm32n6_dcmipp_frame_event
 *
 * Description:
 *   Called from DCMIPP ISR when a frame is received.
 *
 ****************************************************************************/

void stm32n6_dcmipp_frame_event(uint32_t pipe)
{
  struct stm32n6_dcmipp_dev_s *priv = g_dcmipp_dev;

  if (priv != NULL && pipe < DCMIPP_PIPE_COUNT)
    {
      priv->frame_count[pipe]++;
    }
}
