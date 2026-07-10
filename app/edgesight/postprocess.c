/****************************************************************************
 * app/edgesight/postprocess.c
 *
 * SPDX-License-Identifier: Apache-2.0
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
   * Single output tensor with shape [1, N, 85] for COCO 80-class
   * or [1, N, 6] for single-class (person only).
   *
   * Each detection: [x_center, y_center, w, h, obj_conf, class_conf...]
   * Coordinates are normalized to [0..1] relative to input image.
   *
   * TODO: Adapt to actual st_yolo_x_nano output tensor layout.
   * The exact format depends on the stedgeai quantization output.
   * This is a placeholder that will be finalized when we can run
   * the model on real hardware.
   */

  int count = 0;
  const int8_t *data;
  uint32_t num_anchors;
  uint32_t stride;
  uint32_t i;

  (void)img_width;
  (void)img_height;

  if (num_outputs < 1 || raw_output[0] == NULL)
    {
      result->count = 0;
      return 0;
    }

  /* Estimate number of anchors from output size.
   * Assuming format: [num_anchors, 6] for person-only model
   * (x, y, w, h, objectness, person_conf) as INT8
   */

  stride = 6;
  data = (const int8_t *)raw_output[0];
  num_anchors = output_sizes[0] / stride;

  for (i = 0; i < num_anchors && count < PP_MAX_DETECTIONS; i++)
    {
      const int8_t *anchor = &data[i * stride];

      /* Dequantize: assuming zero_point=0, scale ~= 1/128 */

      float x = (float)anchor[0] / 128.0f;
      float y = (float)anchor[1] / 128.0f;
      float w = (float)anchor[2] / 128.0f;
      float h = (float)anchor[3] / 128.0f;
      float obj_conf = sigmoid((float)anchor[4] / 128.0f);
      float cls_conf = sigmoid((float)anchor[5] / 128.0f);
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
  /* MoveNet Lightning output format:
   * Single output tensor: [1, 17, 3] (17 keypoints x [y, x, confidence])
   * Values are normalized to [0..1] relative to the cropped input region.
   *
   * We need to map back to full-image coordinates using the detection bbox.
   *
   * TODO: Adapt dequantization to actual INT8 output parameters.
   */

  const int8_t *data = (const int8_t *)raw_output;
  uint32_t expected_size = KP_COUNT * 3; /* 17 * 3 = 51 bytes */
  int i;

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

  /* Parse keypoints */

  for (i = 0; i < KP_COUNT; i++)
    {
      /* Dequantize: INT8 → float [0..1] */

      float kp_y = ((float)data[i * 3 + 0] + 128.0f) / 255.0f;
      float kp_x = ((float)data[i * 3 + 1] + 128.0f) / 255.0f;
      float kp_c = ((float)data[i * 3 + 2] + 128.0f) / 255.0f;

      /* Map from crop-relative to full-image coordinates */

      result->kp[i].x = result->bbox_x + kp_x * result->bbox_w;
      result->kp[i].y = result->bbox_y + kp_y * result->bbox_h;
      result->kp[i].confidence = kp_c;
    }

  return 0;
}
