/****************************************************************************
 * app/edgesight/test_event_log.c
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
 * EdgeSight - Unit test for event logging module.
 * Verifies ring buffer, drop counting, and dump functionality.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "event_log.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_basic_write(void)
{
  struct event_log_s log;
  int ret;

  event_log_init(&log, NULL);

  ret = event_log_write(&log, EVENT_LEVEL_INFO, 1,
                         "test message 1");
  if (ret < 0)
    {
      printf("  FAIL: write returned %d\n", ret);
      return -1;
    }

  ret = event_log_write(&log, EVENT_LEVEL_ALERT, 2,
                         "test alert");
  if (ret < 0)
    {
      printf("  FAIL: write returned %d\n", ret);
      return -1;
    }

  uint32_t total;
  uint32_t flushed;
  uint16_t dropped;

  event_log_stats(&log, &total, &flushed, &dropped);

  if (total != 2)
    {
      printf("  FAIL: total=%lu (expected 2)\n",
             (unsigned long)total);
      return -1;
    }

  if (dropped != 0)
    {
      printf("  FAIL: dropped=%u (expected 0)\n", dropped);
      return -1;
    }

  printf("  PASS: basic write (%lu entries)\n",
         (unsigned long)total);
  return 0;
}

static int test_ring_buffer_overflow(void)
{
  struct event_log_s log;
  int i;
  int ret;

  event_log_init(&log, NULL);

  /* Fill ring buffer beyond capacity */

  for (i = 0; i < EVENT_LOG_RING_SIZE + 10; i++)
    {
      ret = event_log_write(&log, EVENT_LEVEL_INFO, i,
                             "message %d", i);
      if (i >= EVENT_LOG_RING_SIZE && ret == -1)
        {
          /* Expected: overflow */

          continue;
        }
    }

  uint32_t total;
  uint32_t flushed;
  uint16_t dropped;

  event_log_stats(&log, &total, &flushed, &dropped);

  if (dropped < 10)
    {
      printf("  FAIL: dropped=%u (expected >= 10)\n",
             dropped);
      return -1;
    }

  if (total != (uint32_t)(EVENT_LOG_RING_SIZE + 10))
    {
      printf("  FAIL: total=%lu (expected %d)\n",
             (unsigned long)total,
             EVENT_LOG_RING_SIZE + 10);
      return -1;
    }

  printf("  PASS: overflow handled (dropped=%u, total=%lu)\n",
         dropped, (unsigned long)total);
  return 0;
}

static int test_log_levels(void)
{
  struct event_log_s log;

  event_log_init(&log, NULL);

  event_log_write(&log, EVENT_LEVEL_DEBUG, 0, "debug");
  event_log_write(&log, EVENT_LEVEL_INFO, 0, "info");
  event_log_write(&log, EVENT_LEVEL_WARN, 0, "warn");
  event_log_write(&log, EVENT_LEVEL_ALERT, 0, "alert");
  event_log_write(&log, EVENT_LEVEL_ERROR, 0, "error");

  uint32_t total;
  uint32_t flushed;
  uint16_t dropped;

  event_log_stats(&log, &total, &flushed, &dropped);

  if (total != 5)
    {
      printf("  FAIL: total=%lu (expected 5)\n",
             (unsigned long)total);
      return -1;
    }

  printf("  PASS: all 5 log levels written\n");
  return 0;
}

static int test_dump(void)
{
  struct event_log_s log;

  event_log_init(&log, NULL);

  event_log_write(&log, EVENT_LEVEL_INFO, 100, "test 1");
  event_log_write(&log, EVENT_LEVEL_ALERT, 200, "test 2");
  event_log_write(&log, EVENT_LEVEL_ERROR, 300, "test 3");

  printf("  (dumping 3 entries)\n");
  event_log_dump(&log, 3);

  printf("  PASS: dump completed\n");
  return 0;
}

static int test_init_deinit(void)
{
  struct event_log_s log;
  int ret;

  ret = event_log_init(&log, "/tmp/test_log");
  if (ret < 0)
    {
      printf("  FAIL: init returned %d\n", ret);
      return -1;
    }

  /* Write some entries */

  event_log_write(&log, EVENT_LEVEL_INFO, 1, "before deinit");

  /* Stats should be available */

  uint32_t total;
  uint32_t flushed;
  uint16_t dropped;

  event_log_stats(&log, &total, &flushed, &dropped);

  if (total != 1)
    {
      printf("  FAIL: total=%lu (expected 1)\n",
             (unsigned long)total);
      return -1;
    }

  printf("  PASS: init/deinit cycle works\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_event_log_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("============================\n");
  printf(" EdgeSight Event Log Test\n");
  printf("============================\n\n");

  printf("[1] Basic write:\n");
  failures += (test_basic_write() != 0);

  printf("\n[2] Ring buffer overflow:\n");
  failures += (test_ring_buffer_overflow() != 0);

  printf("\n[3] All log levels:\n");
  failures += (test_log_levels() != 0);

  printf("\n[4] Dump output:\n");
  failures += (test_dump() != 0);

  printf("\n[5] Init/deinit cycle:\n");
  failures += (test_init_deinit() != 0);

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
