/****************************************************************************
 * app/edgesight/postprocess.c
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
 * EdgeSight - AI model post-processing implementation.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "postprocess.h"
#include <math.h>
#include <string.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief Compute Intersection over Union of two boxes
 */

static float calc_iou(const struct detection_s *a,
                      const struct detection_s *b)
{
  float a_x1 = a->x_center - a->width / 2.0f;
  float a_y1 = a->y_center - a->height / 2.0f;
  float a_x2 = a->x_center + a->width / 2.0f;
  float a_y2 = a->y_center + a->height / 2.0f;

  float b_x1 = b->x_center - b->width / 2.0f;
  float b_y1 = b->y_center - b->height / 2.0f;
  float b_x2 = b->x_center + b->width / 2.0f;
  float b_y2 = b->y_center + b->height / 2.0f;

  float inter_x1 = (a_x1 > b_x1) ? a_x1 : b_x1;
  float inter_y1 = (a_y1 > b_y1) ? a_y1 : b_y1;
  float inter_x2 = (a_x2 < b_x2) ? a_x2 : b_x2;
  float inter_y2 = (a_y2 < b_y2) ? a_y2 : b_y2;

  float inter_w = inter_x2 - inter_x1;
  float inter_h = inter_y2 - inter_y1;

  if (inter_w <= 0.0f || inter_h <= 0.0f)
    {
      return 0.0f;
    }

  float inter_area = inter_w * inter_h;
  float a_area = a->width * a->height;
  float b_area = b->width * b->height;
  float union_area = a_area + b_area - inter_area;

  if (union_area <= 0.0f)
    {
      return 0.0f;
    }

  return inter_area / union_area;
}

/**
 * @brief Simple insertion sort by confidence (descending)
 */

static void sort_by_confidence(struct detection_s *dets, int count)
{
  int i;
  int j;

  for (i = 1; i < count; i++)
    {
      struct detection_s key = dets[i];
      j = i - 1;
      while (j >= 0 && dets[j].confidence < key.confidence)
        {
          dets[j + 1] = dets[j];
          j--;
        }

      dets[j + 1] = key;
    }
}

/**
 * @brief Sigmoid activation
 */

static inline float sigmoid(float x)
{
  return 1.0f / (1.0f + expf(-x));
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int postprocess_nms(struct detection_s *dets, int count,
                    float iou_thresh)
{
  bool suppressed[PP_MAX_DETECTIONS];
  int kept = 0;
  int i;
  int j;

  if (count > PP_MAX_DETECTIONS)
    {
      count = PP_MAX_DETECTIONS;
    }

  memset(suppressed, 0, sizeof(suppressed));
  sort_by_confidence(dets, count);

  for (i = 0; i < count; i++)
    {
      if (suppressed[i])
        {
          continue;
        }

      /* Keep this detection */

      if (kept != i)
        {
          dets[kept] = dets[i];
        }

      kept++;

      /* Suppress overlapping detections */

      for (j = i + 1; j < count; j++)
        {
          if (!suppressed[j] &&
              calc_iou(&dets[i], &dets[j]) > iou_thresh)
            {
              suppressed[j] = true;
            }
        }
    }

  return kept;
}

int postprocess_yolo(const void *raw_output[],
                     const uint32_t output_sizes[],
                     uint32_t num_outputs,
                     uint32_t img_width,
                     uint32_t img_height,
                     struct detection_output_s *result)
{
  /* YOLO-X nano output format (INT8 quantized):
   * Single output tensor, format depends on stedgeai compilation.
   *
   * INT8 dequantization formula (from ST reference):
   *   float_value = (int8_value - zero_point) * scale
   *
   * Parameters obtained from stai_network_get_info():
   *   info.outputs[n].quant.scale
   *   info.outputs[n].quant.zero_point
   *
   * Reference: od_pp_st_yolox.c st_yolox_pp_level_decode_and_store_is8()
   */

  int count = 0;
  const int8_t *data;
  uint32_t num_anchors;
  uint32_t stride;
  uint32_t i;

  /* Quantization parameters from model output tensor info.
   * These should be obtained at runtime from stai_network_get_info().
   * Default values from typical YOLO-X nano INT8 compilation.
   */

  float raw_scale = 0.03137f;  /* ~1/32, typical INT8 scale */
  int8_t raw_zp = 0;           /* zero point, typically 0 */

  (void)img_width;
  (void)img_height;

  if (num_outputs < 1 || raw_output[0] == NULL)
    {
      result->count = 0;
      return 0;
    }

  /* Format: [num_anchors, 6] for person-only
   * (x, y, w, h, objectness, person_conf)
   */

  stride = 6;
  data = (const int8_t *)raw_output[0];
  num_anchors = output_sizes[0] / stride;

  for (i = 0; i < num_anchors && count < PP_MAX_DETECTIONS; i++)
    {
      const int8_t *anchor = &data[i * stride];

      /* Dequantize using ST formula:
       * float_value = (int8_value - zero_point) * scale
       */

      float x = (float)(anchor[0] - raw_zp) * raw_scale;
      float y = (float)(anchor[1] - raw_zp) * raw_scale;
      float w = (float)(anchor[2] - raw_zp) * raw_scale;
      float h = (float)(anchor[3] - raw_zp) * raw_scale;

      /* Objectness and class confidence need sigmoid */

      float obj_raw =
        (float)(anchor[4] - raw_zp) * raw_scale;
      float cls_raw =
        (float)(anchor[5] - raw_zp) * raw_scale;
      float obj_conf = sigmoid(obj_raw);
      float cls_conf = sigmoid(cls_raw);
      float conf = obj_conf * cls_conf;

      if (conf < PP_CONF_THRESH)
        {
          continue;
        }

      result->detections[count].x_center = x;
      result->detections[count].y_center = y;
      result->detections[count].width = w;
      result->detections[count].height = h;
      result->detections[count].confidence = conf;
      result->detections[count].class_id = PP_PERSON_CLASS_ID;
      count++;
    }

  /* Apply NMS */

  count = postprocess_nms(result->detections, count, PP_NMS_IOU_THRESH);
  result->count = (uint32_t)count;

  return count;
}

int postprocess_movenet(const void *raw_output,
                        uint32_t output_size,
                        const struct detection_s *det,
                        struct pose_result_s *result)
{
  /* MoveNet Lightning output format (INT8 quantized):
   * Heatmap: [1, num_keypoints, heatmap_h, heatmap_w]
   * INT8 dequant (ST reference):
   *   float = (int8_value - zero_point) * scale
   *
   * Reference: spe_movenet_pp.c movenet_heatmap_max_int8()
   */

  const int8_t *data = (const int8_t *)raw_output;
  uint32_t expected_size = KP_COUNT * 3;
  int i;

  /* Quantization params - get from stai_network_get_info() */

  float raw_scale = 0.03137f;
  int8_t raw_zp = 0;

  if (raw_output == NULL || output_size < expected_size)
    {
      memset(result, 0, sizeof(struct pose_result_s));
      return -1;
    }

  /* Fill bounding box from detection */

  result->bbox_x = det->x_center - det->width / 2.0f;
  result->bbox_y = det->y_center - det->height / 2.0f;
  result->bbox_w = det->width;
  result->bbox_h = det->height;
  result->det_conf = det->confidence;

  /* Parse keypoints with INT8 dequantization */

  for (i = 0; i < KP_COUNT; i++)
    {
      int8_t kp_y_raw = data[i * 3 + 0];
      int8_t kp_x_raw = data[i * 3 + 1];
      int8_t kp_c_raw = data[i * 3 + 2];

      float kp_y =
        (float)(kp_y_raw - raw_zp) * raw_scale;
      float kp_x =
        (float)(kp_x_raw - raw_zp) * raw_scale;
      float kp_c =
        (float)(kp_c_raw - raw_zp) * raw_scale;

      /* Clamp to [0..1] */

      if (kp_y < 0.0f)
        {
          kp_y = 0.0f;
        }

      if (kp_y > 1.0f)
        {
          kp_y = 1.0f;
        }

      if (kp_x < 0.0f)
        {
          kp_x = 0.0f;
        }

      if (kp_x > 1.0f)
        {
          kp_x = 1.0f;
        }

      /* Map from crop-relative to full-image coordinates */

      result->kp[i].x =
        result->bbox_x + kp_x * result->bbox_w;
      result->kp[i].y =
        result->bbox_y + kp_y * result->bbox_h;
      result->kp[i].confidence = kp_c;
    }

  return 0;
}
