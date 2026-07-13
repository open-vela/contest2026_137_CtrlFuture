# ROADMAP — STM32N647X0 板级适配

> 详细功能清单见 [FEATURE-LIST.md](FEATURE-LIST.md)。

## Phase 定义

| Phase | 名称 | 目标 | ADR 范围 |
|-------|------|------|----------|
| **P0** | 最小 NSH 启动 | 真机串口输出 `nsh>` | 001-004 |
| **P1** | 基础外设 | 完整时钟树、GPIO、DMA、SPI/I2C | 005-017 |
| **P2** | 存储 | XSPI Flash 启动、SDMMC | 018-021 |
| **P3** | 通信 | Ethernet、USB、FDCAN | 022-024 |
| **P4** | 高级功能 | NPU、Camera、Display、Security | 025-038 |

> P0/P1 为核心目标，P2/P3 为延伸目标，P4 为远期规划。

---

## P0: 最小 NSH 启动

> 目标: STM32N6 开发板通过 DEV boot 从 SRAM 启动 NuttX，USART1 输出 NSH 命令行。

| # | 模块 | ADR | 依赖 | 验证 | 状态 |
|---|------|-----|------|------|------|
| 1 | CI 集成（symlink + Kconfig patch） | [001](adr/ADR-001.md) | — | BUILD | **DONE** |
| 2 | 最小启动（RCC/GPIO/USART1/IRQ/heap） | [002](adr/ADR-002.md) | — | BUILD | **DONE** |
| 3 | SysTick 系统节拍 | [003](adr/ADR-003.md) | 002 | BUILD | **DONE** |
| 4 | 真机串口验证 | [004](adr/ADR-004.md) | 002, 003 | MEASURED | PENDING |

## P1: 基础外设

> 目标: 完整时钟树、全引脚 GPIO、DMA、SPI/I2C 总线、看门狗。

| # | 模块 | ADR | 依赖 | 验证 | 状态 |
|---|------|-----|------|------|------|
| 5 | RCC 时钟树完善 | [005](adr/ADR-005.md) | 004 | MEASURED | **DONE** |
| 6 | GPIO 完善（165 引脚 AF） | [006](adr/ADR-006.md) | 005 | BUILD | **DONE** |
| 7 | Cache 配置（ICACHE + DCACHE） | [007](adr/ADR-007.md) | 005 | MEASURED | **DONE** |
| 8 | TCM 配置（DTCM + ITCM） | [008](adr/ADR-008.md) | 005 | MEASURED | |
| 9 | PWR 电源管理 | [009](adr/ADR-009.md) | 005 | MEASURED | **DONE** |
| 10 | EXTI 扩展中断 | [010](adr/ADR-010.md) | 006 | QEMU | |
| 11 | GPDMA1 通用 DMA | [011](adr/ADR-011.md) | 005, 007 | MEASURED | |
| 12 | 额外 USART/UART | [012](adr/ADR-012.md) | 005, 006 | MEASURED | |
| 13 | SPI 驱动（SPI1-6） | [013](adr/ADR-013.md) | 006, 011 | MEASURED | |
| 14 | I2C 驱动（I2C1-4） | [014](adr/ADR-014.md) | 006, 011 | MEASURED | |
| 15 | 看门狗（IWDG/WWDG） | [015](adr/ADR-015.md) | 005 | MEASURED | |
| 16 | RTC 实时时钟 | [016](adr/ADR-016.md) | 005 | MEASURED | |
| 17 | MPU 内存保护 | [017](adr/ADR-017.md) | 007, 008 | QEMU | |

## P2: 存储

> 目标: 从外部 Flash 启动，SD 卡文件系统。

| # | 模块 | ADR | 依赖 | 验证 |
|---|------|-----|------|------|
| 18 | XSPI 接口驱动 | [018](adr/ADR-018.md) | 005, 006, 011 | MEASURED |
| 19 | XSPI Flash 启动 | [019](adr/ADR-019.md) | 018 | MEASURED |
| 20 | SDMMC 驱动 | [020](adr/ADR-020.md) | 005, 006, 011 | MEASURED |
| 21 | FMC 存储控制器 | [021](adr/ADR-021.md) | 005, 006 | MEASURED |

## P3: 通信

> 目标: 网络连接、USB 设备。

| # | 模块 | ADR | 依赖 | 验证 |
|---|------|-----|------|------|
| 22 | Ethernet GMAC (1G) | [022](adr/ADR-022.md) | 005, 006, 011, 007 | MEASURED |
| 23 | USB OTG HS | [023](adr/ADR-023.md) | 005, 006, 009 | MEASURED |
| 24 | FDCAN | [024](adr/ADR-024.md) | 005, 006 | MEASURED |

## P4: 高级功能

> 目标: NPU 推理、Camera 图像采集、显示输出、安全启动。

| # | 模块 | ADR | 依赖 | 验证 |
|---|------|-----|------|------|
| 25 | HPDMA1 高性能 DMA | [025](adr/ADR-025.md) | 011 | MEASURED |
| 26 | 高级定时器（TIM1/TIM8） | [026](adr/ADR-026.md) | 005 | MEASURED |
| 27 | 通用定时器 | [027](adr/ADR-027.md) | 005 | MEASURED |
| 28 | 低功耗定时器（LPTIM1-5） | [028](adr/ADR-028.md) | 005, 009 | MEASURED |
| 29 | ADC 驱动 | [029](adr/ADR-029.md) | 005, 006, 011 | MEASURED |
| 30 | DTS 温度传感器 | [030](adr/ADR-030.md) | 005 | MEASURED |
| 31 | LTDC LCD 控制器 | [031](adr/ADR-031.md) | 005, 006, 007, 025 | MEASURED |
| 32 | GPU2D / DMA2D | [032](adr/ADR-032.md) | 031 | MEASURED |
| 33 | DCMIPP 摄像头管线 | [033](adr/ADR-033.md) | 005, 006, 025 | MEASURED |
| 34 | CSI-2 MIPI 接口 | [034](adr/ADR-034.md) | 033 | MEASURED |
| 35 | H.264 编码器 | [035](adr/ADR-035.md) | 033 | MEASURED |
| 36 | Neural-ART NPU | [036](adr/ADR-036.md) | 005, 007, 008, 025 | MEASURED |
| 37 | 加密加速器（SAES, HASH, RNG） | [037](adr/ADR-037.md) | 005 | BUILD |
| 38 | Secure Boot | [038](adr/ADR-038.md) | 037, 019 | MEASURED |

---

## 依赖关系图

```
P0                          P1                          P2          P3
───                         ───                         ───         ───
001 CI ──────────┐
                 │
002 最小启动 ────┤
  │              │
  └─ 003 SysTick ┤
       │         │
       └─ 004 真机验证
              │
              └─ 005 RCC 完善 ─────────────────────────────────────────┐
                   │                                                   │
          ┌────────┼──────────┬──────────┬──────────┐                  │
          │        │          │          │          │                  │
        006 GPIO  007 Cache  008 TCM   009 PWR   015 WDG             │
          │        │          │          │        016 RTC             │
          │        │          │          │                            │
        010 EXTI  │          │          │                            │
          │       011 DMA ←──┘          │                            │
          │        │                    │                            │
     ┌────┼────┬───┤                    │                            │
     │    │    │   │                    │                            │
   012   013  014  │               017 MPU ← 007,008                │
   UART  SPI  I2C  │                                                │
                   │                                                │
              ┌────┼────┐                                           │
              │    │    │                                           │
            018   020   021                                        │
            XSPI  SDMMC FMC                                        │
              │                                          ┌──────────┤
            019                                        022 ETH    023 USB
            XSPI Boot                                  024 FDCAN
              │
              └───────── 038 Secure Boot ← 037 Crypto

P4 (高级功能)
───
025 HPDMA ← 011 DMA
  │
  ├──→ 031 LTDC ← 005,006,007 ──→ 032 GPU2D/DMA2D
  │      │
  ├──→ 033 DCMIPP ← 005,006 ──→ 034 CSI-2 ──→ 035 H.264
  │
  └──→ 036 NPU ← 005,007,008

026 高级定时器 ← 005
027 通用定时器 ← 005
028 低功耗定时器 ← 005,009
029 ADC ← 005,006,011
030 DTS 温度传感器 ← 005
037 加密加速器 ← 005 ──→ 038 Secure Boot ← 019
```

## 风险登记

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| 无真机硬件 | P0.4 无法验证 | 先用 QEMU 验证至 BUILD+QEMU 级别 |
| 上游 NuttX 未合入 stm32n6 | nsh 编译依赖 Kconfig patch | CI 用 symlink + patch 方案绕过 |
| PLL 时钟配置错误 | 系统无法启动 | 先用 HSI 64MHz 直连，验证后再切 PLL |
| DMA 与 Cache 一致性 | 数据损坏 | ADR-011 依赖 ADR-007，强制先做 Cache |
