# CtrlFuture — STM32N647 Board Adaptation

> 2026 首届 openvela AI 硬件开发者大赛 · 队伍 #137 · 新硬件适配赛道

## 一、作品简介

本项目为 STM32N647X0（Arm Cortex-M55, ARMv8.1-M）的 openvela/NuttX 板级适配。STM32N647 是 ST 最新一代 MCU，具备 800MHz 主频、4.2MB SRAM、Neural-ART NPU（600 Gops），但尚无 NuttX 官方芯片驱动。

当前阶段采用 MPS3-AN547（同为 Cortex-M55 内核）作为底层芯片驱动，实现编译与 QEMU 验证，同时在板级文件中完整记录 STM32N647 硬件规格，为后续芯片驱动开发做准备。

## 二、选题方向

**新硬件适配** — 将 STM32N647X0 适配到 openvela (NuttX) 系统，实现最小 NSH 基线。

## 三、目录结构

```
contest2026_137_CtrlFuture/
├── board/contest_board/          # 板级适配代码
│   ├── configs/nsh/defconfig     # NuttX 配置（MPS3-AN547 + Cortex-M55）
│   ├── include/board.h           # 板级头文件（含 STM32N647 硬件规格参考）
│   ├── scripts/
│   │   ├── Make.defs             # 编译规则（ARMv8-M 工具链）
│   │   └── flash.ld              # 链接脚本
│   ├── src/
│   │   ├── board_bringup.c       # 板级初始化（三阶段启动 + procfs/tmpfs 挂载）
│   │   ├── Makefile              # Make 构建
│   │   └── CMakeLists.txt        # CMake 构建
│   ├── Kconfig                   # Kconfig 板级配置
│   └── CMakeLists.txt            # 顶层 CMake
├── logs/                         # AI Coding 日志
├── README.md                     # 本文件
└── README.old                    # 原参赛仓库使用说明
```

## 四、环境搭建与编译

### 4.1 拉取工程

```bash
repo init -u https://github.com/open-vela/contest2026_137_CtrlFuture \
  -b dev-ai-contest-2026 -m contest2026_137_CtrlFuture.xml
repo sync -c -j8
```

### 4.2 编译固件

```bash
# 进入 openvela 工作区根目录（contest2026_137_CtrlFuture 的上一级）
cd <openvela-workspace>

# 编译
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# 清理重新编译
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh distclean
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# 修改配置
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh menuconfig
```

### 4.3 QEMU 运行验证

```bash
# 使用预编译 QEMU 运行固件
prebuilts/qemu/linux-x86_64/bin/qemu-system-arm \
  -machine mps3-an547 \
  -nographic \
  -kernel nuttx/nuttx.bin

# 进入 NSH 后可执行：
#   help    — 查看所有命令
#   hello   — 运行 Hello World 示例
#   ps      — 查看任务状态
#   Ctrl+A X — 退出 QEMU
```

## 五、当前进度

### 5.1 板级基础搭建

- [x] 板级目录结构创建（board/contest_board）
- [x] 最小系统 defconfig（基于 MPS3-AN547 Cortex-M55 芯片驱动）
- [x] 板级头文件 board.h（STM32N647 硬件规格文档化）
- [x] 链接脚本 flash.ld（内存布局定义）
- [x] Make.defs / CMakeLists.txt 构建系统
- [x] 三阶段板级初始化（board_bringup / board_late_initialize / board_app_initialize）
- [x] 编译验证：clean build 通过（flash 42.9%, sram 0.63%）
- [x] QEMU 验证：NSH 命令行启动，help/hello/ps 正常

### 5.2 STM32N6 芯片驱动（arch/arm/src/stm32n6）

> NuttX 目前无 stm32n6 芯片架构支持，以下为完整适配所需开发的芯片级驱动。

#### 5.2.1 核心系统

- [ ] RCC 时钟系统（HSI 64MHz, MSI 4MHz, HSE 16-48MHz, LSI 32kHz, LSE 32.768kHz, PLL1-PLL4）
- [ ] PWR 电源管理（SMPS 降压转换器, 电压缩放, Run/Sleep/Stop/Standby 低功耗模式）
- [ ] SYSCFG 系统配置控制器
- [ ] NVIC 中断控制器（嵌套向量中断）
- [ ] EXTI 扩展中断/事件控制器
- [ ] MPU 内存保护单元
- [ ] TrustZone 安全域配置
- [ ] Cache 配置（32KB ICACHE + 32KB DCACHE）
- [ ] TCM 配置（128KB DTCM + 64KB ITCM, ECC）
- [ ] Backup SRAM（8KB, VBAT 域）

#### 5.2.2 启动与存储

- [ ] 启动模式配置（外部 XSPI Flash 启动 / Serial Boot / Development Boot）
- [ ] XSPI1 接口驱动（扩展 SPI, 8/16-bit, 最高 200MHz）
- [ ] XSPI2 接口驱动
- [ ] XSPIM I/O 管理器
- [ ] FMC 灵活存储控制器（NOR/PSRAM/SDRAM/NAND）
- [ ] OTP 一次性可编程熔丝（1.5KB）

#### 5.2.3 GPIO 与基础外设

- [ ] GPIO 驱动（最多 165 引脚, AF 复用功能映射）
- [ ] GPDMA1 通用 DMA 控制器
- [ ] HPDMA1 高性能 DMA 控制器
- [ ] CRC 循环冗余校验单元

#### 5.2.4 串行通信接口

- [ ] USART 驱动（USART1/2/3/6/10, 全功能, ISO7816, IrDA, LIN）
- [ ] UART 驱动（UART4/5/7/8/9, 基础异步）
- [ ] LPUART1 低功耗串口
- [ ] SPI 驱动（SPI1-SPI6, 其中 4 路支持 I2S）
- [ ] I2C 驱动（I2C1-I2C4, Fm+ SMBus/PMBus）
- [ ] I3C 驱动（I3C1/I3C2, 新一代集成电路互连）
- [ ] SAI 驱动（SAI1/SAI2, 串行音频接口, 4 路 DMIC）
- [ ] SPDIFRX 接收接口

#### 5.2.5 高速通信接口

- [ ] USB OTG HS 驱动（2 路高速 USB 2.0, 设备/主机/OTG）
- [ ] UCPD USB Type-C 电力传输控制器
- [ ] USB HS PHY 控制器（USBPHYC）
- [ ] Ethernet GMAC 驱动（10/100/1000 Mbps, TSN 时间敏感网络）
- [ ] FDCAN 驱动（FDCAN1/2/3, 带 TTCAN 能力）
- [ ] SDMMC 驱动（SDMMC1/SDMMC2, MMC 4.0/SD 1.0.1）
- [ ] MDIOS 管理数据 I/O 接口

#### 5.2.6 定时器系统

- [ ] TIM1/TIM8 高级控制定时器（32-bit, 4 路 IC/OC/PWM, 正交编码器, 最高 240MHz）
- [ ] TIM2/TIM3/TIM4/TIM5 通用定时器（32-bit）
- [ ] TIM9/TIM10/TIM11/TIM12/TIM13/TIM14 通用定时器（16-bit）
- [ ] TIM6/TIM7/TIM18 基本定时器
- [ ] TIM15/TIM16/TIM17 通用定时器（16-bit, 高级控制子集）
- [ ] LPTIM1-LPTIM5 低功耗定时器（5 路, 最高 240MHz）
- [ ] SysTick 系统节拍定时器
- [ ] RTC 实时时钟（亚秒精度, 硬件日历）
- [ ] IWDG 独立看门狗
- [ ] WWDG 系统窗口看门狗

#### 5.2.7 模拟外设

- [ ] ADC 驱动（ADC1/ADC2, 12-bit, 最高 5Msps, 最多 20 通道）
- [ ] DTS 数字温度传感器
- [ ] VREFBUF 内部电压参考缓冲
- [ ] MDF 多功能数字滤波器（6 路滤波器）
- [ ] ADF 音频数字滤波器（含 SAD 声学活动检测）

#### 5.2.8 图形与显示子系统

- [ ] Neo-Chrom GPU2D 2.5D 图形处理器（缩放, 旋转, 纹理映射, 透视变换）
- [ ] Chrom-ART DMA2D 加速器（位块传送, Alpha 混合）
- [ ] LTDC LCD-TFT 显示控制器（最高 XGA 分辨率）
- [ ] GFXTIM 图形专用定时器
- [ ] Chrom-GRC (GFXMMU) 图形内存管理

#### 5.2.9 摄像头与视频子系统

- [ ] DCMI 数字摄像头接口（并行）
- [ ] DCMIPP 摄像头接口像素处理管线（ISP: 坏点校正, 曝光, 去马赛克, 裁剪, YUV 转换）
- [ ] CSI-2 Host（2-lane MIPI CSI-2 摄像头接口）
- [ ] PSSI 并行同步目标接口
- [ ] VENC H.264 视频编码器（Baseline/Main/High Profile, 1080p15/720p30）
- [ ] JPEG 硬件编解码器（MJPEG 运动 JPEG）

#### 5.2.10 AI 加速器

- [ ] Neural-ART NPU 驱动（ST 神经网络加速器 @ 1GHz, 600 Gops, 288 MAC/cycle）
- [ ] NPU DNN 推理引擎接口
- [ ] NPU 流处理引擎
- [ ] NPU 实时加解密 / 权重解压缩

#### 5.2.11 安全与加密子系统

- [ ] SAES 安全 AES 协处理器（2 路, 含 DPA 防护）
- [ ] CRYP 加密处理器
- [ ] HASH 硬件哈希加速器
- [ ] PKA 公钥加速器（DPA 防护）
- [ ] MCE 存储加密引擎（v1.4）
- [ ] RNG 真随机数发生器（NIST SP800-90B）
- [ ] TAMP 篡改检测与备份寄存器
- [ ] Secure Boot (uRoT) 安全启动链
- [ ] HUK 硬件唯一密钥

#### 5.2.12 调试支持

- [ ] SWJ-DP 串行线/JTAG 调试端口
- [ ] ETM 嵌入式跟踪宏单元

### 5.3 板级集成与验证

- [ ] 真实硬件启动验证（STM32N6 开发板）
- [ ] 时钟树配置与校准（PLL1 → 800MHz CPU, PLL2 → 1GHz NPU）
- [ ] 串口控制台适配（替换 CMSDK UART → STM32N6 USART）
- [ ] 板级外设引脚映射（AF 复用功能表）
- [ ] 电源管理配置（SMPS, 低功耗模式）
- [ ] 板级设备树 / Kconfig 扩展
- [ ] 完整功能测试套件

## 六、STM32N647 硬件规格

| 特性 | 规格 |
|------|------|
| 内核 | Arm Cortex-M55 @ 800MHz, Helium MVE, TrustZone |
| SRAM | 4.2MB 连续 + 128KB DTCM (ECC) + 64KB ITCM (ECC) |
| Flash | 无内部 Flash，从外部 XSPI Flash 启动 |
| NPU | ST Neural-ART @ 1GHz, 600 Gops |
| 时钟 | HSI 64MHz, HSE 16-48MHz, 4x PLL |
| 通信 | USART×5, UART×5, LPUART, SPI×5, I2C×4, FDCAN×3 |
| 高速 | USB OTG HS×2, Ethernet 1G, SDMMC×2 |
| 封装 | VFBGA142-264, 最多 165 GPIO |

数据手册：[STM32N647X0 (DS14791)](https://www.st.com/en/microcontrollers-microprocessors/stm32n647x0.html)

## 七、AI Coding 使用说明

> 完整对话日志见 `logs/` 目录（后续补充）。

---

原参赛仓库使用说明已保存至 [README.old](README.old)。
