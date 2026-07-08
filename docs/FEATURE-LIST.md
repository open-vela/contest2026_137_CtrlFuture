# Feature List — STM32N647X0 驱动与板级集成跟踪

> 完整功能 checkbox 跟踪。阶段规划见 [ROADMAP.md](ROADMAP.md)。

## 1. STM32N6 芯片驱动（arch/arm/src/stm32n6）

> NuttX 已包含 stm32n6 基础芯片架构支持，以下为在此基础上完善板级所需的芯片级驱动。

### 1.1 核心系统

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

### 1.2 启动与存储

- [x] Development Boot 模式（SRAM 加载 @ 0x34000400, ST-Link 调试）
- [ ] 外部 XSPI Flash 启动配置
- [ ] Serial Boot 模式
- [ ] XSPI1 接口驱动（扩展 SPI, 8/16-bit, 最高 200MHz）
- [ ] XSPI2 接口驱动
- [ ] XSPIM I/O 管理器
- [ ] FMC 灵活存储控制器（NOR/PSRAM/SDRAM/NAND）
- [ ] OTP 一次性可编程熔丝（1.5KB）

### 1.3 GPIO 与基础外设

- [x] GPIO 驱动 — 基础支持（USART1 引脚 PE5/PE6 AF7）
- [ ] GPIO 完善（全部 165 引脚 AF 复用功能映射）
- [ ] GPDMA1 通用 DMA 控制器
- [ ] HPDMA1 高性能 DMA 控制器
- [ ] CRC 循环冗余校验单元

### 1.4 串行通信接口

- [x] USART1 驱动（串口控制台, PE5-TX / PE6-RX AF7）
- [ ] USART2/3/6/10 驱动（全功能 USART, ISO7816, IrDA, LIN）
- [ ] UART4/5/7/8/9 驱动（基础异步 UART）
- [ ] LPUART1 低功耗串口
- [ ] SPI 驱动（SPI1-SPI6, 其中 4 路支持 I2S）
- [ ] I2C 驱动（I2C1-I2C4, Fm+ SMBus/PMBus）
- [ ] I3C 驱动（I3C1/I3C2, 新一代集成电路互连）
- [ ] SAI 驱动（SAI1/SAI2, 串行音频接口, 4 路 DMIC）
- [ ] SPDIFRX 接收接口

### 1.5 高速通信接口

- [ ] USB OTG HS 驱动（2 路高速 USB 2.0, 设备/主机/OTG）
- [ ] UCPD USB Type-C 电力传输控制器
- [ ] USB HS PHY 控制器（USBPHYC）
- [ ] Ethernet GMAC 驱动（10/100/1000 Mbps, TSN 时间敏感网络）
- [ ] FDCAN 驱动（FDCAN1/2/3, 带 TTCAN 能力）
- [ ] SDMMC 驱动（SDMMC1/SDMMC2, MMC 4.0/SD 1.0.1）
- [ ] MDIOS 管理数据 I/O 接口

### 1.6 定时器系统

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

### 1.7 模拟外设

- [ ] ADC 驱动（ADC1/ADC2, 12-bit, 最高 5Msps, 最多 20 通道）
- [ ] DTS 数字温度传感器
- [ ] VREFBUF 内部电压参考缓冲
- [ ] MDF 多功能数字滤波器（6 路滤波器）
- [ ] ADF 音频数字滤波器（含 SAD 声学活动检测）

### 1.8 图形与显示子系统

- [ ] Neo-Chrom GPU2D 2.5D 图形处理器（缩放, 旋转, 纹理映射, 透视变换）
- [ ] Chrom-ART DMA2D 加速器（位块传送, Alpha 混合）
- [ ] LTDC LCD-TFT 显示控制器（最高 XGA 分辨率）
- [ ] GFXTIM 图形专用定时器
- [ ] Chrom-GRC (GFXMMU) 图形内存管理

### 1.9 摄像头与视频子系统

- [ ] DCMI 数字摄像头接口（并行）
- [ ] DCMIPP 摄像头接口像素处理管线（ISP: 坏点校正, 曝光, 去马赛克, 裁剪, YUV 转换）
- [ ] CSI-2 Host（2-lane MIPI CSI-2 摄像头接口）
- [ ] PSSI 并行同步目标接口
- [ ] VENC H.264 视频编码器（Baseline/Main/High Profile, 1080p15/720p30）
- [ ] JPEG 硬件编解码器（MJPEG 运动 JPEG）

### 1.10 AI 加速器

- [ ] Neural-ART NPU 驱动（ST 神经网络加速器 @ 1GHz, 600 Gops, 288 MAC/cycle）
- [ ] NPU DNN 推理引擎接口
- [ ] NPU 流处理引擎
- [ ] NPU 实时加解密 / 权重解压缩

### 1.11 安全与加密子系统

- [ ] SAES 安全 AES 协处理器（2 路, 含 DPA 防护）
- [ ] CRYP 加密处理器
- [ ] HASH 硬件哈希加速器
- [ ] PKA 公钥加速器（DPA 防护）
- [ ] MCE 存储加密引擎（v1.4）
- [ ] RNG 真随机数发生器（NIST SP800-90B）
- [ ] TAMP 篡改检测与备份寄存器
- [ ] Secure Boot (uRoT) 安全启动链
- [ ] HUK 硬件唯一密钥

### 1.12 调试支持

- [ ] SWJ-DP 串行线/JTAG 调试端口
- [ ] ETM 嵌入式跟踪宏单元

## 2. 板级集成与验证

- [ ] 真实硬件启动验证（STM32N6 开发板 + ST-Link DEV boot）
- [x] 时钟树基础配置（PLL1: HSI/4×50 → 800MHz VCO → IC1/4 = 200MHz CPU）
- [ ] 时钟树完善（PLL2 → 1GHz NPU, PLL3/PLL4 外设时钟）
- [x] 串口控制台（USART1 PE5-TX/PE6-RX AF7）
- [ ] 板级外设引脚映射完善（全部 AF 复用功能表）
- [ ] 电源管理配置（SMPS, I/O 电压域, 低功耗模式）
- [ ] 板级 Kconfig 扩展（外设使能选项）
- [x] CI 构建验证（QEMU 目标，STM32N6 待芯片驱动合入上游 NuttX 后启用）
- [ ] CI 守护完善（nxstyle 编码规范、QEMU 冒烟测试、二进制体积守护）
- [ ] Pre-commit hook（本地开发守护）
- [ ] 完整功能测试套件

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
