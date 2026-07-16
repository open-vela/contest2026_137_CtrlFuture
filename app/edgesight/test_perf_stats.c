/****************************************************************************
 * app/edgesight/test_perf_stats.c
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
 * EdgeSight - Unit test for performance statistics module.
 * Verifies rolling window, min/max/avg, and FPS calculation.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "perf_stats.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_counter_basic(void)
{
  struct perf_counter_s counter;

  memset(&counter, 0, sizeof(counter));
  counter.min_val = UINT32_MAX;

  perf_counter_add(&counter, 10);
  perf_counter_add(&counter, 20);
  perf_counter_add(&counter, 30);

  uint32_t avg = perf_counter_avg(&counter);

  if (avg != 20)
    {
      printf("  FAIL: avg=%lu (expected 20)\n",
             (unsigned long)avg);
      return -1;
    }

  if (counter.min_val != 10)
    {
      printf("  FAIL: min=%lu (expected 10)\n",
             (unsigned long)counter.min_val);
      return -1;
    }

  if (counter.max_val != 30)
    {
      printf("  FAIL: max=%lu (expected 30)\n",
             (unsigned long)counter.max_val);
      return -1;
    }

  printf("  PASS: basic counter (avg=%lu min=%lu max=%lu)\n",
         (unsigned long)avg,
         (unsigned long)counter.min_val,
         (unsigned long)counter.max_val);
  return 0;
}

static int test_rolling_window(void)
{
  struct perf_counter_s counter;
  int i;

  memset(&counter, 0, sizeof(counter));
  counter.min_val = UINT32_MAX;

  /* Fill beyond window size */

  for (i = 1; i <= PERF_WINDOW_SIZE + 10; i++)
    {
      perf_counter_add(&counter, i);
    }

  if (counter.count != PERF_WINDOW_SIZE)
    {
      printf("  FAIL: count=%u (expected %u)\n",
             counter.count, PERF_WINDOW_SIZE);
      return -1;
    }

  /* Average should be of last PERF_WINDOW_SIZE values */

  uint32_t avg = perf_counter_avg(&counter);

  if (avg == 0)
    {
      printf("  FAIL: avg=0\n");
      return -1;
    }

  printf("  PASS: rolling window (count=%u avg=%lu)\n",
         counter.count, (unsigned long)avg);
  return 0;
}

static int test_empty_counter(void)
{
  struct perf_counter_s counter;

  memset(&counter, 0, sizeof(counter));

  uint32_t avg = perf_counter_avg(&counter);

  if (avg != 0)
    {
      printf("  FAIL: empty avg=%lu (expected 0)\n",
             (unsigned long)avg);
      return -1;
    }

  printf("  PASS: empty counter returns 0\n");
  return 0;
}

static int test_fps_calculation(void)
{
  struct perf_stats_s stats;

  perf_stats_init(&stats);

  /* Simulate 10 frames at ~33ms intervals */

  stats.last_frame_ts = 0;
  int i;

  for (i = 0; i < 10; i++)
    {
      stats.last_frame_ts = i * 33;
      perf_stats_frame_begin(&stats);
    }

  uint32_t fps = perf_stats_get_fps(&stats);

  /* Should be ~30 fps (1000/33) */

  if (fps < 20 || fps > 40)
    {
      printf("  FAIL: fps=%lu (expected ~30)\n",
             (unsigned long)fps);
      return -1;
    }

  printf("  PASS: fps=%lu (frame_count=%lu)\n",
         (unsigned long)fps,
         (unsigned long)stats.frame_count);
  return 0;
}

static int test_perf_stats_init(void)
{
  struct perf_stats_s stats;

  perf_stats_init(&stats);

  if (stats.frame_count != 0)
    {
      printf("  FAIL: frame_count=%lu\n",
             (unsigned long)stats.frame_count);
      return -1;
    }

  if (stats.detect_count != 0)
    {
      printf("  FAIL: detect_count=%lu\n",
             (unsigned long)stats.detect_count);
      return -1;
    }

  printf("  PASS: init zeroes all fields\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_perf_stats_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("============================\n");
  printf(" EdgeSight Perf Stats Test\n");
  printf("============================\n\n");

  printf("[1] Basic counter:\n");
  failures += (test_counter_basic() != 0);

  printf("\n[2] Rolling window:\n");
  failures += (test_rolling_window() != 0);

  printf("\n[3] Empty counter:\n");
  failures += (test_empty_counter() != 0);

  printf("\n[4] FPS calculation:\n");
  failures += (test_fps_calculation() != 0);

  printf("\n[5] Init check:\n");
  failures += (test_perf_stats_init() != 0);

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
