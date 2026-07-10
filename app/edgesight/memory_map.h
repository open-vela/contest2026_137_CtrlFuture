/****************************************************************************
 * app/edgesight/memory_map.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * EdgeSight - STM32N6 memory map and buffer allocation plan.
 *
 * STM32N647 has the following SRAM regions:
 *   FLEXMEM:  512KB @ 0x34000000 (CPU accessible, configurable)
 *   AXISRAM1: 1152KB @ 0x34080000 (CPU main RAM)
 *   AXISRAM2:  400KB @ 0x3419C000 (CPU/NPU shared)
 *   AXISRAM3:  448KB @ 0x34200000 (NPU dedicated)
 *   AXISRAM4:  448KB @ 0x34270000 (NPU dedicated)
 *   AXISRAM5:  448KB @ 0x342E0000 (NPU dedicated)
 *   AXISRAM6:  448KB @ 0x34350000 (NPU dedicated)
 *
 * External memories:
 *   XSPI1 NOR Flash: up to 256MB @ 0x70000000 (display framebuffer XIP)
 *   XSPI2 NOR Flash: up to 256MB @ 0x70000000 (model weights XIP)
 *     Model weights start at 0x70380000 (per stedgeai mpool config)
 *
 ****************************************************************************/

#ifndef __APP_EDGESIGHT_MEMORY_MAP_H
#define __APP_EDGESIGHT_MEMORY_MAP_H

/****************************************************************************
 * Memory Region Addresses
 ****************************************************************************/

#define MEM_FLEXMEM_BASE       0x34000000
#define MEM_FLEXMEM_SIZE       (512 * 1024)

#define MEM_AXISRAM1_BASE      0x34080000
#define MEM_AXISRAM1_SIZE      (1152 * 1024)

#define MEM_AXISRAM2_BASE      0x3419C000
#define MEM_AXISRAM2_SIZE      (400 * 1024)

#define MEM_AXISRAM3_BASE      0x34200000
#define MEM_AXISRAM3_SIZE      (448 * 1024)

#define MEM_AXISRAM4_BASE      0x34270000
#define MEM_AXISRAM4_SIZE      (448 * 1024)

#define MEM_AXISRAM5_BASE      0x342E0000
#define MEM_AXISRAM5_SIZE      (448 * 1024)

#define MEM_AXISRAM6_BASE      0x34350000
#define MEM_AXISRAM6_SIZE      (448 * 1024)

#define MEM_XSPI2_BASE         0x70000000
#define MEM_MODEL_WEIGHTS_BASE 0x70380000

/****************************************************************************
 * Buffer Allocation Plan
 *
 * Layout designed to avoid bus contention:
 * - CPU code/data in FLEXMEM + AXISRAM1
 * - Camera DMA targets in AXISRAM2 (shared bus)
 * - NPU weights in XSPI2 Flash (cached via AXI cache)
 * - NPU activations in AXISRAM3-6 (dedicated high-bandwidth path)
 * - Display framebuffers in AXISRAM1 (DMA2D accessible)
 *
 ****************************************************************************/

/* Display framebuffers (in AXISRAM1, upper region) */

#define DISPLAY_WIDTH          800
#define DISPLAY_HEIGHT         480
#define DISPLAY_BPP_BG         2   /* RGB565 */
#define DISPLAY_BPP_FG         2   /* ARGB4444 */

#define DISPLAY_BG_SIZE        (DISPLAY_WIDTH * DISPLAY_HEIGHT * DISPLAY_BPP_BG)
#define DISPLAY_FG_SIZE        (DISPLAY_WIDTH * DISPLAY_HEIGHT * DISPLAY_BPP_FG)

/* Camera buffers (in AXISRAM2) */

#define CAM_DISPLAY_WIDTH      800
#define CAM_DISPLAY_HEIGHT     480
#define CAM_DISPLAY_BPP        2   /* RGB565 */
#define CAM_DISPLAY_BUF_SIZE   (CAM_DISPLAY_WIDTH * CAM_DISPLAY_HEIGHT * CAM_DISPLAY_BPP)

#define CAM_NN_WIDTH           480  /* YOLO input */
#define CAM_NN_HEIGHT          480
#define CAM_NN_BPP             3    /* RGB888 */
#define CAM_NN_BUF_SIZE        (CAM_NN_WIDTH * CAM_NN_HEIGHT * CAM_NN_BPP)

#define CAM_POSE_WIDTH         192  /* MoveNet input */
#define CAM_POSE_HEIGHT        192
#define CAM_POSE_BPP           3    /* RGB888 */
#define CAM_POSE_BUF_SIZE      (CAM_POSE_WIDTH * CAM_POSE_HEIGHT * CAM_POSE_BPP)

/* VENC (H.264) buffers — in AXISRAM2 remaining space */

#define VENC_WIDTH             1280
#define VENC_HEIGHT            720
#define VENC_BPP               2    /* YUV422 */
#define VENC_INPUT_BUF_SIZE    (VENC_WIDTH * VENC_HEIGHT * VENC_BPP)

/* NPU buffers (AXISRAM3-6, managed by stedgeai runtime) */

#define NPU_SRAM_TOTAL         (4 * 448 * 1024)  /* 1.75 MB */

/****************************************************************************
 * Memory Budget Summary
 *
 * Region       | Usage                  | Size      | Available
 * -------------|------------------------|-----------|----------
 * FLEXMEM      | NuttX heap + stack     | ~400 KB   | 512 KB
 * AXISRAM1     | Display FB (bg+fg×2)   | ~2.3 MB   | ~1.15 MB (tight!)
 * AXISRAM2     | Camera bufs + VENC     | ~2.6 MB   | 400 KB (OVERFLOW)
 * AXISRAM3-6   | NPU activations        | 1.75 MB   | 1.75 MB (dedicated)
 * XSPI2 Flash  | Model weights          | ~13.6 MB  | 16+ MB
 *
 * NOTE: AXISRAM2 (400KB) cannot hold all camera + VENC buffers.
 * Solutions:
 *   1. Use XSPI1 for display framebuffer (memory-mapped)
 *   2. Use AXISRAM1 for camera display buffer
 *   3. Use VENC slice mode (16-line macroblock buffer instead of full frame)
 *   4. Reduce NN input to 320×320 (saves 300KB)
 *
 ****************************************************************************/

#endif /* __APP_EDGESIGHT_MEMORY_MAP_H */
