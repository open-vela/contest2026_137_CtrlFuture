/****************************************************************************
 * app/edgesight/test_alert_msg.c
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
 * EdgeSight - Unit test for MQTT alert message formatter.
 * Verifies JSON output format and edge cases.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "alert_msg.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_fall_alert_format(void)
{
  char buf[ALERT_MSG_MAX];
  struct alert_payload_s payload;
  int len;

  memset(&payload, 0, sizeof(payload));
  payload.type = ALERT_TYPE_FALL;
  payload.confidence = 0.87f;
  payload.torso_angle = 62.3f;
  payload.cog_height = 0.32f;
  payload.frame_number = 12345;
  payload.timestamp_ms = 1234567;
  payload.persons_in_scene = 1;
  payload.uptime_s = 3600;

  len = alert_msg_format(buf, sizeof(buf), "edgesight-001",
                          &payload);

  if (len <= 0)
    {
      printf("  FAIL: format returned %d\n", len);
      return -1;
    }

  /* Verify key fields are present */

  if (strstr(buf, "\"device\":\"edgesight-001\"") == NULL)
    {
      printf("  FAIL: missing device field\n");
      return -1;
    }

  if (strstr(buf, "\"type\":\"fall\"") == NULL)
    {
      printf("  FAIL: missing type field\n");
      return -1;
    }

  if (strstr(buf, "\"confidence\"") == NULL)
    {
      printf("  FAIL: missing confidence field\n");
      return -1;
    }

  printf("  PASS: fall alert (%d bytes)\n%s\n", len, buf);
  return 0;
}

static int test_recovery_alert_format(void)
{
  char buf[ALERT_MSG_MAX];
  struct alert_payload_s payload;
  int len;

  memset(&payload, 0, sizeof(payload));
  payload.type = ALERT_TYPE_RECOVERY;
  payload.confidence = 0.95f;
  payload.frame_number = 12400;
  payload.timestamp_ms = 1240000;
  payload.uptime_s = 3650;

  len = alert_msg_format(buf, sizeof(buf), "edgesight-001",
                          &payload);

  if (len <= 0)
    {
      printf("  FAIL: format returned %d\n", len);
      return -1;
    }

  if (strstr(buf, "\"type\":\"recovery\"") == NULL)
    {
      printf("  FAIL: missing type field\n");
      return -1;
    }

  printf("  PASS: recovery alert (%d bytes)\n%s\n", len, buf);
  return 0;
}

static int test_heartbeat_format(void)
{
  char buf[ALERT_MSG_MAX];
  int len;

  len = alert_msg_heartbeat(buf, sizeof(buf),
                             "edgesight-001", 7200,
                             50000, 3);

  if (len <= 0)
    {
      printf("  FAIL: heartbeat returned %d\n", len);
      return -1;
    }

  if (strstr(buf, "\"type\":\"heartbeat\"") == NULL)
    {
      printf("  FAIL: missing type field\n");
      return -1;
    }

  if (strstr(buf, "\"device\":\"edgesight-001\"") == NULL)
    {
      printf("  FAIL: missing device field\n");
      return -1;
    }

  printf("  PASS: heartbeat (%d bytes)\n%s\n", len, buf);
  return 0;
}

static int test_buffer_too_small(void)
{
  char buf[10];
  struct alert_payload_s payload;
  int len;

  memset(&payload, 0, sizeof(payload));
  payload.type = ALERT_TYPE_FALL;
  payload.confidence = 0.87f;

  len = alert_msg_format(buf, sizeof(buf), "test", &payload);

  if (len != -1)
    {
      printf("  FAIL: expected -1, got %d\n", len);
      return -1;
    }

  printf("  PASS: small buffer returns -1\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_alert_msg_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("============================\n");
  printf(" EdgeSight Alert Msg Test\n");
  printf("============================\n\n");

  printf("[1] Fall alert format:\n");
  failures += (test_fall_alert_format() != 0);

  printf("\n[2] Recovery alert format:\n");
  failures += (test_recovery_alert_format() != 0);

  printf("\n[3] Heartbeat format:\n");
  failures += (test_heartbeat_format() != 0);

  printf("\n[4] Buffer too small:\n");
  failures += (test_buffer_too_small() != 0);

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
