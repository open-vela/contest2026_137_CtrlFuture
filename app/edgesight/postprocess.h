/****************************************************************************
 * app/edgesight/postprocess.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * EdgeSight - AI model post-processing routines.
 * Parses raw NPU output tensors into structured detection results.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_POSTPROCESS_H
#define __APP_EDGESIGHT_POSTPROCESS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "fall_detect.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PP_MAX_DETECTIONS    20   /* Max persons to track */
#define PP_NMS_IOU_THRESH    0.45f
#define PP_CONF_THRESH       0.35f
#define PP_PERSON_CLASS_ID   0    /* COCO class 0 = person */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Single object detection result */

struct detection_s
{
  float x_center;    /* Normalized [0..1] */
  float y_center;
  float width;
  float height;
  float confidence;
  uint8_t class_id;
};

/* Object detection output (from YOLO) */

struct detection_output_s
{
  struct detection_s detections[PP_MAX_DETECTIONS];
  uint32_t count;
};

/* Pose estimation output (from MoveNet) */

struct pose_output_s
{
  struct pose_result_s poses[PP_MAX_DETECTIONS];
  uint32_t count;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Post-process YOLO detection output
 * @param raw_output   Raw NPU output buffer(s)
 * @param output_sizes Size of each output tensor
 * @param num_outputs  Number of output tensors
 * @param img_width    Input image width (for denormalization)
 * @param img_height   Input image height
 * @param result       Output: parsed detections (persons only)
 * @return number of persons detected
 */

int postprocess_yolo(const void *raw_output[],
                     const uint32_t output_sizes[],
                     uint32_t num_outputs,
                     uint32_t img_width,
                     uint32_t img_height,
                     struct detection_output_s *result);

/**
 * @brief Post-process MoveNet pose estimation output
 * @param raw_output   Raw NPU output buffer (heatmaps + offsets)
 * @param output_size  Size of output tensor
 * @param det          Detection bounding box (for coordinate mapping)
 * @param result       Output: parsed pose keypoints
 * @return 0 on success
 */

int postprocess_movenet(const void *raw_output,
                        uint32_t output_size,
                        const struct detection_s *det,
                        struct pose_result_s *result);

/**
 * @brief Apply Non-Maximum Suppression to detection list
 * @param dets     Array of detections (modified in place)
 * @param count    Number of detections
 * @param iou_thresh IoU threshold for suppression
 * @return number of detections remaining
 */

int postprocess_nms(struct detection_s *dets, int count,
                    float iou_thresh);

#endif /* __APP_EDGESIGHT_POSTPROCESS_H */
