/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_allocateheap.c
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
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/kmalloc.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SRAM layout: code and data are loaded at the beginning of SRAM.
 * The heap starts immediately after the idle thread stack.
 *
 * SRAM origin is defined by the linker script (0x34000400 for
 * DEV boot mode).  The linker script places _ebss at the end of
 * BSS; g_idle_topstack is _ebss + CONFIG_IDLETHREAD_STACKSIZE.
 *
 * We use the linker-defined _eheap (end of SRAM) as the upper
 * bound if available; otherwise fall back to origin + LENGTH.
 */

#define STM32N6_SRAM_BASE  0x34000400ul
#define STM32N6_SRAM_LEN   4193280ul
#define STM32N6_SRAM_END \
  (STM32N6_SRAM_BASE + STM32N6_SRAM_LEN)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_allocate_heap
 *
 * Description:
 *   This function is called by the OS during initialization
 *   to configure the heap.  The heap starts at the top of the
 *   idle stack and extends to the end of SRAM.
 *
 ****************************************************************************/

void up_allocate_heap(void **heap_start, size_t *heap_size)
{
  *heap_start = (void *)g_idle_topstack;
  *heap_size  = STM32N6_SRAM_END - g_idle_topstack;

  board_autoled_on(LED_HEAPALLOC);
}

/****************************************************************************
 * Name: arm_addregion
 *
 * Description:
 *   Memory manager initialization at up_initialize() time.
 *   Add any additional memory regions here.
 *
 ****************************************************************************/

#if CONFIG_MM_REGIONS > 1
void arm_addregion(void)
{
}
#endif
