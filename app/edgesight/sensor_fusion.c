/****************************************************************************
 * app/edgesight/sensor_fusion.c
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
 * EdgeSight - Multi-modal sensor fusion implementation.
 *
 * Decision rules:
 *   - Vision fall + any secondary sensor -> confirmed fall
 *   - PIR motion + no vision detection -> possible intrusion
 *   - Smoke + high temperature -> fire alert
 *   - Audio anomaly + PIR -> sound alert with motion
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "sensor_fusion.h"
#include <string.h>
#include <stdio.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static float combine_confidence(const struct fusion_input_s *a,
                                 const struct fusion_input_s *b)
{
  /* Bayesian-inspired combination:
   * P(A and B) = P(A) * P(B) when independent
   * Boost by 20% for multi-modal confirmation
   */

  float combined = a->confidence * b->confidence;

  combined = combined * 1.2f;
  if (combined > 1.0f)
    {
      combined = 1.0f;
    }

  return combined;
}

static int check_fall_alert(struct fusion_context_s *ctx,
                             struct fusion_decision_s *decision)
{
  const struct fusion_input_s *vision =
    &ctx->inputs[FUSION_INPUT_VISION];

  if (!vision->valid || vision->confidence < 0.5f)
    {
      return 0;
    }

  /* Vision detected fall with high confidence */

  if (vision->confidence >= 0.7f)
    {
      decision->alert_type = FUSION_ALERT_FALL;
      decision->confidence = vision->confidence;
      decision->sources = (1 << FUSION_INPUT_VISION);
      snprintf(decision->description,
               sizeof(decision->description),
               "Fall detected (vision only, conf=%.2f)",
               (double)vision->confidence);
      return 1;
    }

  /* Vision fall with moderate confidence + secondary sensor */

  const struct fusion_input_s *pir =
    &ctx->inputs[FUSION_INPUT_PIR];
  const struct fusion_input_s *audio =
    &ctx->inputs[FUSION_INPUT_AUDIO];

  if (pir->valid && pir->confidence > 0.3f)
    {
      decision->alert_type = FUSION_ALERT_FALL;
      decision->confidence =
        combine_confidence(vision, pir);
      decision->sources =
        (1 << FUSION_INPUT_VISION) |
        (1 << FUSION_INPUT_PIR);
      snprintf(decision->description,
               sizeof(decision->description),
               "Fall confirmed (vision+PIR, conf=%.2f)",
               (double)decision->confidence);
      return 1;
    }

  if (audio->valid && audio->confidence > 0.4f)
    {
      decision->alert_type = FUSION_ALERT_FALL;
      decision->confidence =
        combine_confidence(vision, audio);
      decision->sources =
        (1 << FUSION_INPUT_VISION) |
        (1 << FUSION_INPUT_AUDIO);
      snprintf(decision->description,
               sizeof(decision->description),
               "Fall confirmed (vision+audio, conf=%.2f)",
               (double)decision->confidence);
      return 1;
    }

  return 0;
}

static int check_fire_alert(struct fusion_context_s *ctx,
                             struct fusion_decision_s *decision)
{
  const struct fusion_input_s *smoke =
    &ctx->inputs[FUSION_INPUT_SMOKE];
  const struct fusion_input_s *temp =
    &ctx->inputs[FUSION_INPUT_TEMP];

  if (!smoke->valid || !temp->valid)
    {
      return 0;
    }

  if (smoke->confidence > 0.6f && temp->value > 50.0f)
    {
      decision->alert_type = FUSION_ALERT_FIRE;
      decision->confidence =
        combine_confidence(smoke, temp);
      decision->sources =
        (1 << FUSION_INPUT_SMOKE) |
        (1 << FUSION_INPUT_TEMP);
      snprintf(decision->description,
               sizeof(decision->description),
               "Fire detected (smoke+temp, conf=%.2f)",
               (double)decision->confidence);
      return 1;
    }

  return 0;
}

static int check_intrusion_alert(
    struct fusion_context_s *ctx,
    struct fusion_decision_s *decision)
{
  const struct fusion_input_s *pir =
    &ctx->inputs[FUSION_INPUT_PIR];
  const struct fusion_input_s *vision =
    &ctx->inputs[FUSION_INPUT_VISION];

  /* PIR motion but no vision person detected = intrusion */

  if (pir->valid && pir->confidence > 0.5f &&
      (!vision->valid || vision->confidence < 0.3f))
    {
      decision->alert_type = FUSION_ALERT_INTRUSION;
      decision->confidence = pir->confidence;
      decision->sources = (1 << FUSION_INPUT_PIR);
      snprintf(decision->description,
               sizeof(decision->description),
               "Intrusion detected (PIR only, conf=%.2f)",
               (double)pir->confidence);
      return 1;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void fusion_init(struct fusion_context_s *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

void fusion_update_input(struct fusion_context_s *ctx,
                         const struct fusion_input_s *input)
{
  if (input->type >= FUSION_INPUT_COUNT)
    {
      return;
    }

  ctx->inputs[input->type] = *input;
}

bool fusion_decide(struct fusion_context_s *ctx,
                   struct fusion_decision_s *decision)
{
  memset(decision, 0, sizeof(*decision));

  /* Check alerts in priority order */

  if (check_fall_alert(ctx, decision))
    {
      ctx->decision_count++;
      ctx->last_alert_type = decision->alert_type;
      return true;
    }

  if (check_fire_alert(ctx, decision))
    {
      ctx->decision_count++;
      ctx->last_alert_type = decision->alert_type;
      return true;
    }

  if (check_intrusion_alert(ctx, decision))
    {
      ctx->decision_count++;
      ctx->last_alert_type = decision->alert_type;
      return true;
    }

  return false;
}

const char *fusion_alert_str(uint8_t alert_type)
{
  switch (alert_type)
    {
      case FUSION_ALERT_NONE:
        return "none";
      case FUSION_ALERT_FALL:
        return "fall";
      case FUSION_ALERT_INTRUSION:
        return "intrusion";
      case FUSION_ALERT_FIRE:
        return "fire";
      case FUSION_ALERT_SOUND:
        return "sound";
      default:
        return "unknown";
    }
}
