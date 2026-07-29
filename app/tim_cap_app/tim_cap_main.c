/****************************************************************************
 * packages/demos/contest2026_137_tim_cap_app/tim_cap_main.c
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
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <nuttx/timers/pwm.h>
#include <nuttx/timers/capture.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TIM_CAP_FREQ     1000      /* 1 kHz PWM test waveform */
#define TIM_CAP_DUTY     0x8000    /* 50% as a ub16 fraction of 65536 */
#define TIM_CAP_FREQ_LO  900       /* Accept 1 kHz +/-10% on the readback */
#define TIM_CAP_FREQ_HI  1100

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 *
 * Description:
 *   TIM PWM + input-capture loopback self-test (ADR-027).  Drives a 1 kHz
 *   50% PWM on TIM3 CH1 (/dev/pwm1) and reads its frequency and duty back
 *   through TIM15 input capture (/dev/cap0).  TIM15 TI1 is sourced from
 *   TIM3 CH1 internally via TISEL, so the loopback is fully on-chip and
 *   uses no external pin.  PASS iff the captured frequency is within
 *   tolerance of the driven 1 kHz and the edge counter advanced.
 *
 ****************************************************************************/

int main(int argc, char *argv[])
{
  struct pwm_info_s info;
  int pwmfd;
  int capfd;
  uint32_t freq = 0;
  uint32_t edges = 0;
  uint8_t duty = 0;
  int ret;

  pwmfd = open("/dev/pwm1", O_RDONLY);
  if (pwmfd < 0)
    {
      printf("TIM CAP LOOPBACK FAIL (open /dev/pwm1 errno=%d)\n", errno);
      return 1;
    }

  /* Drive a 1 kHz / 50% waveform on TIM3 CH1 first, so the inter-timer
   * trigger is already pulsing before the counter window opens.
   */

  info.frequency = TIM_CAP_FREQ;
  info.duty      = TIM_CAP_DUTY;

  ret = ioctl(pwmfd, PWMIOC_SETCHARACTERISTICS, (unsigned long)&info);
  if (ret < 0)
    {
      printf("TIM CAP LOOPBACK FAIL (PWM setchar errno=%d)\n", errno);
      close(pwmfd);
      return 1;
    }

  ret = ioctl(pwmfd, PWMIOC_START, 0);
  if (ret < 0)
    {
      printf("TIM CAP LOOPBACK FAIL (PWM start errno=%d)\n", errno);
      close(pwmfd);
      return 1;
    }

  /* Now open /dev/cap0.  cap_open() starts the counter and records t0, so
   * the counted pulses and the elapsed window begin together, cleanly
   * bracketing only the running PWM.
   */

  capfd = open("/dev/cap0", O_RDONLY);
  if (capfd < 0)
    {
      printf("TIM CAP LOOPBACK FAIL (open /dev/cap0 errno=%d)\n", errno);
      ioctl(pwmfd, PWMIOC_STOP, 0);
      close(pwmfd);
      return 1;
    }

  /* Count over a fixed window long enough for a stable pulse total. */

  usleep(200 * 1000);

  ret = ioctl(capfd, CAPIOC_FREQUENCE, (unsigned long)&freq);
  if (ret < 0)
    {
      printf("TIM CAP LOOPBACK FAIL (cap freq errno=%d)\n", errno);
      goto errout;
    }

  ioctl(capfd, CAPIOC_DUTYCYCLE, (unsigned long)&duty);
  ioctl(capfd, CAPIOC_EDGES, (unsigned long)&edges);

  ioctl(pwmfd, PWMIOC_STOP, 0);
  close(capfd);
  close(pwmfd);

  if (freq >= TIM_CAP_FREQ_LO && freq <= TIM_CAP_FREQ_HI && edges > 0)
    {
      printf("TIM CAP LOOPBACK PASS (freq=%lu duty=%u edges=%lu)\n",
             (unsigned long)freq, (unsigned)duty, (unsigned long)edges);
      return 0;
    }

  printf("TIM CAP LOOPBACK FAIL (freq=%lu duty=%u edges=%lu)\n",
         (unsigned long)freq, (unsigned)duty, (unsigned long)edges);
  return 1;

errout:
  ioctl(pwmfd, PWMIOC_STOP, 0);
  close(capfd);
  close(pwmfd);
  return 1;
}
