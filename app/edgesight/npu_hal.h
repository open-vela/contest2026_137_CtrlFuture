/****************************************************************************
 * app/edgesight/npu_hal.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * EdgeSight - NPU hardware abstraction layer.
 * Wraps ST Edge AI runtime (stedgeai-lib) for NuttX integration.
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_NPU_HAL_H
#define __APP_EDGESIGHT_NPU_HAL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define NPU_MODEL_MAX       2  /* max simultaneous models */

/* Model indices */

#define NPU_MODEL_DETECT    0  /* Person detection (YOLO-X nano) */
#define NPU_MODEL_POSE      1  /* Pose estimation (MoveNet) */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Model info returned after loading */

struct npu_model_info_s
{
  uint32_t input_width;
  uint32_t input_height;
  uint32_t input_channels;
  uint32_t input_size_bytes;
  uint32_t output_count;
  uint32_t output_size_bytes[4]; /* up to 4 output tensors */
};

/* NPU context (opaque to application) */

struct npu_context_s
{
  bool initialized;
  bool model_loaded[NPU_MODEL_MAX];
  struct npu_model_info_s info[NPU_MODEL_MAX];
  void *runtime_ctx;    /* stedgeai runtime context */
  void *network_ctx[NPU_MODEL_MAX]; /* per-model context */
};

/* Inference result */

struct npu_inference_result_s
{
  uint32_t inference_time_ms;
  void *output_buffers[4];
  uint32_t output_sizes[4];
  uint32_t output_count;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief Initialize NPU hardware (clocks, SRAM, cache)
 * @return 0 on success, negative errno on failure
 */

int npu_hal_init(struct npu_context_s *ctx);

/**
 * @brief Load a model for inference
 * @param ctx    NPU context
 * @param index  Model index (NPU_MODEL_DETECT or NPU_MODEL_POSE)
 * @param info   Output: model input/output info
 * @return 0 on success, negative errno on failure
 */

int npu_hal_load_model(struct npu_context_s *ctx, int index,
                       struct npu_model_info_s *info);

/**
 * @brief Run synchronous inference
 * @param ctx    NPU context
 * @param index  Model index
 * @param input  Pointer to input buffer (must match model input size)
 * @param result Output: inference results
 * @return 0 on success, negative errno on failure
 */

int npu_hal_run(struct npu_context_s *ctx, int index,
                const void *input,
                struct npu_inference_result_s *result);

/**
 * @brief Deinitialize NPU and release resources
 */

void npu_hal_deinit(struct npu_context_s *ctx);

#endif /* __APP_EDGESIGHT_NPU_HAL_H */
