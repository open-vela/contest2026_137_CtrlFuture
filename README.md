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

### 5.2 项目文档

- [ROADMAP.md](docs/ROADMAP.md) — 分阶段项目计划与时间线
- [FEATURE-LIST.md](docs/FEATURE-LIST.md) — 完整功能 checkbox 跟踪（78 项芯片驱动 + 11 项板级集成）
- [CHANGELOG.md](CHANGELOG.md) — 版本变更记录
- Architecture Decision Records: [docs/adr/](docs/adr/)

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
