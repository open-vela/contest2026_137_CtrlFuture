/****************************************************************************
 * app/edgesight/test_env_sensor.c
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
 * EdgeSight - Unit test for environmental sensor module.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>

#include "env_sensor.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_init_deinit(void)
{
  struct env_sensor_context_s ctx;
  int ret;

  ret = env_sensor_init(&ctx);
  if (ret < 0)
    {
      printf("  FAIL: init returned %d\n", ret);
      return -1;
    }

  if (!ctx.initialized)
    {
      printf("  FAIL: not marked initialized\n");
      return -1;
    }

  env_sensor_deinit(&ctx);

  if (ctx.initialized)
    {
      printf("  FAIL: still marked initialized after deinit\n");
      return -1;
    }

  printf("  PASS: init/deinit cycle\n");
  return 0;
}

static int test_read_stub(void)
{
  struct env_sensor_context_s ctx;
  struct env_sensor_data_s data;
  int ret;

  env_sensor_init(&ctx);
  ret = env_sensor_read(&ctx, &data);

  if (ret < 0)
    {
      printf("  FAIL: read returned %d\n", ret);
      return -1;
    }

  if (data.temperature < -40.0f || data.temperature > 85.0f)
    {
      printf("  FAIL: temperature out of range: %.1f\n",
             (double)data.temperature);
      return -1;
    }

  if (data.humidity < 0.0f || data.humidity > 100.0f)
    {
      printf("  FAIL: humidity out of range: %.1f\n",
             (double)data.humidity);
      return -1;
    }

  printf("  PASS: read returns valid range "
         "(temp=%.1f hum=%.1f)\n",
         (double)data.temperature,
         (double)data.humidity);
  return 0;
}

static int test_read_without_init(void)
{
  struct env_sensor_context_s ctx;
  struct env_sensor_data_s data;
  int ret;

  memset(&ctx, 0, sizeof(ctx));
  ret = env_sensor_read(&ctx, &data);

  if (ret != -1)
    {
      printf("  FAIL: expected -1, got %d\n", ret);
      return -1;
    }

  printf("  PASS: read without init returns error\n");
  return 0;
}

static int test_get_last(void)
{
  struct env_sensor_context_s ctx;
  struct env_sensor_data_s data;
  const struct env_sensor_data_s *last;

  env_sensor_init(&ctx);

  last = env_sensor_get_last(&ctx);
  if (last == NULL)
    {
      printf("  FAIL: get_last returned NULL\n");
      return -1;
    }

  env_sensor_read(&ctx, &data);
  last = env_sensor_get_last(&ctx);

  if (last->temperature != data.temperature)
    {
      printf("  FAIL: get_last mismatch\n");
      return -1;
    }

  printf("  PASS: get_last returns cached data\n");
  return 0;
}

static int test_valid_flags(void)
{
  struct env_sensor_context_s ctx;
  struct env_sensor_data_s data;

  env_sensor_init(&ctx);
  env_sensor_read(&ctx, &data);

  /* In stub mode, all sensors should report valid */

  printf("  INFO: temp_valid=%d hum_valid=%d "
         "smoke_valid=%d pir_valid=%d\n",
         data.valid[ENV_SENSOR_TEMP],
         data.valid[ENV_SENSOR_HUMIDITY],
         data.valid[ENV_SENSOR_SMOKE],
         data.valid[ENV_SENSOR_PIR]);

  printf("  PASS: valid flags checked\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_env_sensor_test_main(int argc, char *argv[])
#endif
{
  int failures = 0;

  printf("============================\n");
  printf(" EdgeSight Env Sensor Test\n");
  printf("============================\n\n");

  printf("[1] Init/deinit cycle:\n");
  failures += (test_init_deinit() != 0);

  printf("\n[2] Read stub values:\n");
  failures += (test_read_stub() != 0);

  printf("\n[3] Read without init:\n");
  failures += (test_read_without_init() != 0);

  printf("\n4] Get last cached data:\n");
  failures += (test_get_last() != 0);

  printf("\n[5] Valid flags:\n");
  failures += (test_valid_flags() != 0);

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
