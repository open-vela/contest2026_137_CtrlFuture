/****************************************************************************
 * packages/demos/contest2026_137_hash_test_app/hash_test_main.c
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
#include <string.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* NIST FIPS 180-4 test vector: SHA-256("abc"). */

#define HASH_TEST_MSG   "abc"

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Implemented by the STM32N6 HASH driver (arch layer).  Declared here
 * because the arch source headers are not on the application include path.
 */

extern int stm32n6_sha256(const uint8_t *input, size_t len,
                          uint8_t *digest);

static const uint8_t g_sha256_abc[32] =
{
  0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
  0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
  0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
  0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 *
 * Description:
 *   HASH SHA-256 self-test (ADR-037).  Computes SHA-256("abc") on the
 *   STM32N6 HASH accelerator and compares the digest against the NIST
 *   FIPS 180-4 known-answer vector.  The engine is fully on-chip, so a
 *   byte-exact match against the published vector proves a real hardware
 *   computation with no external wiring or instrument.
 *
 ****************************************************************************/

int main(int argc, char *argv[])
{
  uint8_t digest[32];
  int ret;
  int i;

  memset(digest, 0, sizeof(digest));

  ret = stm32n6_sha256((const uint8_t *)HASH_TEST_MSG,
                       strlen(HASH_TEST_MSG), digest);
  if (ret != 0)
    {
      printf("HASH SHA256 FAIL (compute ret=%d)\n", ret);
      return 1;
    }

  printf("HASH SHA256 digest:");
  for (i = 0; i < 32; i++)
    {
      printf("%02x", digest[i]);
    }

  printf("\n");

  if (memcmp(digest, g_sha256_abc, sizeof(digest)) == 0)
    {
      printf("HASH SHA256 PASS (SHA-256(\"abc\") matches NIST vector)\n");
      return 0;
    }

  printf("HASH SHA256 FAIL (digest mismatch)\n");
  return 1;
}
