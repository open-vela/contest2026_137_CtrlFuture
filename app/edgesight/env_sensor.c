/****************************************************************************
 * app/edgesight/env_sensor.c
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
 * EdgeSight - Environmental sensor implementation.
 *
 * Hardware:
 *   - SHT30: Temperature + Humidity (I2C4, addr 0x44)
 *   - MQ-2:  Smoke sensor (ADC, analog output)
 *   - AM312: PIR motion sensor (GPIO, digital output)
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "env_sensor.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <sys/ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SHT30_I2C_ADDR    0x44
#define SHT30_CMD_MEASURE 0x2400

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t get_ms(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static int read_sht30(int fd, float *temp, float *humi)
{
  uint8_t cmd[2];
  uint8_t data[6];
  int ret;

  /* Send measurement command */

  cmd[0] = (SHT30_CMD_MEASURE >> 8) & 0xff;
  cmd[1] = SHT30_CMD_MEASURE & 0xff;

  /* TODO: Use NuttX I2C interface
   * struct i2c_msg_s msg;
   * msg.addr = SHT30_I2C_ADDR;
   * msg.flags = 0;
   * msg.buffer = cmd;
   * msg.length = 2;
   * ret = ioctl(fd, I2C_TRANSFER, &msg);
   */

  (void)cmd;
  (void)fd;

  /* TODO: Read 6 bytes (temp_msb, temp_lsb, temp_crc,
   *       humi_msb, humi_lsb, humi_crc)
   * ret = read(fd, data, 6);
   */

  /* Placeholder: simulate values */

  *temp = 25.0f;
  *humi = 50.0f;

  UNUSED(data);
  UNUSED(ret);

  return 0;
}

static float read_smoke_adc(int fd)
{
  /* TODO: Read ADC value and normalize to 0-1
   * int adc_val;
   * ret = read(fd, &adc_val, sizeof(adc_val));
   * return (float)adc_val / 4095.0f;
   */

  (void)fd;
  return 0.0f;
}

static bool read_pir_gpio(int pin)
{
  /* TODO: Read GPIO pin state
   * int value;
   * ret = ioctl(pin, GPIOC_READ, &value);
   * return value != 0;
   */

  (void)pin;
  return false;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int env_sensor_init(struct env_sensor_context_s *ctx)
{
  memset(ctx, 0, sizeof(*ctx));

  /* TODO: Open I2C4 for SHT30
   * ctx->i2c_fd = open("/dev/i2c4", O_RDWR);
   */

  /* TODO: Open ADC for smoke sensor
   * ctx->adc_fd = open("/dev/adc0", O_RDONLY);
   */

  /* TODO: Configure PIR GPIO pin
   * ctx->pir_gpio = open("/dev/gpio0", O_RDONLY);
   */

  ctx->i2c_fd = -1;
  ctx->adc_fd = -1;
  ctx->pir_gpio = -1;
  ctx->initialized = true;

  syslog(LOG_INFO, "env_sensor: initialized\n");
  return 0;
}

int env_sensor_read(struct env_sensor_context_s *ctx,
                    struct env_sensor_data_s *data)
{
  int ret;

  if (!ctx->initialized)
    {
      return -1;
    }

  memset(data, 0, sizeof(*data));
  data->timestamp_ms = get_ms();

  /* Read SHT30 temperature + humidity */

  ret = read_sht30(ctx->i2c_fd,
                   &data->temperature,
                   &data->humidity);
  if (ret == 0)
    {
      data->valid[ENV_SENSOR_TEMP] = true;
      data->valid[ENV_SENSOR_HUMIDITY] = true;
    }

  /* Read smoke sensor */

  data->smoke_level = read_smoke_adc(ctx->adc_fd);
  data->valid[ENV_SENSOR_SMOKE] =
    (ctx->adc_fd >= 0);

  /* Read PIR motion */

  data->pir_motion = read_pir_gpio(ctx->pir_gpio);
  data->valid[ENV_SENSOR_PIR] =
    (ctx->pir_gpio >= 0);

  /* Cache last reading */

  ctx->last_data = *data;
  return 0;
}

const struct env_sensor_data_s *env_sensor_get_last(
    const struct env_sensor_context_s *ctx)
{
  return &ctx->last_data;
}

void env_sensor_deinit(struct env_sensor_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  if (ctx->i2c_fd >= 0)
    {
      close(ctx->i2c_fd);
    }

  if (ctx->adc_fd >= 0)
    {
      close(ctx->adc_fd);
    }

  if (ctx->pir_gpio >= 0)
    {
      close(ctx->pir_gpio);
    }

  memset(ctx, 0, sizeof(*ctx));
  syslog(LOG_INFO, "env_sensor: deinitialized\n");
}
