/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_timerisr.c
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

#include <stdint.h>

#include <nuttx/arch.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "nvic.h"
#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SysTick runs at processor clock.  Before PLL is configured,
 * the CPU runs directly from HSI at 64 MHz.
 */

#define STM32N6_SYSTICK_CLOCK  STM32_HSI_FREQUENCY
#define SYSTICK_RELOAD \
  ((STM32N6_SYSTICK_CLOCK / CLK_TCK) - 1)

#if SYSTICK_RELOAD > 0x00ffffff
#  error SYSTICK_RELOAD exceeds the range of the RELOAD register
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int stm32n6_timerisr(int irq, uint32_t *regs,
                            void *arg)
{
  UNUSED(irq);
  UNUSED(regs);
  UNUSED(arg);

  nxsched_process_timer();
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_timer_initialize
 *
 * Description:
 *   Initialize the SysTick timer for system tick generation.
 *
 ****************************************************************************/

void up_timer_initialize(void)
{
  uint32_t regval;

  putreg32(SYSTICK_RELOAD, NVIC_SYSTICK_RELOAD);
  putreg32(0, NVIC_SYSTICK_CURRENT);

  irq_attach(STM32_IRQ_SYSTICK,
             (xcpt_t)stm32n6_timerisr, NULL);

  regval = NVIC_SYSTICK_CTRL_CLKSOURCE |
           NVIC_SYSTICK_CTRL_TICKINT |
           NVIC_SYSTICK_CTRL_ENABLE;
  putreg32(regval, NVIC_SYSTICK_CTRL);

  up_enable_irq(STM32_IRQ_SYSTICK);
}
