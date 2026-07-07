/****************************************************************************
 * vendor/openvela/boards/contest2026_137_board/include/board.h
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

#ifndef __BOARDS_CONTEST2026_137_BOARD_INCLUDE_BOARD_H
#define __BOARDS_CONTEST2026_137_BOARD_INCLUDE_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* STM32N647 Target Hardware Reference
 *
 *   Chip            : STM32N647X0 (Arm Cortex-M55, ARMv8.1-M)
 *   Max CPU Clock   : 800 MHz (600 MHz normal, 800 MHz overdrive)
 *   SRAM            : 4.2 MB contiguous
 *   DTCM            : 128 KB with ECC
 *   ITCM            : 64 KB with ECC
 *   Backup SRAM     : 8 KB (VBAT domain)
 *   Internal Flash  : None (boots from external XSPI flash)
 *   NPU             : ST Neural-ART @ 1 GHz, 600 Gops
 *
 * Internal Oscillators:
 *   HSI             : 64 MHz
 *   MSI             : 4 MHz
 *   LSI             : 32 kHz
 *
 * External Oscillators:
 *   HSE             : 16-48 MHz
 *   LSE             : 32.768 kHz
 *
 * PLLs:
 *   PLL1 (system)   : up to 800 MHz CPU clock
 *   PLL2 (NPU)      : up to 1 GHz Neural-ART clock
 *   PLL3, PLL4      : kernel clocks (peripherals)
 *
 * Note: Currently running on MPS3-AN547 (Cortex-M55) QEMU reference
 *       platform while STM32N6 chip drivers are under development.
 */

/* MPS3-AN547 SysTick clock for QEMU emulation */

#define MPS_SYSTICK_CLOCK   (32 * 1000 * 1000)

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */

#endif /* __BOARDS_CONTEST2026_137_BOARD_INCLUDE_BOARD_H */
