/****************************************************************************
 * app/edgesight/test_sensor_fusion.c
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
 * EdgeSight - Unit test for sensor fusion engine.
 * Can run on host (QEMU) to verify logic without real hardware.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>

#include "sensor_fusion.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int test_no_inputs(void)
{
  struct fusion_context_s ctx;
  struct fusion_decision_s decision;

  fusion_init(&ctx);

  bool result = fusion_decide(&ctx, &decision);

  if (result)
    {
      printf("  FAIL: no inputs should not trigger\n");
      return -1;
    }

  printf("  PASS: no inputs -> no alert\n");
  return 0;
}

static int test_fall_vision_only(void)
{
  struct fusion_context_s ctx;
  struct fusion_decision_s decision;
  struct fusion_input_s input;

  fusion_init(&ctx);

  input.type = FUSION_INPUT_VISION;
  input.confidence = 0.8f;
  input.value = 0;
  input.valid = true;
  fusion_update_input(&ctx, &input);

  bool result = fusion_decide(&ctx, &decision);

  if (!result || decision.alert_type != FUSION_ALERT_FALL)
    {
      printf("  FAIL: high confidence vision should trigger fall\n");
      return -1;
    }

  printf("  PASS: vision fall (conf=0.8) -> FALL "
         "(combined conf=%.2f)\n",
         (double)decision.confidence);
  return 0;
}

static int test_fall_vision_low_confidence(void)
{
  struct fusion_context_s ctx;
  struct fusion_decision_s decision;
  struct fusion_input_s input;

  fusion_init(&ctx);

  /* Vision with low confidence, no secondary sensor */

  input.type = FUSION_INPUT_VISION;
  input.confidence = 0.4f;
  input.value = 0;
  input.valid = true;
  fusion_update_input(&ctx, &input);

  bool result = fusion_decide(&ctx, &decision);

  if (result)
    {
      printf("  FAIL: low confidence vision alone should not "
             "trigger\n");
      return -1;
    }

  printf("  PASS: vision fall (conf=0.4) alone -> no alert\n");
  return 0;
}

static int test_fall_vision_plus_pir(void)
{
  struct fusion_context_s ctx;
  struct fusion_decision_s decision;
  struct fusion_input_s vision;
  struct fusion_input_s pir;

  fusion_init(&ctx);

  vision.type = FUSION_INPUT_VISION;
  vision.confidence = 0.5f;
  vision.value = 0;
  vision.valid = true;
  fusion_update_input(&ctx, &vision);

  pir.type = FUSION_INPUT_PIR;
  pir.confidence = 0.6f;
  pir.value = 0;
  pir.valid = true;
  fusion_update_input(&ctx, &pir);

  bool result = fusion_decide(&ctx, &decision);

  if (!result || decision.alert_type != FUSION_ALERT_FALL)
    {
      printf("  FAIL: vision + PIR should confirm fall\n");
      return -1;
    }

  printf("  PASS: vision(0.5) + PIR(0.6) -> FALL "
         "(conf=%.2f, sources=0x%02x)\n",
         (double)decision.confidence, decision.sources);
  return 0;
}

static int test_fall_vision_plus_audio(void)
{
  struct fusion_context_s ctx;
  struct fusion_decision_s decision;
  struct fusion_input_s vision;
  struct fusion_input_s audio;

  fusion_init(&ctx);

  vision.type = FUSION_INPUT_VISION;
  vision.confidence = 0.55f;
  vision.value = 0;
  vision.valid = true;
  fusion_update_input(&ctx, &vision);

  audio.type = FUSION_INPUT_AUDIO;
  audio.confidence = 0.5f;
  audio.value = 0;
  audio.valid = true;
  fusion_update_input(&ctx, &audio);

  bool result = fusion_decide(&ctx, &decision);

  if (!result || decision.alert_type != FUSION_ALERT_FALL)
    {
      printf("  FAIL: vision + audio should confirm fall\n");
      return -1;
    }

  printf("  PASS: vision(0.55) + audio(0.5) -> FALL "
         "(conf=%.2f)\n",
         (double)decision.confidence);
  return 0;
}

static int test_fire_smoke_high_temp(void)
{
  struct fusion_context_s ctx;
  struct fusion_decision_s decision;
  struct fusion_input_s smoke;
  struct fusion_input_s temp;

  fusion_init(&ctx);

  smoke.type = FUSION_INPUT_SMOKE;
  smoke.confidence = 0.7f;
  smoke.value = 0.8f;
  smoke.valid = true;
  fusion_update_input(&ctx, &smoke);

  temp.type = FUSION_INPUT_TEMP;
  temp.confidence = 0.9f;
  temp.value = 60.0f;
  temp.valid = true;
  fusion_update_input(&ctx, &temp);

  bool result = fusion_decide(&ctx, &decision);

  if (!result || decision.alert_type != FUSION_ALERT_FIRE)
    {
      printf("  FAIL: smoke + high temp should trigger fire\n");
      return -1;
    }

  printf("  PASS: smoke(0.7) + temp(60C) -> FIRE "
         "(conf=%.2f)\n",
         (double)decision.confidence);
  return 0;
}

static int test_intrusion_pir_no_vision(void)
{
  struct fusion_context_s ctx;
  struct fusion_decision_s decision;
  struct fusion_input_s pir;

  fusion_init(&ctx);

  pir.type = FUSION_INPUT_PIR;
  pir.confidence = 0.8f;
  pir.value = 0;
  pir.valid = true;
  fusion_update_input(&ctx, &pir);

  /* No vision input -> intrusion */

  bool result = fusion_decide(&ctx, &decision);

  if (!result ||
      decision.alert_type != FUSION_ALERT_INTRUSION)
    {
      printf("  FAIL: PIR without vision should trigger "
             "intrusion\n");
      return -1;
    }

  printf("  PASS: PIR(0.8) alone -> INTRUSION "
         "(conf=%.2f)\n",
         (double)decision.confidence);
  return 0;
}

static int test_priority_fall_over_fire(void)
{
  struct fusion_context_s ctx;
  struct fusion_decision_s decision;
  struct fusion_input_s vision;
  struct fusion_input_s smoke;

  fusion_init(&ctx);

  /* Both fall and fire conditions present */

  vision.type = FUSION_INPUT_VISION;
  vision.confidence = 0.8f;
  vision.value = 0;
  vision.valid = true;
  fusion_update_input(&ctx, &vision);

  smoke.type = FUSION_INPUT_SMOKE;
  smoke.confidence = 0.7f;
  smoke.value = 0.8f;
  smoke.valid = true;
  fusion_update_input(&ctx, &smoke);

  bool result = fusion_decide(&ctx, &decision);

  if (!result || decision.alert_type != FUSION_ALERT_FALL)
    {
      printf("  FAIL: fall should have priority over fire\n");
      return -1;
    }

  printf("  PASS: fall + fire -> FALL (priority)\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_APP_EDGESIGHT_TEST
int main(int argc, char *argv[])
#else
int edgesight_sensor_fusion_test_main(int argc,
                                      char *argv[])
#endif
{
  int failures = 0;

  printf("===================================\n");
  printf(" EdgeSight Sensor Fusion Test\n");
  printf("===================================\n\n");

  printf("[1] No inputs (no alert):\n");
  failures += (test_no_inputs() != 0);

  printf("\n[2] Vision fall high confidence:\n");
  failures += (test_fall_vision_only() != 0);

  printf("\n[3] Vision fall low confidence:\n");
  failures += (test_fall_vision_low_confidence() != 0);

  printf("\n[4] Vision + PIR (confirm fall):\n");
  failures += (test_fall_vision_plus_pir() != 0);

  printf("\n[5] Vision + Audio (confirm fall):\n");
  failures += (test_fall_vision_plus_audio() != 0);

  printf("\n[6] Smoke + high temperature (fire):\n");
  failures += (test_fire_smoke_high_temp() != 0);

  printf("\n[7] PIR without vision (intrusion):\n");
  failures += (test_intrusion_pir_no_vision() != 0);

  printf("\n[8] Priority: fall over fire:\n");
  failures += (test_priority_fall_over_fire() != 0);

  printf("\n===================================\n");
  if (failures == 0)
    {
      printf(" ALL TESTS PASSED\n");
    }
  else
    {
      printf(" %d TEST(S) FAILED\n", failures);
    }

  printf("===================================\n");
  return failures;
}
