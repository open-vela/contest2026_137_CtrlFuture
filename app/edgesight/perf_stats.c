/****************************************************************************
 * app/edgesight/perf_stats.c
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
 * EdgeSight - Performance statistics implementation.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "perf_stats.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t get_ms(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)(ts.tv_sec * 1000 +
                    ts.tv_nsec / 1000000);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void perf_stats_init(struct perf_stats_s *stats)
{
  memset(stats, 0, sizeof(*stats));
  stats->frame_time.min_val = UINT32_MAX;
  stats->detect_time.min_val = UINT32_MAX;
  stats->pose_time.min_val = UINT32_MAX;
  stats->postproc_time.min_val = UINT32_MAX;
  stats->display_time.min_val = UINT32_MAX;
}

void perf_counter_add(struct perf_counter_s *counter,
                      uint32_t value_ms)
{
  /* Update rolling buffer */

  counter->samples[counter->index] = value_ms;
  counter->index =
    (counter->index + 1) % PERF_WINDOW_SIZE;

  if (counter->count < PERF_WINDOW_SIZE)
    {
      counter->count++;
    }

  /* Update min/max */

  if (value_ms < counter->min_val)
    {
      counter->min_val = value_ms;
    }

  if (value_ms > counter->max_val)
    {
      counter->max_val = value_ms;
    }

  counter->total += value_ms;
}

uint32_t perf_counter_avg(
    const struct perf_counter_s *counter)
{
  uint32_t sum = 0;
  uint16_t i;

  if (counter->count == 0)
    {
      return 0;
    }

  for (i = 0; i < counter->count; i++)
    {
      sum += counter->samples[i];
    }

  return sum / counter->count;
}

void perf_stats_frame_begin(struct perf_stats_s *stats)
{
  uint32_t now = get_ms();

  if (stats->last_frame_ts > 0)
    {
      uint32_t dt = now - stats->last_frame_ts;
      perf_counter_add(&stats->frame_time, dt);
    }

  stats->last_frame_ts = now;
  stats->frame_count++;
}

uint32_t perf_stats_get_fps(
    const struct perf_stats_s *stats)
{
  uint32_t avg_ms;

  avg_ms = perf_counter_avg(&stats->frame_time);
  if (avg_ms == 0)
    {
      return 0;
    }

  return 1000 / avg_ms;
}

void perf_stats_dump(const struct perf_stats_s *stats)
{
  printf("=== Performance Stats ===\n");
  printf("Frames: %lu  FPS: %lu\n",
         (unsigned long)stats->frame_count,
         (unsigned long)perf_stats_get_fps(stats));
  printf("Frame time: avg=%lums min=%lums"
         " max=%lums\n",
         (unsigned long)perf_counter_avg(
             &stats->frame_time),
         (unsigned long)stats->frame_time.min_val,
         (unsigned long)stats->frame_time.max_val);
  printf("YOLO infer: avg=%lums min=%lums"
         " max=%lums\n",
         (unsigned long)perf_counter_avg(
             &stats->detect_time),
         (unsigned long)stats->detect_time.min_val,
         (unsigned long)stats->detect_time.max_val);
  printf("Pose infer: avg=%lums min=%lums"
         " max=%lums\n",
         (unsigned long)perf_counter_avg(
             &stats->pose_time),
         (unsigned long)stats->pose_time.min_val,
         (unsigned long)stats->pose_time.max_val);
  printf("Detections: %lu  Falls: %lu\n",
         (unsigned long)stats->detect_count,
         (unsigned long)stats->fall_count);
}
