/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_start.c
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
#include <nuttx/init.h>

#include <stdint.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "nvic.h"
#include "stm32n6_rcc.h"
#include "stm32n6_lowputc.h"

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void stm32_boardinitialize(void);

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define IDLE_STACK \
  ((uintptr_t)_ebss + CONFIG_IDLETHREAD_STACKSIZE)

/****************************************************************************
 * Public Data
 ****************************************************************************/

const uintptr_t g_idle_topstack = IDLE_STACK;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: showprogress
 *
 * Description:
 *   Print a character on the UART to show boot progress.
 *
 ****************************************************************************/

#ifdef CONFIG_DEBUG_FEATURES
static inline void showprogress(char c)
{
  arm_lowputc(c);
}
#else
#  define showprogress(c)
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: __start
 *
 * Description:
 *   This is the reset entry point.
 *
 ****************************************************************************/

void __start(void)
{
  const uint32_t *src;
  uint32_t *dest;

  /* Clear .bss */

  for (dest = (uint32_t *)_sbss; dest < (uint32_t *)_ebss; )
    {
      *dest++ = 0;
    }

  /* Copy .data from flash to SRAM */

  src = (const uint32_t *)_eronly;
  dest = (uint32_t *)_sdata;
  for (; dest < (uint32_t *)_edata; )
    {
      *dest++ = *src++;
    }

  /* Configure clocks */

  stm32n6_clockconfig();

  /* Configure the UART for early debug output */

  stm32n6_lowsetup();
  showprogress('A');

  /* Call board early initialization */

  stm32_boardinitialize();
  showprogress('B');

#ifdef USE_EARLYSERIALINIT
  arm_earlyserialinit();
#endif
  showprogress('C');

  /* Start NuttX */

  showprogress('\r');
  showprogress('\n');
  nx_start();

  for (; ; );
}
