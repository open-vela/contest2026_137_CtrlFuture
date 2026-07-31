/****************************************************************************
 * packages/demos/contest2026_137_ap3216c_test_app/ap3216c_test_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <nuttx/i2c/i2c_master.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* AP3216C ambient-light / proximity / IR sensor on I2C4 at 7-bit address
 * 0x1E (ALIENTEK STM32N647 board wiring; PE13=SCL, PE14=SDA, AF4).  It is a
 * register-mapped device: the system-configuration register 0x00 holds the
 * mode.  Writing 0x04 issues a software reset; writing 0x03 enables
 * ALS+PS+IR and reads back as 0x03.  That write/read-back handshake proves
 * the I2C4 bus transacts data in both directions with a real external slave
 * -- no jumper or instrument, and no reliance on ambient light for a value.
 */

#define AP3216C_I2C_BUS    "/dev/i2c4"
#define AP3216C_I2C_ADDR   0x1e
#define AP3216C_FREQUENCY  400000
#define AP3216C_REG_SYSCFG 0x00
#define AP3216C_MODE_RESET 0x04
#define AP3216C_MODE_ALL   0x03  /* ALS + PS + IR active */
#define AP3216C_RESET_US   20000 /* datasheet reset time >= 10 ms */

/* Data registers 0x0A..0x0F (IR/ALS/PS), read back only to show a live
 * sample alongside the mode handshake.
 */

#define AP3216C_REG_DATA   0x0a
#define AP3216C_DATA_LEN   6

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ap3216c_write_reg
 *
 * Description:
 *   Write one 8-bit value to an AP3216C register: a single 2-byte write
 *   message [regaddr, value].  Returns 0 on ACK, a negative errno on NACK
 *   or bus error.
 *
 ****************************************************************************/

static int ap3216c_write_reg(int fd, uint8_t reg, uint8_t val)
{
  struct i2c_transfer_s xfer;
  struct i2c_msg_s msg;
  uint8_t buf[2];

  buf[0]        = reg;
  buf[1]        = val;
  msg.frequency = AP3216C_FREQUENCY;
  msg.addr      = AP3216C_I2C_ADDR;
  msg.flags     = 0;
  msg.buffer    = buf;
  msg.length    = 2;
  xfer.msgv     = &msg;
  xfer.msgc     = 1;

  if (ioctl(fd, I2CIOC_TRANSFER, (unsigned long)(uintptr_t)&xfer) < 0)
    {
      return -errno;
    }

  return 0;
}

/****************************************************************************
 * Name: ap3216c_read_regs
 *
 * Description:
 *   Read len bytes starting at reg using a repeated-start transaction: a
 *   1-byte write of the register address (no STOP) followed by a read.
 *   Returns 0 on success, a negative errno otherwise.
 *
 ****************************************************************************/

static int ap3216c_read_regs(int fd, uint8_t reg, uint8_t *buf, int len)
{
  struct i2c_transfer_s xfer;
  struct i2c_msg_s msg[2];

  msg[0].frequency = AP3216C_FREQUENCY;
  msg[0].addr      = AP3216C_I2C_ADDR;
  msg[0].flags     = 0;
  msg[0].buffer    = &reg;
  msg[0].length    = 1;

  msg[1].frequency = AP3216C_FREQUENCY;
  msg[1].addr      = AP3216C_I2C_ADDR;
  msg[1].flags     = I2C_M_READ;
  msg[1].buffer    = buf;
  msg[1].length    = len;

  xfer.msgv = msg;
  xfer.msgc = 2;

  if (ioctl(fd, I2CIOC_TRANSFER, (unsigned long)(uintptr_t)&xfer) < 0)
    {
      return -errno;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 *
 * Description:
 *   AP3216C I2C4 self-test (ADR-014).  Opens /dev/i2c4, software-resets the
 *   sensor, enables all channels by writing 0x03 to the system-config
 *   register, and reads it back.  PASSes iff every transfer ACKs and the
 *   register reads back 0x03 -- proof that the I2C4 bus transacts data both
 *   ways against a real onboard slave (no jumper or instrument needed).  It
 *   also prints one live IR/ALS/PS sample for visibility.
 *
 ****************************************************************************/

int main(int argc, char *argv[])
{
  uint8_t data[AP3216C_DATA_LEN];
  uint8_t mode;
  uint16_t ir;
  uint16_t als;
  uint16_t ps;
  int fd;
  int ret;

  fd = open(AP3216C_I2C_BUS, O_RDWR);
  if (fd < 0)
    {
      printf("AP3216C FAIL (open %s errno=%d)\n", AP3216C_I2C_BUS, errno);
      return 1;
    }

  /* Software reset, then let the device settle. */

  ret = ap3216c_write_reg(fd, AP3216C_REG_SYSCFG, AP3216C_MODE_RESET);
  if (ret < 0)
    {
      printf("AP3216C FAIL (reset NACK ret=%d -- no slave at 0x%02x?)\n",
             ret, AP3216C_I2C_ADDR);
      close(fd);
      return 1;
    }

  usleep(AP3216C_RESET_US);

  /* Enable ALS + PS + IR, then read the mode register back. */

  ret = ap3216c_write_reg(fd, AP3216C_REG_SYSCFG, AP3216C_MODE_ALL);
  if (ret < 0)
    {
      printf("AP3216C FAIL (mode write ret=%d)\n", ret);
      close(fd);
      return 1;
    }

  ret = ap3216c_read_regs(fd, AP3216C_REG_SYSCFG, &mode, 1);
  if (ret < 0)
    {
      printf("AP3216C FAIL (mode read ret=%d)\n", ret);
      close(fd);
      return 1;
    }

  if (mode != AP3216C_MODE_ALL)
    {
      printf("AP3216C FAIL (mode readback 0x%02x != 0x%02x)\n",
             mode, AP3216C_MODE_ALL);
      close(fd);
      return 1;
    }

  /* Read one live IR/ALS/PS sample (informational). */

  ret = ap3216c_read_regs(fd, AP3216C_REG_DATA, data, AP3216C_DATA_LEN);
  close(fd);

  if (ret < 0)
    {
      printf("AP3216C FAIL (data read ret=%d)\n", ret);
      return 1;
    }

  ir  = (data[0] & 0x80) ? 0 :
        (((uint16_t)data[1] << 2) | (data[0] & 0x03));
  als = ((uint16_t)data[3] << 8) | data[2];
  ps  = (data[4] & 0x40) ? 0 :
        (((uint16_t)(data[5] & 0x3f) << 4) | (data[4] & 0x0f));

  printf("AP3216C PASS (mode=0x%02x ir=%u als=%u ps=%u)\n",
         mode, ir, als, ps);
  return 0;
}
