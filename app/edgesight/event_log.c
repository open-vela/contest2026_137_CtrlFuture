/****************************************************************************
 * app/edgesight/event_log.c
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
 * EdgeSight - Event logging system implementation.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "event_log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t get_timestamp_ms(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static const char *level_str(enum event_level_e level)
{
  switch (level)
    {
      case EVENT_LEVEL_DEBUG:
        return "DBG";
      case EVENT_LEVEL_INFO:
        return "INF";
      case EVENT_LEVEL_WARN:
        return "WRN";
      case EVENT_LEVEL_ALERT:
        return "ALT";
      case EVENT_LEVEL_ERROR:
        return "ERR";
      default:
        return "???";
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int event_log_init(struct event_log_s *log,
                   const char *sd_path)
{
  memset(log, 0, sizeof(*log));

  if (sd_path != NULL)
    {
      strncpy(log->log_path, sd_path,
              EVENT_LOG_PATH_MAX - 1);
      log->log_path[EVENT_LOG_PATH_MAX - 1] = '\0';
      log->sd_available = true;
    }

  return 0;
}

int event_log_write(struct event_log_s *log,
                    enum event_level_e level,
                    uint32_t frame,
                    const char *fmt, ...)
{
  struct event_entry_s *entry;
  va_list ap;

  /* Check if ring buffer is full */

  if (log->count >= EVENT_LOG_RING_SIZE)
    {
      log->dropped++;
      return -1;
    }

  /* Get next write slot */

  entry = &log->ring[log->head];
  entry->timestamp_ms = get_timestamp_ms();
  entry->frame_number = frame;
  entry->level = (uint8_t)level;

  /* Format message */

  va_start(ap, fmt);
  vsnprintf(entry->message, EVENT_LOG_MSG_MAX, fmt, ap);
  va_end(ap);

  /* Advance head */

  log->head = (log->head + 1) % EVENT_LOG_RING_SIZE;
  log->count++;
  log->total_logged++;

  return 0;
}

int event_log_flush(struct event_log_s *log)
{
  int flushed = 0;
  struct event_entry_s *entry;
  char filepath[EVENT_LOG_PATH_MAX + 32];
  FILE *fp;

  if (!log->sd_available || log->count == 0)
    {
      return 0;
    }

  /* Open log file in append mode */

  snprintf(filepath, sizeof(filepath),
           "%s/edgesight.log", log->log_path);

  fp = fopen(filepath, "a");
  if (fp == NULL)
    {
      return -1;
    }

  /* Write all pending entries */

  while (log->count > 0)
    {
      entry = &log->ring[log->tail];

      fprintf(fp,
              "[%010lu] F%06lu [%s] %s\n",
              (unsigned long)entry->timestamp_ms,
              (unsigned long)entry->frame_number,
              level_str((enum event_level_e)entry->level),
              entry->message);

      log->tail = (log->tail + 1) % EVENT_LOG_RING_SIZE;
      log->count--;
      flushed++;
    }

  fclose(fp);
  log->total_flushed += flushed;

  return flushed;
}

void event_log_stats(const struct event_log_s *log,
                     uint32_t *total_logged,
                     uint32_t *total_flushed,
                     uint16_t *dropped)
{
  if (total_logged != NULL)
    {
      *total_logged = log->total_logged;
    }

  if (total_flushed != NULL)
    {
      *total_flushed = log->total_flushed;
    }

  if (dropped != NULL)
    {
      *dropped = log->dropped;
    }
}

void event_log_dump(const struct event_log_s *log,
                    uint16_t max_entries)
{
  uint16_t idx;
  uint16_t count;
  uint16_t i;
  const struct event_entry_s *entry;

  count = log->count;
  if (max_entries > 0 && max_entries < count)
    {
      count = max_entries;
    }

  if (count == 0)
    {
      printf("[event_log] No entries in ring buffer\n");
      return;
    }

  /* Start from oldest unread entry */

  idx = log->tail;
  if (max_entries > 0 && max_entries < log->count)
    {
      /* Skip to show only the most recent N */

      uint16_t skip = log->count - max_entries;
      idx = (log->tail + skip) % EVENT_LOG_RING_SIZE;
    }

  printf("[event_log] Recent events (%u/%u, %u dropped):\n",
         count, log->count, log->dropped);

  for (i = 0; i < count; i++)
    {
      entry = &log->ring[idx];
      printf("  [%010lu] F%06lu [%s] %s\n",
             (unsigned long)entry->timestamp_ms,
             (unsigned long)entry->frame_number,
             level_str((enum event_level_e)entry->level),
             entry->message);
      idx = (idx + 1) % EVENT_LOG_RING_SIZE;
    }
}
