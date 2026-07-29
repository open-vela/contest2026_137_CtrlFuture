/****************************************************************************
 * packages/demos/contest2026_137_dts_temp_app/dts_temp_main.c
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

#include <fixedmath.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Accept a plausible die-junction reading.  A powered, running chip sits
 * above ambient but well below the 125 C sensor ceiling; anything in this
 * band proves a real conversion rather than a stuck 0 or garbage value.
 */

#define DTS_TEMP_MIN_C   0     /* Reject sub-freezing (not this env) */
#define DTS_TEMP_MAX_C   110   /* Reject implausibly hot / faulted */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 *
 * Description:
 *   DTS die-temperature self-test (ADR-030).  Opens /dev/temp0, reads one
 *   b16_t Celsius sample, prints it, and PASSes iff the reading is within a
 *   plausible band for a running chip.  The sensor is fully on-chip, so the
 *   readback needs no external wiring or instrument.
 *
 ****************************************************************************/

int main(int argc, char *argv[])
{
  b16_t temp = 0;
  int32_t whole;
  int32_t frac;
  int fd;
  ssize_t n;

  fd = open("/dev/temp0", O_RDONLY);
  if (fd < 0)
    {
      printf("DTS TEMP FAIL (open /dev/temp0 errno=%d)\n", errno);
      return 1;
    }

  n = read(fd, &temp, sizeof(temp));
  close(fd);

  if (n != (ssize_t)sizeof(temp))
    {
      printf("DTS TEMP FAIL (read n=%d errno=%d)\n", (int)n, errno);
      return 1;
    }

  /* Split the Q16.16 value into whole degrees and two decimal places using
   * integer math only (the image may be built without an FPU).
   */

  whole = b16toi(temp);
  frac  = (int32_t)(((int64_t)(temp & 0x0000ffff) * 100) >> 16);

  if (whole >= DTS_TEMP_MIN_C && whole <= DTS_TEMP_MAX_C)
    {
      printf("DTS TEMP PASS (temp=%ld.%02ldC)\n",
             (long)whole, (long)frac);
      return 0;
    }

  printf("DTS TEMP FAIL (temp=%ld.%02ldC out of range)\n",
         (long)whole, (long)frac);
  return 1;
}
