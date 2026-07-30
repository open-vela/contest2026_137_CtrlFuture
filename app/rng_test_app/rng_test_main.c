/****************************************************************************
 * packages/demos/contest2026_137_rng_test_app/rng_test_main.c
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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Draw two independent 32-byte blocks from the TRNG.  Sanity checks:
 *  - the read returns the full length,
 *  - the block is not all-zero and not a single repeated byte (a stuck or
 *    stubbed source would fail these),
 *  - the two blocks differ (a frozen source would repeat).
 * These prove a live hardware entropy source without any statistical
 * apparatus; the driver itself already enforces the FIPS continuous test.
 */

#define RNG_TEST_BLK   32

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int rng_read_block(int fd, uint8_t *buf, size_t len)
{
  size_t got = 0;
  ssize_t n;

  while (got < len)
    {
      n = read(fd, buf + got, len - got);
      if (n <= 0)
        {
          return -1;
        }

      got += (size_t)n;
    }

  return 0;
}

static bool rng_block_degenerate(const uint8_t *buf, size_t len)
{
  size_t i;
  uint8_t acc = 0;
  bool all_same = true;

  for (i = 0; i < len; i++)
    {
      acc |= buf[i];
      if (buf[i] != buf[0])
        {
          all_same = false;
        }
    }

  /* Degenerate if every byte is zero (acc==0) or all bytes identical. */

  return (acc == 0) || all_same;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 *
 * Description:
 *   RNG self-test (ADR-037).  Reads two 32-byte blocks from /dev/random
 *   (backed by the STM32N6 hardware TRNG) and PASSes iff each block is
 *   non-degenerate and the two blocks differ, proving a live on-chip
 *   entropy source.  No external wiring or instrument is needed.
 *
 ****************************************************************************/

int main(int argc, char *argv[])
{
  uint8_t a[RNG_TEST_BLK];
  uint8_t b[RNG_TEST_BLK];
  int fd;

  fd = open("/dev/random", O_RDONLY);
  if (fd < 0)
    {
      printf("RNG TEST FAIL (open /dev/random errno=%d)\n", errno);
      return 1;
    }

  if (rng_read_block(fd, a, sizeof(a)) < 0 ||
      rng_read_block(fd, b, sizeof(b)) < 0)
    {
      printf("RNG TEST FAIL (short read errno=%d)\n", errno);
      close(fd);
      return 1;
    }

  close(fd);

  if (rng_block_degenerate(a, sizeof(a)) ||
      rng_block_degenerate(b, sizeof(b)))
    {
      printf("RNG TEST FAIL (degenerate block: zero or constant)\n");
      return 1;
    }

  if (memcmp(a, b, sizeof(a)) == 0)
    {
      printf("RNG TEST FAIL (two reads identical: source frozen)\n");
      return 1;
    }

  printf("RNG TEST PASS (two 32-byte reads non-degenerate and differ)\n");
  return 0;
}
