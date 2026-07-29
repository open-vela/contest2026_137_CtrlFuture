/****************************************************************************
 * packages/demos/contest2026_137_lptim_stop_app/lptim_stop_main.c
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
#include <stdbool.h>
#include <stdlib.h>

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Implemented by the STM32N6 LPTIM driver (arch layer).  Declared here
 * because the arch source headers are not on the application include path.
 */

extern int stm32n6_lptim_stopwake(unsigned int ms, bool *stopf,
                                  unsigned int *elapsed);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 *
 * Description:
 *   LPTIM Stop-mode wakeup demo.  Arms LPTIM3 as a one-shot on the LSI,
 *   drops the CPU into Stop mode, and reports whether the LPTIM interrupt
 *   woke the core (PWR_CPUCR.STOPF confirms Stop was genuinely entered).
 *   Optional argument: Stop duration in milliseconds (default 2000).
 *
 ****************************************************************************/

int main(int argc, char *argv[])
{
  unsigned int ms = 2000;
  unsigned int elapsed = 0;
  bool stopf = false;
  int ret;

  if (argc > 1)
    {
      ms = (unsigned int)strtoul(argv[1], NULL, 0);
    }

  printf("LPTIM STOP WAKE: arming LPTIM3 for %u ms, entering Stop...\n", ms);

  ret = stm32n6_lptim_stopwake(ms, &stopf, &elapsed);
  if (ret == 0)
    {
      printf("LPTIM STOP WAKE PASS (stopf=%d elapsed=%us)\n",
             (int)stopf, elapsed);
      return 0;
    }

  printf("LPTIM STOP WAKE FAIL (ret=%d stopf=%d elapsed=%us)\n",
         ret, (int)stopf, elapsed);
  return 1;
}
