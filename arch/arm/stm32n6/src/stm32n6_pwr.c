/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_pwr.c
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

#include <stdbool.h>
#include <stdint.h>

#include "arm_internal.h"
#include "stm32n6_pwr.h"
#include "hardware/stm32_pwr.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PWR_VOS_TIMEOUT  10000

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_pwr_set_voltage_scale
 *
 * Description:
 *   Set the voltage scaling level.
 *   Scale 0 = highest performance (800MHz CPU).
 *   Scale 1 = high performance.
 *   Must wait for VOSRDY before switching PLL to high frequency.
 *
 ****************************************************************************/

int stm32n6_pwr_set_voltage_scale(unsigned int scale)
{
  uint32_t regval;
  int timeout;

  if (scale > PWR_VOS_SCALE1)
    {
      return -1;
    }

  regval  = getreg32(STM32_PWR_VOSCR);
  regval &= ~PWR_VOSCR_VOS;
  regval |= (scale & 0x1);
  putreg32(regval, STM32_PWR_VOSCR);

  /* Wait for VOSRDY */

  timeout = PWR_VOS_TIMEOUT;
  while ((getreg32(STM32_PWR_VOSCR) & PWR_VOSCR_VOSRDY) == 0)
    {
      if (--timeout <= 0)
        {
          return -1;
        }
    }

  return 0;
}

/****************************************************************************
 * Name: stm32n6_pwr_get_voltage_scale
 *
 * Description:
 *   Get the currently applied voltage scaling level.
 *
 ****************************************************************************/

unsigned int stm32n6_pwr_get_voltage_scale(void)
{
  uint32_t regval;

  regval = getreg32(STM32_PWR_VOSCR);
  return (regval & PWR_VOSCR_ACTVOS) >> 16;
}

/****************************************************************************
 * Name: stm32n6_pwr_enablebkp
 *
 * Description:
 *   Enables or disables write access to the backup domain (RTC
 *   registers, RTC backup data registers, and backup SRAM).  The
 *   backup domain defaults to write-protected on reset; RTC
 *   configuration writes are silently ignored unless this is
 *   called first with writable=true.  Ported from apache/nuttx
 *   upstream stm32_pwr_enablebkp().
 *
 * Input Parameters:
 *   writable - true: enable write access to backup domain registers
 *              false: restore write protection
 *
 * Returned Value:
 *   true if the backup domain was already writable before this call.
 *
 ****************************************************************************/

bool stm32n6_pwr_enablebkp(bool writable)
{
  uint32_t regval;
  bool waswritable;

  regval = getreg32(STM32_PWR_DBPCR);
  waswritable = ((regval & PWR_DBPCR_DBP) != 0);

  if (writable)
    {
      regval |= PWR_DBPCR_DBP;
    }
  else
    {
      regval &= ~PWR_DBPCR_DBP;
    }

  putreg32(regval, STM32_PWR_DBPCR);

  return waswritable;
}

/****************************************************************************
 * Name: stm32n6_pwr_enablevddio
 *
 * Description:
 *   Mark a set of I/O voltage domains as supply-valid in PWR SVMCR3
 *   and optionally select their VRSEL (1.8V) range.  Ported from
 *   apache/nuttx upstream stm32_pwr_enablevddio().
 *
 * Input Parameters:
 *   mask - OR of PWR_SVMCR3_VDDIOxSV and PWR_SVMCR3_VDDIOxVRSEL bits.
 *
 ****************************************************************************/

void stm32n6_pwr_enablevddio(uint32_t mask)
{
  uint32_t regval;

  regval  = getreg32(STM32_PWR_SVMCR3);
  regval |= mask;
  putreg32(regval, STM32_PWR_SVMCR3);
}
