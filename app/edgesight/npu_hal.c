/****************************************************************************
 * app/edgesight/npu_hal.c
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
 * EdgeSight - NPU HAL implementation.
 * Wraps ST Edge AI runtime (stedgeai-lib) for NuttX.
 *
 * TODO: Replace stub code with actual stedgeai API calls when
 * hardware is available.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "npu_hal.h"
#include "memory_map.h"
#include <string.h>
#include <stdio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* When building with real hardware, include stedgeai headers:
 * #include "stai.h"
 * #include "stai_network.h"
 */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief Enable NPU SRAM banks and clocks
 *
 * Reference: STM32N6-GettingStarted-ObjectDetection/main.c NPURam_enable()
 *
 * On real hardware:
 *   __HAL_RCC_NPU_CLK_ENABLE();
 *   __HAL_RCC_NPU_FORCE_RESET();
 *   __HAL_RCC_NPU_RELEASE_RESET();
 *   __HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();
 *   __HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();
 *   __HAL_RCC_AXISRAM5_MEM_CLK_ENABLE();
 *   __HAL_RCC_AXISRAM6_MEM_CLK_ENABLE();
 *   + RAMCFG_HandleTypeDef for each bank
 */

static int npu_enable_hw(void)
{
  /* TODO: Implement with real HAL calls */

  printf("[npu] NPU clocks and SRAM enabled (stub)\n");
  return 0;
}

/**
 * @brief Enable NPU AXI cache
 *
 * Reference: npu_cache_enable_clocks_and_reset()
 *
 * On real hardware:
 *   __HAL_RCC_CACHEAXIRAM_MEM_CLK_ENABLE();
 *   __HAL_RCC_CACHEAXI_CLK_ENABLE();
 *   __HAL_RCC_CACHEAXI_FORCE_RESET();
 *   __HAL_RCC_CACHEAXI_RELEASE_RESET();
 *   npu_cache_enable();
 */

static int npu_enable_cache(void)
{
  /* TODO: Implement with real HAL calls */

  printf("[npu] NPU cache enabled (stub)\n");
  return 0;
}

/**
 * @brief Configure sleep mode clock retention for NPU
 *
 * Reference: set_clk_sleep_mode()
 */

static void npu_config_sleep_clocks(void)
{
  /* TODO: Implement with real HAL calls:
   * __HAL_RCC_NPU_CLK_SLEEP_ENABLE();
   * __HAL_RCC_CACHEAXI_CLK_SLEEP_ENABLE();
   * __HAL_RCC_AXISRAM3..6_MEM_CLK_SLEEP_ENABLE();
   */
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int npu_hal_init(struct npu_context_s *ctx)
{
  int ret;

  memset(ctx, 0, sizeof(*ctx));

  /* Step 1: Enable NPU hardware */

  ret = npu_enable_hw();
  if (ret < 0)
    {
      return ret;
    }

  /* Step 2: Enable NPU cache */

  ret = npu_enable_cache();
  if (ret < 0)
    {
      return ret;
    }

  /* Step 3: Configure sleep mode retention */

  npu_config_sleep_clocks();

  /* Step 4: Initialize stedgeai runtime
   *
   * On real hardware:
   *   ret = stai_runtime_init();
   *   assert(ret == STAI_SUCCESS);
   */

  printf("[npu] stedgeai runtime initialized (stub)\n");

  ctx->initialized = true;
  return 0;
}

int npu_hal_load_model(struct npu_context_s *ctx, int index,
                       struct npu_model_info_s *info)
{
  if (!ctx->initialized || index >= NPU_MODEL_MAX)
    {
      return -1;
    }

  /* On real hardware:
   *   ret = stai_network_init(network_ctx[index]);
   *   ret = stai_network_get_info(network_ctx[index], &stai_info);
   *   info->input_width = stai_info.inputs[0].shape[1];
   *   info->input_height = stai_info.inputs[0].shape[2];
   *   ...
   */

  /* Fill placeholder info based on model index */

  if (index == NPU_MODEL_DETECT)
    {
      /* YOLO-X nano: 480x480x3 input, single output tensor */

      info->input_width = 480;
      info->input_height = 480;
      info->input_channels = 3;
      info->input_size_bytes = 480 * 480 * 3;
      info->output_count = 1;
      info->output_size_bytes[0] = 2100 * 6;  /* ~2100 anchors × 6 */
    }
  else if (index == NPU_MODEL_POSE)
    {
      /* MoveNet Lightning: 192x192x3 input, 17×3 output */

      info->input_width = 192;
      info->input_height = 192;
      info->input_channels = 3;
      info->input_size_bytes = 192 * 192 * 3;
      info->output_count = 1;
      info->output_size_bytes[0] = 17 * 3;
    }
  else
    {
      return -1;
    }

  ctx->info[index] = *info;
  ctx->model_loaded[index] = true;

  printf("[npu] Model %d loaded (stub): %lux%lux%lu\n",
         index,
         (unsigned long)info->input_width,
         (unsigned long)info->input_height,
         (unsigned long)info->input_channels);

  return 0;
}

int npu_hal_run(struct npu_context_s *ctx, int index,
                const void *input,
                struct npu_inference_result_s *result)
{
  if (!ctx->initialized || !ctx->model_loaded[index])
    {
      return -1;
    }

  (void)input;

  /* On real hardware:
   *   stai_network_get_inputs(ctx->network_ctx[index], &nn_in, &n);
   *   memcpy(nn_in, input, ctx->info[index].input_size_bytes);
   *   SCB_CleanInvalidateDCache_by_Addr(nn_in, size);
   *   uint32_t t0 = HAL_GetTick();
   *   stai_network_run(ctx->network_ctx[index], STAI_MODE_SYNC);
   *   uint32_t t1 = HAL_GetTick();
   *   stai_network_get_outputs(ctx->network_ctx[index], out, &n_out);
   */

  memset(result, 0, sizeof(*result));
  result->inference_time_ms = 0;  /* Will be real timing on HW */
  result->output_count = ctx->info[index].output_count;

  return 0;
}

void npu_hal_deinit(struct npu_context_s *ctx)
{
  if (!ctx->initialized)
    {
      return;
    }

  /* On real hardware:
   *   stai_network_deinit(ctx->network_ctx[0]);
   *   stai_network_deinit(ctx->network_ctx[1]);
   */

  memset(ctx, 0, sizeof(*ctx));
  printf("[npu] NPU deinitialized\n");
}
