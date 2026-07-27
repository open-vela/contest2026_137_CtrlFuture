/****************************************************************************
 * vendor/openvela/boards/contest2026_137_board/src/board_bringup.c
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

#include <nuttx/config.h>

#include <sys/types.h>
#include <sys/mount.h>
#include <syslog.h>

#include <nuttx/board.h>
#include <nuttx/fs/fs.h>

#ifdef CONFIG_RTC_DRIVER
#  include <nuttx/timers/rtc.h>
#endif

#ifdef CONFIG_ARCH_CHIP_STM32N6
#  include "arm_internal.h"
#  include "stm32n6_dcmipp.h"
#  include "stm32n6_ltdc.h"
#  include "stm32n6_rtc.h"
#  ifdef CONFIG_DEV_GPIO
#    include "stm32n6_gpio.h"
#  endif
#  ifdef CONFIG_STM32_TIM2
#    include "stm32n6_tim.h"
#  endif
#  ifdef CONFIG_STM32_TIM5
#    include "stm32n6_oneshot.h"
#  endif
#  ifdef CONFIG_STM32_LPTIM1
#    include "stm32n6_lptim.h"
#  endif
#  ifdef CONFIG_STM32_ADC1
#    include "stm32n6_adc.h"
#  endif
#endif

#include <arch/board/board.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_bringup (STM32N6) / board_bringup (generic)
 *
 * Description:
 *   Perform architecture-specific initialization — mount filesystems, etc.
 *
 ****************************************************************************/

static int board_bringup(void)
{
  int ret = OK;

#ifdef CONFIG_FS_PROCFS
  ret = nx_mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR,
             "ERROR: Failed to mount procfs at /proc: %d\n", ret);
    }
#endif

#ifdef CONFIG_FS_TMPFS
  ret = nx_mount(NULL, CONFIG_LIBC_TMPDIR, "tmpfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR,
             "ERROR: Failed to mount tmpfs at %s: %d\n",
             CONFIG_LIBC_TMPDIR, ret);
    }
#endif

#ifdef CONFIG_ARCH_CHIP_STM32N6
#  ifdef CONFIG_STM32_RTC
  /* Initialize the RTC peripheral (clock, prescaler, calendar) */

  ret = stm32n6_rtc_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: RTC init failed: %d\n", ret);
    }
#    ifdef CONFIG_RTC_DRIVER
  else
    {
      /* Register the RTC upper-half character driver at /dev/rtc0 */

      ret = rtc_initialize(0, stm32n6_rtc_lowerhalf());
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: rtc_initialize failed: %d\n", ret);
        }
    }
#    endif
#  endif

#  ifdef CONFIG_DEV_GPIO
  /* Register GPIO character devices for the cmocka drivertest_gpio suite.
   * Pins follow the ALIENTEK STM32N647 board (ST SoftwarePackage BSP):
   *   /dev/gpio1 = LED0 (PG10) push-pull output  -> write/readback tests
   *   /dev/gpio2 = KEY0 (PC6)  EXTI input         -> interrupt test
   */

  ret = stm32n6_gpio_lower_initialize(GPIO_PORTG | GPIO_PIN(10), 1,
                                      GPIO_OUTPUT_PIN);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: gpio1 (PG10) init failed: %d\n", ret);
    }

  ret = stm32n6_gpio_lower_initialize(GPIO_PORTC | GPIO_PIN(6), 2,
                                      GPIO_INTERRUPT_PIN);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: gpio2 (PC6) init failed: %d\n", ret);
    }
#  endif

#  ifdef CONFIG_STM32_TIM2
  /* Register TIM2 as the timer character device /dev/timer0 for the
   * cmocka drivertest_timer suite (periodic 1 us-resolution timeouts).
   */

  ret = stm32n6_timer_initialize("/dev/timer0", 2);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: TIM2 init failed: %d\n", ret);
    }
#  endif

#  ifdef CONFIG_STM32_TIM5
  /* Register TIM5 as the one-shot timer /dev/oneshot for the cmocka
   * drivertest_oneshot suite.
   */

    {
      struct oneshot_lowerhalf_s *os = stm32n6_oneshot_initialize(5);
      if (os == NULL)
        {
          syslog(LOG_ERR, "ERROR: TIM5 oneshot init failed\n");
        }
      else
        {
          ret = oneshot_register("/dev/oneshot", os);
          if (ret < 0)
            {
              syslog(LOG_ERR, "ERROR: oneshot_register failed: %d\n", ret);
            }
        }
    }
#  endif

#  ifdef CONFIG_STM32_LPTIM1
  /* Register LPTIM1 as the timer character device /dev/timer1 for the
   * cmocka drivertest_timer suite (LSI-clocked periodic timeouts).
   */

  ret = stm32n6_lptim_initialize("/dev/timer1", 1);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: LPTIM1 init failed: %d\n", ret);
    }
#  endif

#  ifdef CONFIG_STM32_ADC1
  /* Register ADC1 as /dev/adc0 for the cmocka drivertest_adc suite.  It
   * samples the chip-internal VREFINT source (no external wiring).
   */

  ret = stm32n6_adc_initialize("/dev/adc0", 1);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: ADC1 init failed: %d\n", ret);
    }
#  endif

#  ifdef CONFIG_VIDEO
  /* Initialize DCMIPP camera (800x480 @ 30fps) */

  ret = stm32n6_dcmipp_init(800, 480, 30);
  if (ret < 0)
    {
      syslog(LOG_ERR,
             "ERROR: DCMIPP init failed: %d\n", ret);
    }
#  endif

#  ifdef CONFIG_VIDEO_FB
  /* Initialize LTDC display (800x480, dual-layer)
   * Framebuffers allocated from board.h or linker script
   */

  ret = stm32n6_ltdc_init(800, 480,
                            (void *)BOARD_LCD_BG_ADDR,
                            (void *)BOARD_LCD_FG_ADDR0,
                            (void *)BOARD_LCD_FG_ADDR1);
  if (ret < 0)
    {
      syslog(LOG_ERR,
             "ERROR: LTDC init failed: %d\n", ret);
    }
#  endif
#endif

  return ret;
}

/****************************************************************************
 * Name: stm32_boardinitialize
 *
 * Description:
 *   STM32N6 arch layer calls this early in boot.  For QEMU/MPS3 builds
 *   this symbol is not referenced by the arch layer.
 *
 ****************************************************************************/

#ifdef CONFIG_ARCH_CHIP_STM32N6
void stm32_boardinitialize(void)
{
}
#endif

/****************************************************************************
 * Name: board_late_initialize
 *
 * Description:
 *   Called after up_initialize(), before the initial app starts.
 *
 ****************************************************************************/

#ifdef CONFIG_BOARD_LATE_INITIALIZE
void board_late_initialize(void)
{
  board_bringup();
}
#endif

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description:
 *   Perform application specific initialization.
 *
 ****************************************************************************/

int board_app_initialize(uintptr_t arg)
{
  UNUSED(arg);

#ifndef CONFIG_BOARD_LATE_INITIALIZE
  return board_bringup();
#else
  return OK;
#endif
}
