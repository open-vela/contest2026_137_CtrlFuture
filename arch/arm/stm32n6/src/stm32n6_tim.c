/****************************************************************************
 * arch/arm/src/stm32n6/stm32n6_tim.c
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

/* ADR-027: General-purpose timer (TIM2) lower-half driver.
 *
 * This delivers the timing / periodic-interrupt function of ADR-027 via
 * the NuttX timer character framework (/dev/timerN).  TIM2 is a 32-bit
 * general-purpose timer on APB1; its 32-bit auto-reload avoids counter
 * cascading and gives a very large maximum timeout at a 1 MHz tick.
 *
 * The timer is driven at a fixed 1 MHz tick (1 us resolution) by
 * programming PSC from the APB1 timer input clock, so timeout values map
 * directly to microseconds regardless of the running core clock.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/timers/timer.h>
#include <nuttx/spinlock.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "chip.h"
#include "stm32n6_tim.h"
#include "hardware/stm32_tim.h"
#include "hardware/stm32_rcc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Fixed 1 MHz tick: 1 microsecond per counter increment */

#define STM32N6_TIM_TICK_FREQ  1000000

/* TIM2 is a 32-bit counter, so the maximum timeout at a 1 us tick is the
 * full 32-bit range expressed in microseconds.
 */

#define STM32N6_TIM_MAXTIMEOUT 0xffffffffull

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32n6_tim_lowerhalf_s
{
  const struct timer_ops_s *ops;      /* Lower-half operations (must be 1st) */
  uint32_t                  base;     /* Timer register base address */
  uint32_t                  timclk;   /* Timer input clock (Hz) */
  int                       irq;      /* Timer update IRQ number */
  tccb_t                    callback; /* Upper-half timeout callback */
  void                     *arg;      /* Argument for the callback */
  uint32_t                  timeout;  /* Current timeout (microseconds) */
  bool                      started;  /* True when the timer is running */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int  stm32n6_tim_start(struct timer_lowerhalf_s *lower);
static int  stm32n6_tim_stop(struct timer_lowerhalf_s *lower);
static int  stm32n6_tim_getstatus(struct timer_lowerhalf_s *lower,
                                   struct timer_status_s *status);
static int  stm32n6_tim_settimeout(struct timer_lowerhalf_s *lower,
                                    uint32_t timeout);
static void stm32n6_tim_setcallback(struct timer_lowerhalf_s *lower,
                                     tccb_t callback, void *arg);
static int  stm32n6_tim_maxtimeout(struct timer_lowerhalf_s *lower,
                                    uint32_t *maxtimeout);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct timer_ops_s g_stm32n6_tim_ops =
{
  .start      = stm32n6_tim_start,
  .stop       = stm32n6_tim_stop,
  .getstatus  = stm32n6_tim_getstatus,
  .settimeout = stm32n6_tim_settimeout,
  .setcallback = stm32n6_tim_setcallback,
  .maxtimeout = stm32n6_tim_maxtimeout,
};

#ifdef CONFIG_STM32_TIM2
static struct stm32n6_tim_lowerhalf_s g_tim2_lowerhalf =
{
  .ops    = &g_stm32n6_tim_ops,
  .base   = STM32_TIM2_BASE,
  .timclk = STM32_APB1_TIM_FREQUENCY,
  .irq    = STM32_IRQ_TIM2,
};
#endif

static spinlock_t g_tim_lock = SP_UNLOCKED;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_tim_enableclk
 *
 * Description:
 *   Enable the APB1 peripheral clock feeding this timer.  Without this a
 *   register write is silently dropped and reads return zero.
 *
 ****************************************************************************/

static void stm32n6_tim_enableclk(struct stm32n6_tim_lowerhalf_s *priv)
{
  irqstate_t flags;
  uint32_t regval;

  flags = spin_lock_irqsave(&g_tim_lock);

#ifdef CONFIG_STM32_TIM2
  if (priv->base == STM32_TIM2_BASE)
    {
      regval  = getreg32(STM32_RCC_APB1ENR1);
      regval |= RCC_APB1ENR1_TIM2EN;
      putreg32(regval, STM32_RCC_APB1ENR1);

      /* Also keep the clock alive across CPU Sleep (WFI); otherwise the
       * counter freezes while a task blocks and never fires the update
       * interrupt that is supposed to wake it.
       */

      regval  = getreg32(STM32_RCC_APB1LPENR1);
      regval |= RCC_APB1LPENR1_TIM2LPEN;
      putreg32(regval, STM32_RCC_APB1LPENR1);
    }
#endif

  spin_unlock_irqrestore(&g_tim_lock, flags);
}

/****************************************************************************
 * Name: stm32n6_tim_interrupt
 *
 * Description:
 *   Common timer update interrupt handler.  Acknowledges the update flag
 *   and invokes the registered upper-half callback.  A periodic callback
 *   may adjust the next interval; a NULL or non-reloading callback stops
 *   the timer.
 *
 ****************************************************************************/

static int stm32n6_tim_interrupt(int irq, void *context, void *arg)
{
  struct stm32n6_tim_lowerhalf_s *priv =
    (struct stm32n6_tim_lowerhalf_s *)arg;
  uint32_t sr;

  DEBUGASSERT(priv != NULL);

  sr = getreg32(priv->base + STM32_TIM_SR_OFFSET);
  if ((sr & TIM_SR_UIF) == 0)
    {
      return OK;
    }

  /* Acknowledge the update interrupt (rc_w0: write 0 to clear) */

  putreg32(~TIM_SR_UIF, priv->base + STM32_TIM_SR_OFFSET);

  if (priv->callback != NULL)
    {
      uint32_t next = priv->timeout;

      if (priv->callback(&next, priv->arg))
        {
          /* Reload: honour a possibly-updated interval */

          if (next != priv->timeout)
            {
              stm32n6_tim_settimeout(
                (struct timer_lowerhalf_s *)priv, next);
            }
        }
      else
        {
          stm32n6_tim_stop((struct timer_lowerhalf_s *)priv);
        }
    }
  else
    {
      stm32n6_tim_stop((struct timer_lowerhalf_s *)priv);
    }

  return OK;
}

/****************************************************************************
 * Name: stm32n6_tim_start
 ****************************************************************************/

static int stm32n6_tim_start(struct timer_lowerhalf_s *lower)
{
  struct stm32n6_tim_lowerhalf_s *priv =
    (struct stm32n6_tim_lowerhalf_s *)lower;
  irqstate_t flags;
  uint32_t psc;

  DEBUGASSERT(priv != NULL);

  if (priv->started)
    {
      return -EBUSY;
    }

  /* Feed the timer its peripheral clock before touching any register */

  stm32n6_tim_enableclk(priv);

  flags = spin_lock_irqsave(&g_tim_lock);

  /* Stop the counter while (re)configuring */

  putreg32(0, priv->base + STM32_TIM_CR1_OFFSET);

  /* Program the prescaler for a 1 MHz tick (1 us resolution) */

  psc = (priv->timclk / STM32N6_TIM_TICK_FREQ) - 1;
  putreg32(psc, priv->base + STM32_TIM_PSC_OFFSET);

  /* Program the auto-reload authoritatively here.  settimeout() may have
   * run while the peripheral clock was still gated (its ARR write silently
   * dropped), so re-apply it now that the clock is guaranteed live.  An
   * ARR of 0 would freeze the counter and never raise an update event.
   */

  if (priv->timeout != 0)
    {
      putreg32(priv->timeout - 1, priv->base + STM32_TIM_ARR_OFFSET);
    }

  /* Only counter overflow generates an update interrupt (URS), and the
   * auto-reload value is preloaded (ARPE).
   */

  putreg32(TIM_CR1_URS | TIM_CR1_ARPE,
           priv->base + STM32_TIM_CR1_OFFSET);

  /* Force an update event to load PSC/ARR.  UG sets UIF (and, with CNT and
   * the compare registers both zero, the capture/compare flags) regardless
   * of URS, so clear the entire status register afterwards.  Enabling UIE
   * with a stale UIF still latched would fire the ISR immediately and, if
   * the flag is not fully drained, storm.
   */

  putreg32(TIM_EGR_UG, priv->base + STM32_TIM_EGR_OFFSET);
  putreg32(0, priv->base + STM32_TIM_SR_OFFSET);

  /* Enable the update interrupt and start counting */

  putreg32(TIM_DIER_UIE, priv->base + STM32_TIM_DIER_OFFSET);
  modifyreg32(priv->base + STM32_TIM_CR1_OFFSET, 0, TIM_CR1_CEN);

  priv->started = true;

  spin_unlock_irqrestore(&g_tim_lock, flags);

  up_enable_irq(priv->irq);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_tim_stop
 ****************************************************************************/

static int stm32n6_tim_stop(struct timer_lowerhalf_s *lower)
{
  struct stm32n6_tim_lowerhalf_s *priv =
    (struct stm32n6_tim_lowerhalf_s *)lower;
  irqstate_t flags;

  DEBUGASSERT(priv != NULL);

  if (!priv->started)
    {
      return -EINVAL;
    }

  flags = spin_lock_irqsave(&g_tim_lock);

  /* Disable the update interrupt, stop the counter, clear pending flag */

  putreg32(0, priv->base + STM32_TIM_DIER_OFFSET);
  putreg32(0, priv->base + STM32_TIM_CR1_OFFSET);
  putreg32(~TIM_SR_UIF, priv->base + STM32_TIM_SR_OFFSET);

  priv->started = false;

  spin_unlock_irqrestore(&g_tim_lock, flags);

  up_disable_irq(priv->irq);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_tim_getstatus
 ****************************************************************************/

static int stm32n6_tim_getstatus(struct timer_lowerhalf_s *lower,
                                  struct timer_status_s *status)
{
  struct stm32n6_tim_lowerhalf_s *priv =
    (struct stm32n6_tim_lowerhalf_s *)lower;
  uint32_t arr;
  uint32_t cnt;

  DEBUGASSERT(priv != NULL && status != NULL);

  status->flags = 0;
  if (priv->started)
    {
      status->flags |= TCFLAGS_ACTIVE;
    }

  if (priv->callback != NULL)
    {
      status->flags |= TCFLAGS_HANDLER;
    }

  status->timeout = priv->timeout;

  /* Time remaining until expiry, in microseconds (1 tick == 1 us) */

  arr = getreg32(priv->base + STM32_TIM_ARR_OFFSET);
  cnt = getreg32(priv->base + STM32_TIM_CNT_OFFSET);
  status->timeleft = (arr >= cnt) ? (arr - cnt) : 0;

  return OK;
}

/****************************************************************************
 * Name: stm32n6_tim_settimeout
 ****************************************************************************/

static int stm32n6_tim_settimeout(struct timer_lowerhalf_s *lower,
                                   uint32_t timeout)
{
  struct stm32n6_tim_lowerhalf_s *priv =
    (struct stm32n6_tim_lowerhalf_s *)lower;
  irqstate_t flags;

  DEBUGASSERT(priv != NULL);

  if (timeout == 0 || timeout > STM32N6_TIM_MAXTIMEOUT)
    {
      return -EINVAL;
    }

  flags = spin_lock_irqsave(&g_tim_lock);

  priv->timeout = timeout;

  /* 1 tick == 1 us, so the reload value is (timeout - 1) counts */

  putreg32(timeout - 1, priv->base + STM32_TIM_ARR_OFFSET);

  spin_unlock_irqrestore(&g_tim_lock, flags);

  return OK;
}

/****************************************************************************
 * Name: stm32n6_tim_setcallback
 ****************************************************************************/

static void stm32n6_tim_setcallback(struct timer_lowerhalf_s *lower,
                                     tccb_t callback, void *arg)
{
  struct stm32n6_tim_lowerhalf_s *priv =
    (struct stm32n6_tim_lowerhalf_s *)lower;
  irqstate_t flags;

  DEBUGASSERT(priv != NULL);

  flags = spin_lock_irqsave(&g_tim_lock);
  priv->callback = callback;
  priv->arg      = arg;
  spin_unlock_irqrestore(&g_tim_lock, flags);
}

/****************************************************************************
 * Name: stm32n6_tim_maxtimeout
 ****************************************************************************/

static int stm32n6_tim_maxtimeout(struct timer_lowerhalf_s *lower,
                                   uint32_t *maxtimeout)
{
  DEBUGASSERT(maxtimeout != NULL);

  *maxtimeout = (uint32_t)STM32N6_TIM_MAXTIMEOUT;
  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32n6_timer_initialize
 *
 * Description:
 *   Bind a general-purpose timer to a character device and register it.
 *
 * Input Parameters:
 *   devpath - Character device path (e.g. "/dev/timer0").
 *   timer   - Timer peripheral number (currently only 2 is supported).
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int stm32n6_timer_initialize(const char *devpath, int timer)
{
  struct stm32n6_tim_lowerhalf_s *priv;
  void *handle;

  DEBUGASSERT(devpath != NULL);

  switch (timer)
    {
#ifdef CONFIG_STM32_TIM2
      case 2:
        priv = &g_tim2_lowerhalf;
        break;
#endif

      default:
        return -ENODEV;
    }

  /* Attach the update ISR (kept disabled at the NVIC until start) */

  irq_attach(priv->irq, stm32n6_tim_interrupt, priv);

  handle = timer_register(devpath, (struct timer_lowerhalf_s *)priv);
  if (handle == NULL)
    {
      irq_detach(priv->irq);
      return -EEXIST;
    }

  return OK;
}
