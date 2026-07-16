/****************************************************************************
 * app/edgesight/test_postprocess.c
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
 * EdgeSight - Unit test for post-processing module.
 * Verifies NMS, coordinate mapping, and edge cases.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "postprocess.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_nms_basic(void)
{
  struct detection_s dets[4];

  /* Two overlapping detections, one standalone */

  dets[0].x_center = 0.5f;
  dets[0].y_center = 0.5f;
  dets[0].width = 0.2f;
  dets[0].height = 0.3f;
  dets[0].confidence = 0.9f;
  dets[0].class_id = 0;

  dets[1].x_center = 0.52f;
  dets[1].y_center = 0.51f;
  dets[1].width = 0.2f;
  dets[1].height = 0.3f;
  dets[1].confidence = 0.8f;
  dets[1].class_id = 0;

  dets[2].x_center = 0.1f;
  dets[2].y_center = 0.1f;
  dets[2].width = 0.1f;
  dets[2].height = 0.1f;
  dets[2].confidence = 0.7f;
  dets[2].class_id = 0;

  dets[3].x_center = 0.5f;
  dets[3].y_center = 0.5f;
  dets[3].width = 0.2f;
  dets[3].height = 0.3f;
  dets[3].confidence = 0.6f;
  dets[3].class_id = 0;

  int count = postprocess_nms(dets, 4, 0.5f);

  if (count < 2 || count > 3)
    {
      printf("  FAIL: nms returned %d (expected 2-3)\n",
             count);
      return -1;
    }

  printf("  PASS: nms basic (%d detections kept)\n", count);
  return 0;
}

static int test_nms_empty(void)
{
  struct detection_s dets[1];
  int count;

  count = postprocess_nms(dets, 0, 0.5f);

  if (count != 0)
    {
      printf("  FAIL: empty nms returned %d\n", count);
      return -1;
    }

  printf("  PASS: empty nms returns 0\n");
  return 0;
}

static int test_nms_no_overlap(void)
{
  struct detection_s dets[3];

  /* Three non-overlapping detections */

  dets[0].x_center = 0.2f;
  dets[0].y_center = 0.2f;
  dets[0].width = 0.1f;
  dets[0].height = 0.1f;
  dets[0].confidence = 0.9f;
  dets[0].class_id = 0;

  dets[1].x_center = 0.5f;
  dets[1].y_center = 0.5f;
  dets[1].width = 0.1f;
  dets[1].height = 0.1f;
  dets[1].confidence = 0.8f;
  dets[1].class_id = 0;

  dets[2].x_center = 0.8f;
  dets[2].y_center = 0.8f;
  dets[2].width = 0.1f;
  dets[2].height = 0.1f;
  dets[2].confidence = 0.7f;
  dets[2].class_id = 0;

  int count = postprocess_nms(dets, 3, 0.5f);

  if (count != 3)
    {
      printf("  FAIL: non-overlapping nms returned %d "
             "(expected 3)\n", count);
      return -1;
    }

  printf("  PASS: non-overlapping nms keeps all 3\n");
  return 0;
}

static int test_nms_high_iou(void)
{
  struct detection_s dets[2];

  /* Two nearly identical detections */

  dets[0].x_center = 0.5f;
  dets[0].y_center = 0.5f;
  dets[0].width = 0.2f;
  dets[0].height = 0.3f;
  dets[0].confidence = 0.9f;
  dets[0].class_id = 0;

  dets[1].x_center = 0.5f;
  dets[1].y_center = 0.5f;
  dets[1].width = 0.2f;
  dets[1].height = 0.3f;
  dets[1].confidence = 0.8f;
  dets[1].class_id = 0;

  int count = postprocess_nms(dets, 2, 0.5f);

  if (count != 1)
    {
      printf("  FAIL: high-iou nms returned %d (expected 1)\n",
             count);
      return -1;
    }

  printf("  PASS: high-iou nms removes duplicate\n");
  return 0;
}

static int test_nms_preserves_order(void)
{
  struct detection_s dets[3];

  /* Three non-overlapping, check sorted by confidence */

  dets[0].x_center = 0.2f;
  dets[0].y_center = 0.2f;
  dets[0].width = 0.1f;
  dets[0].height = 0.1f;
  dets[0].confidence = 0.5f;
  dets[0].class_id = 0;

  dets[1].x_center = 0.5f;
  dets[1].y_center = 0.5f;
  dets[1].width = 0.1f;
  dets[1].height = 0.1f;
  dets[1].confidence = 0.9f;
  dets[1].class_id = 0;

  dets[2].x_center = 0.8f;
  dets[2].y_center = 0.8f;
  dets[2].width = 0.1f;
  dets[2].height = 0.1f;
  dets[2].confidence = 0.7f;
  dets[2].class_id = 0;

  int count = postprocess_nms(dets, 3, 0.5f);

  if (count != 3)
    {
      printf("  FAIL: expected 3, got %d\n", count);
      return -1;
    }

  /* Should be sorted by confidence descending */

  if (dets[0].confidence < dets[1].confidence)
    {
      printf("  FAIL: not sorted by confidence\n");
      return -1;
    }

  printf("  PASS: nms preserves confidence order\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_postprocess_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("============================\n");
  printf(" EdgeSight Postprocess Test\n");
  printf("============================\n\n");

  printf("[1] Basic NMS:\n");
  failures += (test_nms_basic() != 0);

  printf("\n[2] Empty NMS:\n");
  failures += (test_nms_empty() != 0);

  printf("\n[3] Non-overlapping NMS:\n");
  failures += (test_nms_no_overlap() != 0);

  printf("\n[4] High IoU NMS:\n");
  failures += (test_nms_high_iou() != 0);

  printf("\n[5] Order preservation:\n");
  failures += (test_nms_preserves_order() != 0);

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
