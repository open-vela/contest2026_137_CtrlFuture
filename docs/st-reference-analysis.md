# ST 官方参考代码分析报告

## 分析范围

- `STM32CubeN6` — HAL/LL 驱动 + 全套官方例程
- `STM32N6-GettingStarted-ObjectDetection` — NPU 目标检测完整工程

## 一、HAL 驱动覆盖情况

ST 官方 HAL 驱动（`stm32n6xx-hal-driver`）已覆盖 EdgeSight 所有需要的外设：

| 外设 | HAL 文件 | 代码行数 | 移植难度 |
|------|----------|---------|---------|
| DCMIPP (Camera ISP) | hal_dcmipp.c | 8603 | 高 |
| LTDC (显示) | hal_ltdc.c | 4164 | 中 |
| GPU2D | hal_gpu2d.c | 752 | 低 |
| Ethernet | hal_eth.c | 3652 | 中高 |
| SD/MMC | hal_sd.c + ll_sdmmc.c | 4089+2111 | 中高 |
| XSPI (Flash) | hal_xspi.c | 3598 | 中 |
| DMA | hal_dma.c | 1812 | 中 |
| DMA2D | hal_dma2d.c | 2186 | 中 |
| VENC (H.264) | ll_venc.c | 148 (LL薄封装) | 高(需h264encapi) |
| SAI (Audio) | hal_sai.c | 2898 | 中 |
| I2C | hal_i2c.c | 7824 | 中 |
| SPI | hal_spi.c | 4054 | 中 |

## 二、关键例程对应关系

| EdgeSight 需求 | ST 官方例程 | 路径 |
|---------------|------------|------|
| Camera + ISP + NPU + 显示 | ObjectDetection | STM32N6-GettingStarted-ObjectDetection/ |
| H.264 + RTSP 流 + 音频 | VENC_RTSP_Server | Projects/STM32N6570-DK/Applications/VENC/VENC_RTSP_Server/ |
| H.264 + SD 卡录像 | VENC_SDCard_ThreadX | Projects/STM32N6570-DK/Applications/VENC/VENC_SDCard_ThreadX/ |
| H.264 + USB 视频流 | VENC_USB | Projects/STM32N6570-DK/Applications/VENC/VENC_USB/ |
| LTDC 显示 | LTDC_Horizontal_Mirroring | Projects/STM32N6570-DK/Examples/LTDC/ |
| SD 卡 | SD example | Projects/STM32N6570-DK/Examples/SD/ |
| XSPI Flash | XSPI example | Projects/NUCLEO-N657X0-Q/Examples/XSPI/ |
| Ethernet | NetXDuo | Projects/STM32N6570-DK/Applications/NetXDuo/ |

## 三、时钟树配置（800MHz）

从 `SystemClock_Config_800MHz.c` 提取的完整配置：

```
前置条件: BSP_SMPS_Init(SMPS_VOLTAGE_OVERDRIVE) + PWR_REGULATOR_VOLTAGE_SCALE0

PLL1: HSI(64MHz) / M=2 * N=25 = 800MHz
  → IC1 div=1 → CPU 800MHz
  → IC2 div=2 → AXI 400MHz
  → IC4 div=4 → SDMMC 200MHz

PLL2: HSI(64MHz) / M=8 * N=125 = 1000MHz
  → IC6 div=1 → NPU 1000MHz (ObjectDetection 用)
  → IC17 div=3 → DCMIPP 333MHz

PLL3: HSI(64MHz) / M=8 * N=225 = 900MHz (或 BYPASS)
  → IC11 div=1 → AXISRAM3/4/5/6 900MHz
  → IC8 → MDF1(音频) 时钟源

PLL4: HSI(64MHz) / M=8 * N=172 → 各种外设时钟
  → IC7 → SAI1(音频输出) 时钟源

总线:
  HCLK = AXI/2 = 200MHz
  PCLK1~5 = HCLK = 200MHz
```

## 四、NPU 初始化关键步骤

从 ObjectDetection main.c 提取：

```c
// 1. 使能 NPU 时钟并复位
__HAL_RCC_NPU_CLK_ENABLE();
__HAL_RCC_NPU_FORCE_RESET();
__HAL_RCC_NPU_RELEASE_RESET();

// 2. 使能 NPU 专用 SRAM (4 x 448KB = 1.75MB)
__HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();
__HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();
__HAL_RCC_AXISRAM5_MEM_CLK_ENABLE();
__HAL_RCC_AXISRAM6_MEM_CLK_ENABLE();
// + RAMCFG 使能每个 bank

// 3. 使能 NPU Cache
__HAL_RCC_CACHEAXIRAM_MEM_CLK_ENABLE();
__HAL_RCC_CACHEAXI_CLK_ENABLE();
npu_cache_enable();

// 4. 安全域配置（RIF）
// NPU/DMA2D/DCMIPP/LTDC 设为 secure privileged

// 5. Sleep 模式时钟保持（推理期间 WFE 不断时钟）
__HAL_RCC_NPU_CLK_SLEEP_ENABLE();
__HAL_RCC_CACHEAXI_CLK_SLEEP_ENABLE();
// ... 各 SRAM bank

// 6. NPU 推理 API
stai_runtime_init();
stai_network_init(context);
stai_network_get_info(context, &info);
stai_network_get_inputs(context, &nn_in, &n_inputs);
stai_network_run(context, STAI_MODE_SYNC);
```

## 五、Camera + DCMIPP 双管线架构

```
Camera Sensor (IMX335)
    │
    ├─ PIPE1 → 缩放到 LCD 尺寸 → RGB565 → LTDC Layer0 (显示)
    │          CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE1, ...)
    │          CMW_CAMERA_Start(DCMIPP_PIPE1, lcd_buffer, CMW_MODE_CONTINUOUS)
    │
    └─ PIPE2 → 缩放到 NN 输入尺寸 → RGB888 → NPU 推理
               CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE2, ...)
               CMW_CAMERA_Start(DCMIPP_PIPE2, nn_input, CMW_MODE_SNAPSHOT)
```

Camera 中间件 (`stm32-mw-camera`) 封装了 DCMIPP 初始化 + ISP 配置 + 传感器驱动。

## 六、H.264 编码架构

```
DCMIPP → YUV422/420 帧 → H264EncIn → h264encapi → H264EncOut → 输出 NAL 单元
                                                                    │
                                                    ┌───────────────┼───────────────┐
                                                    │               │               │
                                              RTSP/RTP         SD Card          USB UVC

支持模式:
- Frame mode: 整帧编码（双缓冲）
- Slice mode (HW handshake): 逐行宏块编码（低延迟，单缓冲）

支持分辨率: 480p / 720p / 1080p
编码器 API: h264encapi.h (ST 私有库)
LL 驱动: stm32n6xx_ll_venc.c (寄存器级薄封装)
```

## 七、XSPI Flash 初始化

```c
BSP_XSPI_NOR_Init_t NOR_Init;
NOR_Init.InterfaceMode = BSP_XSPI_NOR_OPI_MODE;
NOR_Init.TransferRate = BSP_XSPI_NOR_DTR_TRANSFER;
BSP_XSPI_NOR_Init(0, &NOR_Init);
BSP_XSPI_NOR_EnableMemoryMappedMode(0);  // XIP 模式
```

模型权重存储在 XSPI2 Flash，通过 memory-mapped 直接寻址。

## 八、对 NuttX 移植的策略建议

### 方案 A: 最小改动（推荐保底）
直接在 NuttX 里调用 ST HAL 库，不重写底层寄存器操作。
- 优点：快速、可靠、100% 兼容 ST 参考代码
- 缺点：引入 HAL 依赖，代码量大，不符合 NuttX 纯寄存器风格
- 适用：NPU、VENC、DCMIPP 这些复杂 IP（自己写寄存器不现实）

### 方案 B: NuttX 原生驱动
参考 HAL 的寄存器操作逻辑，写成 NuttX driver 框架。
- 优点：代码干净，符合社区规范
- 缺点：工作量大
- 适用：Ethernet、SDMMC、LTDC、XSPI 这些有成熟 NuttX 模式的外设

### 推荐组合：
| 外设 | 策略 |
|------|------|
| Ethernet | 方案 B（参考 stm32h7 NuttX 驱动 + N6 HAL 寄存器定义） |
| SDMMC | 方案 B（参考 stm32h7） |
| XSPI | 方案 B（参考 stm32h7 QSPI） |
| LTDC | 方案 B（参考 stm32f7 LTDC） |
| DCMIPP + ISP | 方案 A（直接用 stm32-mw-camera 中间件） |
| NPU | 方案 A（直接用 stedgeai-lib） |
| VENC | 方案 A（直接用 h264encapi） |
| GPU2D | 方案 A（HAL 很薄，直接包装） |
