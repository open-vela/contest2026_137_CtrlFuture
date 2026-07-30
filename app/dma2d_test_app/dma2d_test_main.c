/****************************************************************************
 * packages/demos/contest2026_137_dma2d_test_app/dma2d_test_main.c
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

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Implemented by the STM32N6 DMA2D driver (arch layer).  Declared here
 * because the arch source headers are not on the application include path.
 */

extern int stm32n6_dma2d_probe(void);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 *
 * Description:
 *   DMA2D de-risk probe (ADR-032).  Runs a register-to-memory fill through
 *   the DMA2D accelerator after granting it the SRAM region's trusted
 *   compartment ID via RIFSC, then verifies the write landed.  PASSes iff
 *   DMA2D can write SRAM in DEV boot (proving the RISAF grant works, unlike
 *   GPDMA1 which cannot be granted).  Detailed RIMC/RISAF state is logged by
 *   the arch helper so one run fully characterises the firewall.
 *
 ****************************************************************************/

int main(int argc, char *argv[])
{
  int ret = stm32n6_dma2d_probe();

  if (ret == 0)
    {
      printf("DMA2D PROBE PASS (register-to-memory write landed in SRAM)\n");
      return 0;
    }

  printf("DMA2D PROBE FAIL (ret=%d, see dma2d: log lines above)\n", ret);
  return 1;
}
