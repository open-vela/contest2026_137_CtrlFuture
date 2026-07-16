# CtrlFuture — STM32N647 Board Adaptation

> 2026 首届 openvela AI 硬件开发者大赛 · 队伍 #137 · 新硬件适配赛道

## 一、作品简介

本项目为 STM32N647X0（Arm Cortex-M55, ARMv8.1-M）的 openvela/NuttX 板级适配。STM32N647 是 ST 最新一代 MCU，具备 800MHz 主频、4.2MB SRAM、Neural-ART NPU（600 Gops）。

板级支持已实现**双目标条件编译**：
- **STM32N6 真机**（`configs/nsh`）— 使用本仓库 STM32N6 芯片驱动（`arch/arm/stm32n6/`），USART1 控制台，DEV boot 模式 SRAM 加载（0x34000400, 4.2MB）
- **QEMU 仿真**（`configs/nsh-qemu`）— 使用 NuttX 上游 MPS3-AN547 芯片驱动，CMSDK UART0 控制台，flash + SRAM 布局

两套配置通过 `CONFIG_ARCH_CHIP_STM32N6` 在 board.h、board_bringup.c、链接脚本、构建系统中统一切换。

## 二、选题方向

**新硬件适配** — 将 STM32N647X0 适配到 openvela (NuttX) 系统，实现最小 NSH 基线。

## 三、目录结构

```
contest2026_137_CtrlFuture/
├── arch/arm/stm32n6/                 # ★ STM32N6 芯片驱动（out-of-tree）
│   ├── src/                          # 驱动源码（symlink → nuttx/arch/arm/src/stm32n6）
│   │   ├── stm32n6_start.c           # 复位入口
│   │   ├── stm32n6_rcc.c             # 时钟配置（HSI 64MHz）
│   │   ├── stm32n6_gpio.c            # GPIO 驱动（PE5/PE6 USART1）
│   │   ├── stm32n6_lowputc.c         # 低级串口输出
│   │   ├── stm32n6_serial.c          # NuttX serial 框架驱动
│   │   ├── stm32n6_irq.c             # NVIC 中断控制器
│   │   ├── stm32n6_timerisr.c        # SysTick 系统定时器
│   │   ├── stm32n6_allocateheap.c    # 堆分配
│   │   └── hardware/stm32_memorymap.h
│   ├── include/                      # 芯片头文件（symlink → nuttx/arch/arm/include/stm32n6）
│   └── kconfig-stm32n6.patch         # Kconfig 注入补丁
├── board/contest_board/              # 板级适配代码
│   ├── configs/
│   │   ├── nsh/defconfig             # STM32N6 真机配置
│   │   └── nsh-qemu/defconfig        # MPS3-AN547 QEMU 配置
│   ├── include/board.h               # 双目标：STM32N6 时钟+引脚 / MPS SysTick
│   ├── scripts/                      # 链接脚本 + Make.defs
│   └── src/board_bringup.c           # 板级初始化
├── app/hello_app/                    # Hello World 示例应用
├── docs/                             # 项目文档
│   ├── adr/ADR-001~038.md            # 38 个架构决策记录
│   ├── ROADMAP.md                    # 分阶段计划 + 依赖图
│   ├── FEATURE-LIST.md               # 完整功能 checkbox
│   └── BEST-PRACTICES.md             # 嵌入式开发最佳实践
├── scripts/                          # CI 检查脚本
├── .github/workflows/build.yml       # GitHub Actions CI
├── .githooks/                        # 本地 commit hooks
├── CHANGELOG.md                      # 变更日志
└── logs/                             # AI Coding 日志
```

## 四、环境搭建与编译

### 4.1 拉取工程

```bash
repo init -u https://github.com/open-vela/contest2026_137_CtrlFuture \
  -b dev-ai-contest-2026 -m contest2026_137_CtrlFuture.xml
repo sync -c -j8
```

### 4.2 设置芯片驱动 symlink

STM32N6 芯片驱动位于本仓库 `arch/arm/stm32n6/`，需要 symlink 到 NuttX 构建树：

```bash
cd <openvela-workspace>
ln -s ../../../../contest2026_137_CtrlFuture/arch/arm/stm32n6/src \
      nuttx/arch/arm/src/stm32n6
ln -s ../../../../contest2026_137_CtrlFuture/arch/arm/stm32n6/include \
      nuttx/arch/arm/include/stm32n6
git -C nuttx apply contest2026_137_CtrlFuture/arch/arm/stm32n6/kconfig-stm32n6.patch
```

> CI 环境自动执行以上步骤，参见 `.github/workflows/build.yml`。

### 4.3 编译固件

```bash
cd <openvela-workspace>

# ---- STM32N6 真机固件 ----
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# ---- QEMU 仿真固件 ----
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu -j8

# 清理重新编译
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh distclean
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# 修改配置
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh menuconfig
```

### 4.4 QEMU 运行验证

> 注意：必须使用 `nsh-qemu` 配置编译，`nsh` 配置为真机固件，无法在 QEMU 运行。

```bash
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu -j8

prebuilts/qemu/linux-x86_64/bin/qemu-system-arm \
  -machine mps3-an547 -nographic -kernel nuttx/nuttx.bin

# 进入 NSH 后可执行：help, hello, ps
# Ctrl+A X — 退出 QEMU
```

## 五、当前进度

### 5.1 P0: 最小 NSH 启动（3/4 完成）

| 模块 | ADR | 状态 | 说明 |
|------|-----|------|------|
| CI 集成 | [001](docs/adr/ADR-001.md) | **DONE** | symlink + Kconfig patch，双目标 CI 绿灯 |
| 最小启动 | [002](docs/adr/ADR-002.md) | **DONE** | RCC/GPIO/USART1/IRQ/heap/start，8 个源文件 |
| SysTick | [003](docs/adr/ADR-003.md) | **DONE** | HSI 64MHz SysTick，调度器时钟源正常 |
| 真机验证 | [004](docs/adr/ADR-004.md) | PENDING | 待 STM32N6 开发板到手验证 |

### 5.2 工程基础设施

- [x] GitHub Actions CI（nsh + nsh-qemu 双目标编译 + QEMU 冒烟测试）
- [x] nxstyle 编码规范自动检查
- [x] commit message 格式校验（scope + Signed-off-by + 禁止 AI 标记/中文）
- [x] 本地 ci-check.sh 一键检查（11 项）
- [x] pre-commit / commit-msg hooks
- [x] 二进制体积守护 (< 1MB)
- [x] defconfig 一致性检查

### 5.3 项目文档

- [ROADMAP.md](docs/ROADMAP.md) — 5 阶段（P0-P4）+ 38 模块依赖图
- [FEATURE-LIST.md](docs/FEATURE-LIST.md) — 完整功能 checkbox 跟踪
- [CHANGELOG.md](CHANGELOG.md) — 版本变更记录
- [docs/adr/](docs/adr/) — 38 个架构决策记录，每个映射到 GitHub Issue

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

## 七、技术架构

### 芯片驱动注入方式

NuttX 上游尚无 STM32N6 系列完整支持。本仓库采用 out-of-tree 方式开发芯片驱动：

```
本仓库 arch/arm/stm32n6/src/  ──symlink──>  nuttx/arch/arm/src/stm32n6/
本仓库 arch/arm/stm32n6/include/ ─symlink─>  nuttx/arch/arm/include/stm32n6/
kconfig-stm32n6.patch ──git apply──> nuttx/arch/arm/src/Kconfig
```

CI 和本地开发均通过此方案实现零侵入编译。

### 当前时钟配置

- CPU 运行在 HSI 64MHz（PLL 尚未配置）
- SysTick 时钟源 = HSI 64MHz
- USART1 时钟 = APB2（HSI 64MHz）
- PLL 配置将在 P1 阶段（ADR-005）实现

## 八、Renode 测试基础设施

`tests/renode/` 下维护一套 STM32N647X0 的 Renode 平台描述
（`stm32n647x0.repl`）+ 30 个 C# 外设仿真模型（`peripherals/*.cs`）
+ 47 个 Robot Framework 测试套件（`tests/*.robot`，共 283 个测试
用例，全部通过），用于在没有真实开发板的情况下对驱动代码做**寄存器
级/驱动逻辑级**的回归验证。

采用 L1/L2/L3 分级（定义见 [tests/renode/README.md](tests/renode/README.md)）：

| 级别 | 含义 |
|------|------|
| L1-register | 基础寄存器可读写 |
| L2-state | 控制/状态位联动行为（如 CR.xxxON → SR.xxxRDY） |
| L3-functional | 端到端功能路径（如 SPI/I2C/DMA 真实字节搬运、NSH 命令输出） |
| boot-regression | 启动仍能到达 `nsh>` |

所有驱动的寄存器偏移/位域定义均已用 CMSIS 权威头文件
（`stm32n647xx.h`）交叉核对，FDCAN/SDMMC/XSPI/EMAC/RCC 等外设并用
apache/nuttx 官方 STM32N657 端口做二次交叉验证（详见
[tests/renode/README.md](tests/renode/README.md) 的
"CMSIS Alignment Campaign" 章节）。

**能力边界**：这套测试验证的是驱动代码与硬件寄存器手册的一致性、
以及状态位联动逻辑的正确性，**不能替代真机验证**——时钟频率、
总线时序、电压域、功耗等物理特性仍需 [ADR-004](docs/adr/ADR-004.md)
中列出的真机+示波器/逻辑分析仪流程。

运行方式：

```bash
bash contest2026_137_CtrlFuture/scripts/renode-test.sh
```

## 九、AI Coding 使用说明

> 完整对话日志见 `logs/` 目录（后续补充）。

---

原参赛仓库使用说明已保存至 [README.old](README.old)。
