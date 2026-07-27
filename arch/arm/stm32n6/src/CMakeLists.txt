# ##############################################################################
# arch/arm/src/stm32n6/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with this
# work for additional information regarding copyright ownership.  The ASF
# licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
# ##############################################################################

set(SRCS
    stm32n6_start.c
    stm32n6_rcc.c
    stm32n6_lowputc.c
    stm32n6_serial.c
    stm32n6_irq.c
    stm32n6_gpio.c
    stm32n6_timerisr.c
    stm32n6_pwr.c
    stm32n6_idle.c)

if(CONFIG_VIDEO)
  list(APPEND SRCS stm32n6_dcmipp.c)
endif()

if(CONFIG_VIDEO_FB)
  list(APPEND SRCS stm32n6_ltdc.c)
endif()

if(CONFIG_STM32_RNG)
  list(APPEND SRCS stm32n6_rng.c)
endif()

if(CONFIG_STM32_RTC)
  list(APPEND SRCS stm32n6_rtc.c)
  if(CONFIG_RTC_DRIVER)
    list(APPEND SRCS stm32n6_rtc_lowerhalf.c)
  endif()
endif()

if(CONFIG_DEV_GPIO)
  list(APPEND SRCS stm32n6_gpio_lower.c)
endif()

if(CONFIG_STM32_TIM2)
  list(APPEND SRCS stm32n6_tim.c)
endif()

if(CONFIG_STM32_TIM5)
  list(APPEND SRCS stm32n6_oneshot.c)
endif()

if(CONFIG_STM32_ADC1)
  list(APPEND SRCS stm32n6_adc.c)
endif()

target_sources(arch PRIVATE ${SRCS})
