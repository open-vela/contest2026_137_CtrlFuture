/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_gpio.h
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

#ifndef __ARCH_ARM_SRC_STM32N6_STM32N6_GPIO_H
#define __ARCH_ARM_SRC_STM32N6_STM32N6_GPIO_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* GPIO pin encoding:
 *
 *   3322 2222 2222 1111 1111 11
 *   1098 7654 3210 9876 5432 1098 7654 3210
 *   ---- ---- ---- ---- ---- ---- ---- ----
 *   MMOO PPPP SSAA AAFF FFTT TTBB BBBB BBBB
 *
 * M = mode (2 bits: input/output/af/analog)
 * O = output type (1 bit: push-pull/open-drain) + speed (1 bit)
 * P = pull-up/down (2 bits) + reserved (2 bits)
 * S = speed (2 bits)
 * A = alternate function (4 bits)
 * F = reserved flags (4 bits)
 * T = port (4 bits: 0=A, 1=B, ...)
 * B = pin (4 bits: 0-15)
 *
 * Simplified encoding for skeleton:
 */

/* Mode field */

#define GPIO_MODE_SHIFT      30
#define GPIO_MODE_MASK       (3ul << GPIO_MODE_SHIFT)
#define GPIO_MODE_INPUT      (0ul << GPIO_MODE_SHIFT)
#define GPIO_MODE_OUTPUT     (1ul << GPIO_MODE_SHIFT)
#define GPIO_MODE_AF         (2ul << GPIO_MODE_SHIFT)
#define GPIO_MODE_ANALOG     (3ul << GPIO_MODE_SHIFT)

/* Output type */

#define GPIO_OTYPE_SHIFT     28
#define GPIO_OTYPE_PP        (0ul << GPIO_OTYPE_SHIFT)
#define GPIO_OTYPE_OD        (1ul << GPIO_OTYPE_SHIFT)

/* Speed */

#define GPIO_SPEED_SHIFT     24
#define GPIO_SPEED_LOW       (0ul << GPIO_SPEED_SHIFT)
#define GPIO_SPEED_HIGH      (3ul << GPIO_SPEED_SHIFT)

/* Pull-up/down */

#define GPIO_PUPD_SHIFT      22
#define GPIO_PUPD_NONE       (0ul << GPIO_PUPD_SHIFT)
#define GPIO_PUPD_PU         (1ul << GPIO_PUPD_SHIFT)
#define GPIO_PUPD_PD         (2ul << GPIO_PUPD_SHIFT)

/* Alternate function */

#define GPIO_AF_SHIFT        16
#define GPIO_AF_MASK         (0xful << GPIO_AF_SHIFT)
#define GPIO_AF(n)           ((uint32_t)(n) << GPIO_AF_SHIFT)

/* Port */

#define GPIO_PORT_SHIFT      4
#define GPIO_PORT_MASK       (0xful << GPIO_PORT_SHIFT)
#define GPIO_PORTA           (0ul << GPIO_PORT_SHIFT)
#define GPIO_PORTB           (1ul << GPIO_PORT_SHIFT)
#define GPIO_PORTC           (2ul << GPIO_PORT_SHIFT)
#define GPIO_PORTD           (3ul << GPIO_PORT_SHIFT)
#define GPIO_PORTE           (4ul << GPIO_PORT_SHIFT)
#define GPIO_PORTF           (5ul << GPIO_PORT_SHIFT)
#define GPIO_PORTG           (6ul << GPIO_PORT_SHIFT)
#define GPIO_PORTH           (7ul << GPIO_PORT_SHIFT)

/* Pin */

#define GPIO_PIN_SHIFT       0
#define GPIO_PIN_MASK        (0xful << GPIO_PIN_SHIFT)
#define GPIO_PIN(n)          ((uint32_t)(n) << GPIO_PIN_SHIFT)

/* USART1 pins: PE5=TX AF7, PE6=RX AF7 */

#define GPIO_USART1_TX \
  (GPIO_MODE_AF | GPIO_OTYPE_PP | GPIO_SPEED_HIGH | \
   GPIO_PUPD_PU | GPIO_AF(7) | GPIO_PORTE | GPIO_PIN(5))

#define GPIO_USART1_RX \
  (GPIO_MODE_AF | GPIO_OTYPE_PP | GPIO_SPEED_HIGH | \
   GPIO_PUPD_PU | GPIO_AF(7) | GPIO_PORTE | GPIO_PIN(6))

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int stm32n6_configgpio(uint32_t cfgset);

#endif /* __ARCH_ARM_SRC_STM32N6_STM32N6_GPIO_H */
