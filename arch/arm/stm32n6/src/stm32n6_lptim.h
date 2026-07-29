/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_lptim.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_STM32N6_LPTIM_H
#define __ARCH_ARM_SRC_STM32N6_STM32N6_LPTIM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Name: stm32n6_lptim_initialize
 *
 * Description:
 *   Bind a low-power timer to a character device and register it with the
 *   NuttX timer framework at the given device path.  The timer runs from
 *   the LSI clock as a plain periodic timer; the low-power Stop-mode wake
 *   and PWM-output sub-functions of LPTIM are not exposed here.
 *
 * Input Parameters:
 *   devpath - Character device path (e.g. "/dev/timer1").
 *   timer   - LPTIM peripheral number.  LPTIM1 lives on APB1; LPTIM2..5
 *             live on APB4.  Any of 1..5 is supported.
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int stm32n6_lptim_initialize(const char *devpath, int timer);

#ifdef CONFIG_STM32_LPTIM2_PWM

/****************************************************************************
 * Name: stm32n6_lppwm_initialize
 *
 * Description:
 *   Register LPTIM2 as a PWM character device (e.g. "/dev/pwm0"), driving
 *   the LPTIM2_CH1 output from the LSI clock.  PWM mode is mutually
 *   exclusive with the plain periodic-timer mode of the same instance.
 *
 * Input Parameters:
 *   devpath - Character device path (e.g. "/dev/pwm0").
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int stm32n6_lppwm_initialize(const char *devpath);

#endif /* CONFIG_STM32_LPTIM2_PWM */

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_STM32N6_STM32N6_LPTIM_H */
