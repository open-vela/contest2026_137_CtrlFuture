/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_iwdg.c
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
 * STM32N6 IWDG (Independent Watchdog) driver for NuttX.
 * Provides hardware watchdog timer for system hang detection.
 *
 * Adapted from STM32H7 NuttX reference (stm32_iwdg.c).
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <syslog.h>
#include <string.h>

#include "stm32n6_iwdg.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* IWDG register base */

#define STM32N6_IWDG_BASE   0x46004800  /* CMSIS IWDG_BASE_NS */

/* IWDG register offsets */

#define IWDG_KR_OFFSET     0x00
#define IWDG_PR_OFFSET     0x04
#define IWDG_RLR_OFFSET    0x08
#define IWDG_SR_OFFSET     0x0C
#define IWDG_WINR_OFFSET   0x10

/* IWDG key values */

#define IWDG_KEY_RELOAD    0xAAAA
#define IWDG_KEY_ENABLE    0xCCCC
#define IWDG_KEY_ACCESS    0x5555

/* IWDG prescaler values */

#define IWDG_PR_DIV4       0
#define IWDG_PR_DIV8       1
#define IWDG_PR_DIV16      2
#define IWDG_PR_DIV32      3
#define IWDG_PR_DIV64      4
#define IWDG_PR_DIV128     5
#define IWDG_PR_DIV256     6

/* IWDG status bits */

#define IWDG_SR_PVU        (1 << 0)  /* Prescaler value update */
#define IWDG_SR_RVU        (1 << 1)  /* Reload value update */

/* IWDG clock: LSI = 32 kHz */

#define IWDG_LSI_FREQ      32000

/****************************************************************************
 * Private Data
 ****************************************************************************/

static bool g_iwdg_initialized = false;
static uint32_t g_iwdg_timeout_ms;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline void iwdg_putreg(uint32_t offset, uint32_t value)
{
  *(volatile uint32_t *)(STM32N6_IWDG_BASE + offset) = value;
}

static inline uint32_t iwdg_getreg(uint32_t offset)
{
  return *(volatile uint32_t *)(STM32N6_IWDG_BASE + offset);
}

static int iwdg_wait_prescaler(void)
{
  uint32_t timeout = 100000;

  while (timeout-- > 0)
    {
      if (!(iwdg_getreg(IWDG_SR_OFFSET) & IWDG_SR_PVU))
        {
          return 0;
        }
    }

  return -ETIMEDOUT;
}

static int iwdg_wait_reload(void)
{
  uint32_t timeout = 100000;

  while (timeout-- > 0)
    {
      if (!(iwdg_getreg(IWDG_SR_OFFSET) & IWDG_SR_RVU))
        {
          return 0;
        }
    }

  return -ETIMEDOUT;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32n6_iwdg_initialize(uint32_t timeout_ms)
{
  uint32_t pr;
  uint32_t rlr;
  uint32_t prescaler;
  uint32_t reload;
  int ret;

  if (g_iwdg_initialized)
    {
      return 0;
    }

  /* Calculate prescaler and reload for desired timeout.
   * timeout = (reload + 1) * prescaler / LSI_FREQ
   * Try to find best fit with minimal prescaler.
   *
   * DIV4:   tick = 0.125ms, max = 8.192s
   * DIV8:   tick = 0.25ms,  max = 16.384s
   * DIV16:  tick = 0.5ms,   max = 32.768s
   * DIV32:  tick = 1ms,     max = 65.536s
   * DIV64:  tick = 2ms,     max = 131.072s
   * DIV128: tick = 4ms,     max = 262.144s
   * DIV256: tick = 8ms,     max = 524.288s
   */

  prescaler = 32;  /* DIV32: 1ms tick */
  pr = IWDG_PR_DIV32;
  reload = (timeout_ms * IWDG_LSI_FREQ) / (prescaler * 1000);

  if (reload > 0xfff)
    {
      reload = 0xfff;
    }

  /* Unlock IWDG registers */

  iwdg_putreg(IWDG_KR_OFFSET, IWDG_KEY_ACCESS);

  /* Wait for prescaler update */

  ret = iwdg_wait_prescaler();
  if (ret < 0)
    {
      return ret;
    }

  /* Set prescaler */

  iwdg_putreg(IWDG_PR_OFFSET, pr);

  /* Wait for reload update */

  ret = iwdg_wait_reload();
  if (ret < 0)
    {
      return ret;
    }

  /* Set reload value */

  iwdg_putreg(IWDG_RLR_OFFSET, reload);

  /* Start watchdog */

  iwdg_putreg(IWDG_KR_OFFSET, IWDG_KEY_ENABLE);

  /* Initial kick */

  iwdg_putreg(IWDG_KR_OFFSET, IWDG_KEY_RELOAD);

  g_iwdg_initialized = true;
  g_iwdg_timeout_ms = timeout_ms;

  syslog(LOG_INFO, "iwdg: initialized, timeout=%lums\n",
         (unsigned long)timeout_ms);
  return 0;
}

void stm32n6_iwdg_feed(void)
{
  if (g_iwdg_initialized)
    {
      iwdg_putreg(IWDG_KR_OFFSET, IWDG_KEY_RELOAD);
    }
}

uint32_t stm32n6_iwdg_get_timeout(void)
{
  return g_iwdg_timeout_ms;
}
