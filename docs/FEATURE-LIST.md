# Feature List — STM32N647X0 驱动与板级集成跟踪

> 完整功能 checkbox 跟踪。阶段规划见 [ROADMAP.md](ROADMAP.md)。

> **图例 (2026-07-20 起)**：
> - `[x]` 完成 —— BUILD + QEMU/Renode 仿真级验证通过
> - `[~]` 部分完成 —— 代码存在但有已知缺口（见各项备注）
> - `[ ]` 未开始
>
> **验证口径**：ADR-004（真机验证）仍 PENDING，所有 `[x]`/`[~]`
> 均未经过真机 MEASURED 验证。

## 1. STM32N6 芯片驱动（arch/arm/src/stm32n6）

> NuttX 已包含 stm32n6 基础芯片架构支持，以下为在此基础上完善板级所需的芯片级驱动。

### 1.1 核心系统

- [x] RCC 时钟系统 — 基础支持（HSI 64MHz, PLL1: M=4 N=50, IC1/4=200MHz CPU）
- [~] RCC 完善（ADR-005：HSI/HSE/PLL1 + 时钟 API 已 DONE；MSI/LSI/LSE/PLL2-4 未实现，board `stm32n6_clockconfig.c` 有 8 处 TODO）
- [x] PWR 电源管理（ADR-009：电压缩放、backup-domain/VDDIO 助手；SMPS 配置未实现）
- [ ] SYSCFG 系统配置控制器
- [x] NVIC 中断控制器（ARMv8-M 通用支持）
- [x] EXTI 扩展中断/事件控制器（ADR-010）
- [x] MPU 内存保护单元（ADR-017）
- [ ] TrustZone 安全域配置
- [x] Cache 配置（32KB ICACHE + 32KB DCACHE，ADR-007）
- [x] TCM 配置（128KB DTCM + 64KB ITCM, ECC，ADR-008）
- [ ] Backup SRAM（8KB, VBAT 域）

### 1.2 启动与存储

- [x] Development Boot 模式（SRAM 加载 @ 0x34000400, ST-Link 调试）
- [x] 外部 XSPI Flash 启动配置（ADR-019，仿真级）
- [ ] Serial Boot 模式
- [~] XSPI1 接口驱动（ADR-018：仅 memory-mapped 只读；间接模式写/擦除为死代码，xspi.c:112-117）
- [~] XSPI2 接口驱动（xspi.c 支持 bus 2，但无 `STM32_XSPI2` Kconfig 门控）
- [ ] XSPIM I/O 管理器
- [ ] FMC 灵活存储控制器（NOR/PSRAM/SDRAM/NAND，ADR-021）
- [ ] OTP 一次性可编程熔丝（1.5KB；仅 Renode L1 模型）

### 1.3 GPIO 与基础外设

- [x] GPIO 驱动 — 基础支持（USART1 引脚 PE5/PE6 AF7）
- [x] GPIO 完善（全部 165 引脚 AF 复用功能映射，ADR-006）
- [~] GPDMA1 通用 DMA 控制器（ADR-011：mem2mem 轮询可用；无 ISR、无外设触发/DMAMUX 路径）
- [ ] HPDMA1 高性能 DMA 控制器 —— **决策不实现独立驱动**（ADR-025 Accepted：由 DCMIPP/VENC/NPU 硬件管道自管理）
- [ ] CRC 循环冗余校验单元

### 1.4 串行通信接口

- [x] USART1 驱动（串口控制台, PE5-TX / PE6-RX AF7）
- [~] USART2/3/6/10 驱动（ADR-012：USART2/3 有 Kconfig 与驱动支持；USART6/10 仅 Renode 模型覆盖）
- [~] UART4/5/7/8/9 驱动（仅 Renode 模型 L1-L3 覆盖，无 NuttX 驱动/Kconfig）
- [ ] LPUART1 低功耗串口
- [~] SPI 驱动（ADR-013：SPI1/2 有 Kconfig 门控，轮询传输；SPI3-6 仅 Renode 模型；无 IRQ/DMA 路径）
- [~] I2C 驱动（ADR-014：I2C1/2 有 Kconfig 门控；`.irq = 0` 实为轮询（i2c.c:148）；I2C3/4 仅 Renode 模型）
- [ ] I3C 驱动（I3C1/I3C2, 新一代集成电路互连）
- [~] SAI 驱动（stm32n6_sai.c：SAI1 仅输入路径；无输出，SAI2 未支持）
- [ ] SPDIFRX 接收接口

### 1.5 高速通信接口

- [~] USB OTG HS 驱动（ADR-023：设备模式初始化；主机模式 -ENOSYS（otg.c:279），EP 数据通路未实现）
- [ ] UCPD USB Type-C 电力传输控制器
- [ ] USB HS PHY 控制器（USBPHYC）
- [~] Ethernet GMAC 驱动（ADR-022：初始化 + 描述符构建；无 `netdev_register`、无 TX/RX 数据路径与 ISR、无 deinit）
- [~] FDCAN 驱动（ADR-024：init/send/receive 存在；无 `can_register` 上半部、无 ISR）
- [~] SDMMC 驱动（ADR-020：块读写存在；无 `mmcsd_register`（sdmmc.c:515），VFS 不可达；SDMMC2 无 Kconfig 门控）
- [ ] MDIOS 管理数据 I/O 接口

### 1.6 定时器系统

- [ ] TIM1/TIM8 高级控制定时器（32-bit, 4 路 IC/OC/PWM, 正交编码器, 最高 240MHz；仅 Renode L2 模型）
- [ ] TIM2/TIM3/TIM4/TIM5 通用定时器（32-bit）
- [ ] TIM9/TIM10/TIM11/TIM12/TIM13/TIM14 通用定时器（16-bit）
- [ ] TIM6/TIM7/TIM18 基本定时器
- [ ] TIM15/TIM16/TIM17 通用定时器（16-bit, 高级控制子集）
- [ ] LPTIM1-LPTIM5 低功耗定时器（5 路, 最高 240MHz；仅 Renode L2 模型）
- [x] SysTick 系统节拍定时器
- [~] RTC 实时时钟（ADR-016：init/get/set 可用；缺 backup-domain 写解锁，真机写入可能被丢弃 —— UPSTREAM_PORTING_GAPS #10）
- [x] IWDG 独立看门狗（ADR-015）
- [ ] WWDG 系统窗口看门狗

### 1.7 模拟外设

- [ ] ADC 驱动（ADC1/ADC2, 12-bit, 最高 5Msps, 最多 20 通道；仅 Renode L3 模型）
- [ ] DTS 数字温度传感器（仅 Renode L2/L3 模型）
- [ ] VREFBUF 内部电压参考缓冲
- [ ] MDF 多功能数字滤波器（6 路滤波器）
- [ ] ADF 音频数字滤波器（含 SAD 声学活动检测）

### 1.8 图形与显示子系统

- [ ] Neo-Chrom GPU2D 2.5D 图形处理器（缩放, 旋转, 纹理映射, 透视变换）
- [ ] Chrom-ART DMA2D 加速器（位块传送, Alpha 混合；仅 Renode L1 模型）
- [~] LTDC LCD-TFT 显示控制器（ADR-031 PARTIAL：/dev/fb0 框架已注册，**未编程任何 LTDC 寄存器**；`CONFIG_VIDEO_FB` 门控且未在任何 defconfig 启用）
- [ ] GFXTIM 图形专用定时器
- [ ] Chrom-GRC (GFXMMU) 图形内存管理

### 1.9 摄像头与视频子系统

- [ ] DCMI 数字摄像头接口（并行）
- [~] DCMIPP 摄像头接口像素处理管线（ADR-033 PARTIAL：/dev/video0 框架已注册，CMW_CAMERA 调用全注释；`CONFIG_VIDEO` 门控且未在任何 defconfig 启用）
- [ ] CSI-2 Host（2-lane MIPI CSI-2 摄像头接口；仅 Renode L1 占位模型）
- [ ] PSSI 并行同步目标接口
- [ ] VENC H.264 视频编码器（Baseline/Main/High Profile, 1080p15/720p30；仅 Renode L1 模型）
- [ ] JPEG 硬件编解码器（MJPEG 运动 JPEG）

### 1.10 AI 加速器

- [ ] Neural-ART NPU 驱动（ADR-036 Proposed：stm32n6_npu.c 不存在；app 侧 npu_hal.c 为 stub）
- [ ] NPU DNN 推理引擎接口
- [ ] NPU 流处理引擎
- [ ] NPU 实时加解密 / 权重解压缩

### 1.11 安全与加密子系统

- [ ] SAES 安全 AES 协处理器（2 路, 含 DPA 防护）
- [ ] CRYP 加密处理器（仅 Renode L1 模型）
- [ ] HASH 硬件哈希加速器
- [ ] PKA 公钥加速器（DPA 防护）
- [ ] MCE 存储加密引擎（v1.4）
- [x] RNG 真随机数发生器（stm32n6_rng.c 完整；`CONFIG_STM32_RNG` 已在 nsh defconfig 启用）
- [ ] TAMP 篡改检测与备份寄存器
- [ ] Secure Boot (uRoT) 安全启动链（ADR-038）
- [ ] HUK 硬件唯一密钥

### 1.12 调试支持

- [ ] SWJ-DP 串行线/JTAG 调试端口
- [ ] ETM 嵌入式跟踪宏单元

## 2. 板级集成与验证

- [ ] 真实硬件启动验证（STM32N6 开发板 + ST-Link DEV boot，ADR-004 PENDING —— 所有 MEASURED 级验证的总闸门）
- [x] 时钟树基础配置（PLL1: HSI/4×50 → 800MHz VCO → IC1/4 = 200MHz CPU）
- [ ] 时钟树完善（PLL2 → 1GHz NPU, PLL3/PLL4 外设时钟；board `stm32n6_clockconfig.c` 8 处 TODO）
- [x] 串口控制台（USART1 PE5-TX/PE6-RX AF7）
- [x] 板级外设引脚映射完善（全部 AF 复用功能表，ADR-006）
- [~] 电源管理配置（ADR-009：电压缩放 DONE；SMPS 与 Stop/Standby 低功耗模式未配置）
- [x] 板级 Kconfig 扩展（STM32_* 外设使能选项）
- [x] CI 构建验证（nsh + nsh-qemu 双目标；edgesight config 未纳入 CI）
- [x] CI 守护完善（nxstyle 编码规范、QEMU 冒烟测试、二进制体积守护）
- [x] Pre-commit hook（本地开发守护）
- [~] 完整功能测试套件（Renode 50 个 Robot suite 全绿；无真机测试；edgesight 应用单测未接入运行）

## 3. 交叉引用

| 功能模块 | ADR | Phase |
|----------|-----|-------|
| CI 集成 | [ADR-001](adr/ADR-001.md) | P0 |
| 最小启动（RCC/GPIO/USART1/IRQ/heap） | [ADR-002](adr/ADR-002.md) | P0 |
| SysTick 系统节拍 | [ADR-003](adr/ADR-003.md) | P0 |
| 真机串口验证 | [ADR-004](adr/ADR-004.md) | P0 |
| RCC 时钟树完善 | [ADR-005](adr/ADR-005.md) | P1 |
| GPIO 完善 | [ADR-006](adr/ADR-006.md) | P1 |
| Cache 配置 | [ADR-007](adr/ADR-007.md) | P1 |
| TCM 配置 | [ADR-008](adr/ADR-008.md) | P1 |
| PWR 电源管理 | [ADR-009](adr/ADR-009.md) | P1 |
| EXTI 扩展中断 | [ADR-010](adr/ADR-010.md) | P1 |
| GPDMA1 通用 DMA | [ADR-011](adr/ADR-011.md) | P1 |
| 额外 USART/UART | [ADR-012](adr/ADR-012.md) | P1 |
| SPI 驱动 | [ADR-013](adr/ADR-013.md) | P1 |
| I2C 驱动 | [ADR-014](adr/ADR-014.md) | P1 |
| 看门狗 | [ADR-015](adr/ADR-015.md) | P1 |
| RTC 实时时钟 | [ADR-016](adr/ADR-016.md) | P1 |
| MPU 内存保护 | [ADR-017](adr/ADR-017.md) | P1 |
| XSPI 接口驱动 | [ADR-018](adr/ADR-018.md) | P2 |
| XSPI Flash 启动 | [ADR-019](adr/ADR-019.md) | P2 |
| SDMMC 驱动 | [ADR-020](adr/ADR-020.md) | P2 |
| FMC 存储控制器 | [ADR-021](adr/ADR-021.md) | P2 |
| Ethernet GMAC | [ADR-022](adr/ADR-022.md) | P3 |
| USB OTG HS | [ADR-023](adr/ADR-023.md) | P3 |
| FDCAN | [ADR-024](adr/ADR-024.md) | P3 |
| HPDMA1 高性能 DMA | [ADR-025](adr/ADR-025.md) | P4 |
| 高级定时器 | [ADR-026](adr/ADR-026.md) | P4 |
| 通用定时器 | [ADR-027](adr/ADR-027.md) | P4 |
| 低功耗定时器 | [ADR-028](adr/ADR-028.md) | P4 |
| ADC 驱动 | [ADR-029](adr/ADR-029.md) | P4 |
| DTS 温度传感器 | [ADR-030](adr/ADR-030.md) | P4 |
| LTDC LCD 控制器 | [ADR-031](adr/ADR-031.md) | P4 |
| GPU2D / DMA2D | [ADR-032](adr/ADR-032.md) | P4 |
| DCMIPP 摄像头管线 | [ADR-033](adr/ADR-033.md) | P4 |
| CSI-2 MIPI 接口 | [ADR-034](adr/ADR-034.md) | P4 |
| H.264 编码器 | [ADR-035](adr/ADR-035.md) | P4 |
| Neural-ART NPU | [ADR-036](adr/ADR-036.md) | P4 |
| 加密加速器 | [ADR-037](adr/ADR-037.md) | P4 |
| Secure Boot | [ADR-038](adr/ADR-038.md) | P4 |
