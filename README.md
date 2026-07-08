# CtrlFuture — STM32N647 Board Adaptation

> 2026 首届 openvela AI 硬件开发者大赛 · 队伍 #137 · 新硬件适配赛道

## 一、作品简介

本项目为 STM32N647X0（Arm Cortex-M55, ARMv8.1-M）的 openvela/NuttX 板级适配。STM32N647 是 ST 最新一代 MCU，具备 800MHz 主频、4.2MB SRAM、Neural-ART NPU（600 Gops）。

板级支持已实现**双目标条件编译**：
- **STM32N6 真机**（`configs/nsh`）— 使用 STM32N6 芯片驱动，USART1 控制台，DEV boot 模式 SRAM 加载（0x34000400, 4.2MB）
- **QEMU 仿真**（`configs/nsh-qemu`）— 使用 MPS3-AN547 芯片驱动，CMSDK UART0 控制台，flash + SRAM 布局

两套配置通过 `CONFIG_ARCH_CHIP_STM32N6` 在 board.h、board_bringup.c、链接脚本、构建系统中统一切换。

## 二、选题方向

**新硬件适配** — 将 STM32N647X0 适配到 openvela (NuttX) 系统，实现最小 NSH 基线。

## 三、目录结构

```
contest2026_137_CtrlFuture/
├── board/contest_board/              # 板级适配代码
│   ├── configs/
│   │   ├── nsh/defconfig             # STM32N6 真机配置（USART1, 4.2MB SRAM）
│   │   └── nsh-qemu/defconfig        # MPS3-AN547 QEMU 配置（CMSDK UART0, 2MB SRAM）
│   ├── include/board.h               # 双目标：STM32N6 时钟树+引脚映射 / MPS SysTick
│   ├── scripts/
│   │   ├── Make.defs                 # 编译规则（ARMv8-M 工具链, 条件选择链接脚本）
│   │   ├── flash.ld                  # STM32N6 链接脚本（SRAM 0x34000400, DEV boot）
│   │   └── flash-qemu.ld            # MPS3-AN547 链接脚本（flash 512K + sram 2M）
│   ├── src/
│   │   ├── board_bringup.c           # 板级初始化（三阶段 + STM32N6 条件编译）
│   │   ├── Makefile                  # Make 构建
│   │   └── CMakeLists.txt            # CMake 构建（条件链接脚本选择）
│   ├── Kconfig                       # Kconfig 板级配置
│   └── CMakeLists.txt                # 顶层 CMake
├── logs/                             # AI Coding 日志
├── README.md                         # 本文件
└── README.old                        # 原参赛仓库使用说明
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

# ---- STM32N6 真机固件 ----
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# ---- QEMU 仿真固件 ----
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu -j8

# 清理重新编译（以 nsh 为例）
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh distclean
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# 修改配置
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh menuconfig
```

### 4.3 QEMU 运行验证

> 注意：必须使用 `nsh-qemu` 配置编译，`nsh` 配置为真机固件，无法在 QEMU 运行。

```bash
# 编译 QEMU 固件
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu distclean
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu -j8

# 运行
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
- [x] 双目标 defconfig（nsh: STM32N6 真机 / nsh-qemu: MPS3-AN547 QEMU）
- [x] 板级头文件 board.h（STM32N6 时钟树 + 引脚映射 / MPS SysTick 双分支）
- [x] 双链接脚本（flash.ld: STM32N6 SRAM DEV boot / flash-qemu.ld: MPS3 flash+sram）
- [x] Make.defs / CMakeLists.txt 条件构建系统（按 CONFIG_ARCH_CHIP_STM32N6 选择）
- [x] 三阶段板级初始化 + stm32_board_initialize() 支持
- [x] STM32N6 芯片驱动集成（arch/arm/src/stm32n6）
- [x] STM32N6 真机编译验证：clean build 通过
- [x] QEMU 编译验证：clean build 通过
- [x] QEMU NSH 验证：命令行启动，help/hello/ps 正常

### 5.2 STM32N6 芯片驱动（arch/arm/src/stm32n6）

> NuttX 已包含 stm32n6 基础芯片架构支持，以下为在此基础上完善板级所需的芯片级驱动。

#### 5.2.1 核心系统

- [x] RCC 时钟系统 — 基础支持（HSI 64MHz, PLL1: M=4 N=50, IC1/4=200MHz CPU）
- [ ] RCC 完善（MSI 4MHz, HSE 16-48MHz, LSI 32kHz, LSE 32.768kHz, PLL2-PLL4）
- [ ] PWR 电源管理（SMPS 降压转换器, 电压缩放, Run/Sleep/Stop/Standby 低功耗模式）
- [ ] SYSCFG 系统配置控制器
- [x] NVIC 中断控制器（ARMv8-M 通用支持）
- [ ] EXTI 扩展中断/事件控制器
- [ ] MPU 内存保护单元
- [ ] TrustZone 安全域配置
- [ ] Cache 配置（32KB ICACHE + 32KB DCACHE）
- [ ] TCM 配置（128KB DTCM + 64KB ITCM, ECC）
- [ ] Backup SRAM（8KB, VBAT 域）

#### 5.2.2 启动与存储

- [x] Development Boot 模式（SRAM 加载 @ 0x34000400, ST-Link 调试）
- [ ] 外部 XSPI Flash 启动配置
- [ ] Serial Boot 模式
- [ ] XSPI1 接口驱动（扩展 SPI, 8/16-bit, 最高 200MHz）
- [ ] XSPI2 接口驱动
- [ ] XSPIM I/O 管理器
- [ ] FMC 灵活存储控制器（NOR/PSRAM/SDRAM/NAND）
- [ ] OTP 一次性可编程熔丝（1.5KB）

#### 5.2.3 GPIO 与基础外设

- [x] GPIO 驱动 — 基础支持（USART1 引脚 PE5/PE6 AF7）
- [ ] GPIO 完善（全部 165 引脚 AF 复用功能映射）
- [ ] GPDMA1 通用 DMA 控制器
- [ ] HPDMA1 高性能 DMA 控制器
- [ ] CRC 循环冗余校验单元

#### 5.2.4 串行通信接口

- [x] USART1 驱动（串口控制台, PE5-TX / PE6-RX AF7）
- [ ] USART2/3/6/10 驱动（全功能 USART, ISO7816, IrDA, LIN）
- [ ] UART4/5/7/8/9 驱动（基础异步 UART）
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

- [ ] 真实硬件启动验证（STM32N6 开发板 + ST-Link DEV boot）
- [x] 时钟树基础配置（PLL1: HSI/4×50 → 800MHz VCO → IC1/4 = 200MHz CPU）
- [ ] 时钟树完善（PLL2 → 1GHz NPU, PLL3/PLL4 外设时钟）
- [x] 串口控制台（USART1 PE5-TX/PE6-RX AF7）
- [ ] 板级外设引脚映射完善（全部 AF 复用功能表）
- [ ] 电源管理配置（SMPS, I/O 电压域, 低功耗模式）
- [ ] 板级 Kconfig 扩展（外设使能选项）
- [ ] CI 构建验证（STM32N6 + QEMU 双目标自动化测试）
- [ ] 完整功能测试套件

## 六、STM32N647 硬件规格

| 特性 | 规格 |
|------|------|
| 内核 | Arm Cortex-M55 @ 800MHz, Helium MVE, TrustZone |
| SRAM | 4.2MB 连续 + 128KB DTCM (ECC) + 64KB ITCM (ECC) |
| Flash | 无内部 Flash，从外部 XSPI Flash 启动 |
| NPU | ST Neural-ART @ 1GHz, 600 Gops |
| 时钟 | HSI 64MHz, HSE 16-48MHz, 4x PLL |
| 通信 | USART×5, UART×5, LPUART, SPI×6, I2C×4, FDCAN×3 |
| 高速 | USB OTG HS×2, Ethernet 1G, SDMMC×2 |
| 封装 | VFBGA142-264, 最多 165 GPIO |

数据手册：[STM32N647X0 (DS14791)](https://www.st.com/en/microcontrollers-microprocessors/stm32n647x0.html)

## 七、AI Coding 使用说明

> 完整对话日志见 `logs/` 目录（后续补充）。

---

原参赛仓库使用说明已保存至 [README.old](README.old)。
