# STM32N6 Driver + Renode Level 3 Model Co-Development Plan

## Context

P0 (ADR-001~003) 已完成，NuttX NSH 在 Renode 中成功启动。需要建立完整的开发-验证-回归闭环体系，按 ADR 顺序逐模块推进 P1~P4，驱动代码与 Renode 模型同步编写，保证每个功能可自动化验证且不随后续开发退化。

关键约束：不考虑时间效率，只追求准确性。采用 Level 3 严格自定义模型。

---

## 技术决策

### Renode 外设模型语言：C#（必选）

| 能力 | C# | Python |
|------|-----|--------|
| GPIO/中断信号生成 | 支持（IRQ.Set, Blink） | **不支持** |
| 寄存器集合（位域抽象） | DoubleWordRegisterCollection | 无 |
| 状态机框架 | IFlagRegisterField + callbacks | 无 |
| 时钟门控守卫 | 可实现 | 无法关联其他外设 |
| 运行时加载（无需重编译） | 不支持，需编译 Renode | 支持 |
| DMA/外设间 GPIO 连接 | INumberedGPIOOutput / IGPIOReceiver | 无 |

结论：Level 3 模型**必须用 C#**。Python 仅能做简单读写桩，无法满足中断、状态机、外设间信号的需求。代价是每次新增/修改模型需重编译 Renode（`build.sh --no-gui`），但这在本项目中可接受。

### 测试架构：双层验证

| 层 | 框架 | 位置 | 验证内容 |
|----|------|------|----------|
| 寄存器/模型 | Robot Framework (Renode) | `tests/renode/tests/*.robot` | 外设寄存器行为、状态转移、中断生成 |
| 驱动功能 | cmocka drivertest (NuttX app) | `app/drivertest/` | 实际数据收发、loopback、错误恢复 |

上游 openvela 已有 `apps/testing/drivers/drivertest_uart.c` 等 30+ 驱动测试（cmocka 框架），支持 UART 写/读/burst loopback。我们的 Robot Framework 测试通过在 Renode 中运行 drivertest 应用来验证端到端功能。

### CI 部署

```
本地开发 → 本地全套测试 → git push → CI 自动测试 → PR 审查
```

Renode CI 方案：
- 使用 Renode portable tar.gz（自带 dotnet runtime，无外部依赖）
- `renode-test` 脚本支持 headless 无 GUI 运行
- 自定义 C# 模型随 Renode 源码一起编译（CI 中 `build.sh --no-gui`）
- Robot Framework 测试输出标准 xUnit XML，集成 GitHub Actions artifact

### 文档存放策略

| 内容 | 位置 | 跟踪方式 |
|------|------|----------|
| 开发方法论（本文档） | `docs/DEV-METHODOLOGY.md` | git 跟踪，版本化 |
| AI 执行指令 | `CLAUDE.md` 中引用方法论文档 | .gitignore，本地 |
| ADR 文档 | `docs/adr/ADR-NNN.md` | git 跟踪 |
| Changelog | `docs/CHANGELOG.md` | git 跟踪 |

CLAUDE.md 新增一节引用：
```markdown
## Development Methodology
详见 `docs/DEV-METHODOLOGY.md`。每个 ADR 开发必须遵循其中定义的 8 阶段流水线。
```

---

## 完整工作流（ADR→代码→测试→PR→Issue 闭环）

### 单 ADR 完整生命周期

```
┌─────────────────────────────────────────────────────────────────────┐
│ 1. 开始：领取 ADR-NNN 对应的 GitHub Issue                            │
│    - 在 issue 中评论"开始开发"                                       │
│    - 本地创建 feature 分支（大功能）或直接在 develop（小改动）          │
├─────────────────────────────────────────────────────────────────────┤
│ 2. Phase A: Datasheet 提取（共同输入）                                │
│    - 从 stm32n647xx.h 提取 TypeDef + 位域                           │
│    - 从 HAL 源码提取初始化时序                                       │
│    - 从裸机示例确认典型使用方式                                       │
├─────────────────────────────────────────────────────────────────────┤
│ 3. Phase B+C: 双轨并行开发                                           │
│    - Track 1: NuttX 驱动（参考 stm32h7 + HAL）                       │
│    - Track 2: Renode C# 模型（参考 CMSIS + HAL，不看驱动）            │
├─────────────────────────────────────────────────────────────────────┤
│ 4. Phase D: 测试开发                                                 │
│    - Robot Framework: 寄存器级 + 集成级                               │
│    - cmocka drivertest: 功能验证应用（如需要）                        │
├─────────────────────────────────────────────────────────────────────┤
│ 5. Phase E: 本地验证                                                 │
│    - scripts/ci-check.sh (nxstyle + 编译 + QEMU)                    │
│    - 编译 Renode (build.sh --no-gui)                                │
│    - renode-test tests/renode/tests/ (全套回归)                      │
├─────────────────────────────────────────────────────────────────────┤
│ 6. Phase F: 提交                                                     │
│    - Commit 1: arch: add <periph> driver for STM32N6 (ADR-NNN)      │
│    - Commit 2: tests: add <periph> Renode model and tests (ADR-NNN) │
│    - 更新 docs/CHANGELOG.md                                         │
│    - 更新 docs/adr/ADR-NNN.md 状态 → DONE                           │
├─────────────────────────────────────────────────────────────────────┤
│ 7. Phase G: 推送 + PR                                                │
│    - git push openvela develop                                       │
│    - bash scripts/pr-create.sh                                       │
│    - CI 自动运行：style + build + renode-test + qemu-smoke           │
├─────────────────────────────────────────────────────────────────────┤
│ 8. Phase H: 关闭                                                     │
│    - PR 合入后，关闭对应 GitHub Issue                                 │
│    - Issue 评论中记录验证结果                                         │
│    - 如有真机验证，补充 MEASURED 结果                                  │
└─────────────────────────────────────────────────────────────────────┘
```

### Git 提交规范（与 CLAUDE.md 对齐）

```
<scope>: <imperative summary> (ADR-NNN)

<body: what + why>

Signed-off-by: changting27 <changting27@outlook.com>
```

Scope 映射：
- 驱动代码 → `arch:`
- Renode 模型/测试 → `tests:`
- 板级代码 → `board/contest_board:`
- 文档 → `docs:`
- CI 脚本 → `ci:` / `scripts:`

---

## 目录结构

```
contest2026_137_CtrlFuture/
├── arch/arm/stm32n6/
│   ├── src/
│   │   ├── hardware/                    # 寄存器头文件（每 ADR 新增）
│   │   │   ├── stm32_memorymap.h       (existing)
│   │   │   ├── stm32_rcc.h             (ADR-005)
│   │   │   ├── stm32_gpio.h            (ADR-006)
│   │   │   ├── stm32_pwr.h             (ADR-009)
│   │   │   ├── stm32_dma.h             (ADR-011)
│   │   │   └── stm32_<periph>.h
│   │   ├── stm32n6_rcc.c               (existing → ADR-005 扩展)
│   │   ├── stm32n6_gpio.c              (existing → ADR-006 扩展)
│   │   └── stm32n6_<periph>.c          (每 ADR 新增)
│   └── include/
├── tests/
│   └── renode/
│       ├── stm32n647x0.repl             # 平台定义（逐 ADR 替换桩→真实模型）
│       ├── peripherals/                  # Level 3 C# 模型
│       │   ├── STM32N6_RCC.cs
│       │   ├── STM32N6_PWR.cs
│       │   ├── STM32N6_GPDMA.cs
│       │   ├── STM32N6_EXTI.cs
│       │   ├── STM32N6_SPI.cs
│       │   ├── STM32N6_I2C.cs
│       │   ├── STM32N6_IWDG.cs
│       │   ├── STM32N6_RTC.cs
│       │   └── STM32N6_Peripherals.csproj
│       ├── tests/                        # Robot Framework 测试
│       │   ├── resources/
│       │   │   └── stm32n6-common.robot  # 共享 Setup/Teardown
│       │   ├── 000-boot-regression.robot # 永久回归锚（ADR-002/003）
│       │   ├── 005-rcc.robot
│       │   ├── 005-rcc-functional.robot  # 运行 NuttX 后验证时钟切换
│       │   ├── 006-gpio.robot
│       │   └── ...
│       └── run-tests.sh                  # 本地一键测试入口
├── app/
│   └── drivertest/                       # 驱动功能测试应用
│       ├── drivertest_uart.c             # UART 读/写/loopback
│       ├── drivertest_spi.c
│       ├── drivertest_i2c.c
│       ├── Makefile
│       └── Kconfig
├── docs/
│   ├── DEV-METHODOLOGY.md               # 本文档（git 跟踪）
│   ├── CHANGELOG.md                      # 版本变更记录
│   ├── ROADMAP.md                        (existing)
│   └── adr/ADR-NNN.md                    (existing)
├── scripts/
│   ├── ci-check.sh                       (existing)
│   ├── pr-create.sh                      (existing)
│   ├── qemu-smoke.sh                     (existing)
│   ├── renode-build.sh                   # 编译含自定义外设的 Renode
│   └── renode-test.sh                    # 运行全套 Robot 测试
└── .github/workflows/
    ├── build.yml                          (existing → 扩展 renode stage)
    └── renode-test.yml                    # 新增：Renode 回归测试
```

---

## CI 流水线（扩展后）

### `.github/workflows/renode-test.yml`

```yaml
# 触发：push to develop, PR to dev-ai-contest-2026
jobs:
  renode-regression:
    runs-on: ubuntu-22.04
    steps:
      - checkout（含 submodule renode 源码引用）
      - 安装 dotnet SDK 8.0
      - 编译 Renode（build.sh --no-gui，含 tests/renode/peripherals/）
      - 编译 NuttX nsh 固件
      - pip install robotframework
      - renode-test tests/renode/tests/ --results results/
      - upload-artifact: results/（Robot HTML 报告）
```

### 本地测试脚本 `scripts/renode-test.sh`

```bash
#!/bin/bash
# 编译 Renode（如有模型变更）
# 编译 NuttX nsh
# 运行全部 Robot 测试
# 输出 PASS/FAIL 摘要
```

---

## 测试完备性设计

### 每个 ADR 测试覆盖矩阵

| 维度 | Robot Framework (模型验证) | drivertest (功能验证) |
|------|---------------------------|----------------------|
| 寄存器复位值 | 启动前读所有寄存器，对比 datasheet | — |
| 位域读写 | 写入→回读，验证 RO/WO/RW/W1C 行为 | — |
| 状态转移 | enable→ready、start→busy→complete | — |
| 中断生成/清除 | 触发条件→IRQ assert，清除→IRQ deassert | — |
| 时钟门控 | 未使能时写入：模型记录警告，不产生副作用 | — |
| 初始化序列 | NuttX 启动后寄存器状态符合预期 | — |
| 数据传输 | — | UART: 发送/接收/loopback |
| 错误恢复 | — | 溢出/帧错误后恢复正常 |
| DMA 联动 | DMA 传输完成中断触发 | 数据正确性验证 |
| 回归 | 全部旧测试通过 | NSH 交互正常 |

### UART 功能测试示例（端到端）

```robot
*** Test Cases ***
UART Loopback Data Integrity
    Setup STM32N6
    # 连接 USART1 TX→RX (Renode 内部 loopback)
    Execute Command    connector Connect sysbus.usart1 sysbus.usart1
    Start Emulation
    Wait For Prompt On Uart    nsh>
    # 运行 drivertest 应用
    Write Line To Uart         drivertest -n 2 /dev/ttyS0
    Wait For Line On Uart      PASS    timeout=10
```

对应 NuttX 侧 `app/drivertest/drivertest_uart.c`（参考上游 `apps/testing/drivers/drivertest_uart.c`）：
- Case 0: 写入 "Hello" → UART TDR → 模型 TX
- Case 1: 模型 RX → UART RDR → 读取验证
- Case 2: Loopback burst — 发送 N 字节，接收 N 字节，逐字节比较

---

## Renode 模型开发流程（C# 详细）

### 1. 从 CMSIS 提取寄存器

```bash
# 辅助脚本：从 stm32n647xx.h 提取 TypeDef 偏移
grep -A200 "typedef struct" stm32n647xx.h | grep "__IO\|__I\|__O"
```

### 2. C# 模型模板

```csharp
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_RCC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_RCC(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x800;

        private void DefineRegisters()
        {
            // CR @ 0x0000: HSION→HSIRDY, HSEON→HSERDY, PLL1ON→PLL1RDY
            Registers.CR.Define(this, 0x00000005) // reset value: HSION=1, HSIRDY=1
                .WithFlag(0, out hsion, name: "HSION")
                .WithReservedBits(1, 1)
                .WithFlag(2, FieldMode.Read,
                    valueProviderCallback: _ => hsion.Value, name: "HSIRDY")
                // ...
                .WithFlag(8, out hseon, name: "HSEON",
                    writeCallback: (_, val) => { if(val) UpdateClocks(); })
                .WithFlag(10, FieldMode.Read,
                    valueProviderCallback: _ => hseon.Value, name: "HSERDY")
                // PLL1
                .WithFlag(24, out pll1on, name: "PLL1ON")
                .WithFlag(25, FieldMode.Read,
                    valueProviderCallback: _ => pll1on.Value, name: "PLL1RDY");
        }

        private enum Registers : long
        {
            CR = 0x0000,
            // ... 全部来自 CMSIS TypeDef
        }
    }
}
```

### 3. 编译集成

推荐方式：将 C# 文件放入 Renode 源码树的自定义目录：
```
renode/src/Infrastructure/src/Emulator/Peripherals/Peripherals/Miscellaneous/STM32N6_RCC.cs
```
然后整体 `build.sh`。CI 中只需 clone renode + 复制 .cs 文件 + build。

---

## 回归防护

| 机制 | 说明 |
|------|------|
| `000-boot-regression.robot` | 永远第一个执行，验证 NSH 启动 + help + ps |
| 时钟门控守卫 | 每个 L3 模型：未使能时访问记录 Warning，返回 0 |
| .repl 增量演进 | 只将 Tag/MappedMemory 替换为真实模型，不删除 |
| 分离提交 | 驱动/模型/测试可独立回滚 |
| CI 全套回归 | 每次 push 跑全部 Robot 测试 + QEMU 冒烟 |
| 版本标记 | .repl 顶部注释列出已集成 ADR |
| defconfig 锁定 | `scripts/ci-check.sh` 检查 defconfig 一致性 |

---

## Renode Fidelity Matrix (Phase-1)

完整 L1/L2/L3 保真度表、Robot suite ↔ ADR 映射、路径布局以
[`tests/renode/README.md`](../tests/renode/README.md) 为**唯一权威源**。
本文只摘要；更新矩阵时改 README，不必批量改 ADR 正文。

### 保真度层级

| 层级 | 含义 | 典型证据 |
|------|------|----------|
| L1 | 寄存器可读 / 复位值 | `L1-register` Robot cases |
| L2 | 控制/状态位与状态机 | enable→ready、soft IRQ、FIFO 标志 |
| L3 | 功能路径 | mem2mem / SPI loopback / SDMMC probe / UART console |

### 路径布局（勿写错 RENODE_SRC）

| 变量 | 路径 | 说明 |
|------|------|------|
| `OPENVELA_ROOT` | `/home/takumi/mi/open-velao-contest` | 顶层 |
| `WORKSPACE` | `$OPENVELA_ROOT/ctrl_future` | NuttX 构建根 |
| `RENODE_SRC` | `$OPENVELA_ROOT/renode` | Renode 源码（**不是** `$WORKSPACE/renode`） |
| `SOFTWARE_PACKAGE` | `$OPENVELA_ROOT/SoftwarePackage` | ST HAL / CMSIS |
| CMSIS | `$SOFTWARE_PACKAGE/STM32Cube_FW_N6_V1.0.0/.../stm32n647xx.h` | 寄存器真源 |
| 本仓 Renode 树 | `tests/renode/` | `.repl` / `peripherals/*.cs` / `tests/*.robot` |

### Robot 编号 ≠ ADR 编号

Robot 文件名前缀是历史落地顺序，**不等于** ROADMAP ADR 号。
示例：`021-emac`→ADR-022，`023-fdcan`→ADR-024，`024-otg`→ADR-023，
`025-ltdc`→ADR-031，`026-dcmipp`→ADR-033，`039-rng`→ADR-037。
**禁止**为对齐编号而批量重命名 suite。完整对照表见 README。

### Phase-1 波次结果（摘要）

- Wave 0–3 已将驱动相关外设模型抬到可用 L2/L3（见 README 表）。
- L3 已达：USART1（stock）、GPDMA mem2mem、SPI FIFO loopback、
  I2C master path、SDMMC probe/CMD。
- L2+ 覆盖：RCC、PWR、EXTI、IWDG、RTC、XSPI、EMAC、SAI、FDCAN、
  OTG、LTDC、DCMIPP、RNG 等。
- 仍为 L1（本波未抬升）：HPDMA、TIM、LPTIM、ADC、DTS、DMA2D、CSI、
  VENC、NPU、CRYP、OTP。
- Phase-2 cmocka drivertest e2e 延后，理由见 README。

---

## CLAUDE.md 新增内容（工作流串联）

在 CLAUDE.md 中追加：

```markdown
## ADR Development Workflow

每个 ADR 的开发必须遵循 `docs/DEV-METHODOLOGY.md` 中定义的 8 阶段流水线。

关键检查点：
1. 开始前：确认 GitHub Issue 存在且状态为 Open
2. 开发中：双轨并行（驱动 + Renode 模型），互不参考
3. 提交前：本地 `scripts/ci-check.sh` + `scripts/renode-test.sh` 全部通过
4. 提交时：分离 commit（驱动 / 模型+测试 / 文档），标题含 (ADR-NNN)
5. 提交后：更新 CHANGELOG.md + ADR 状态
6. PR 合入后：关闭对应 Issue，评论验证结果

## Renode Custom Build

自定义外设模型位于 `tests/renode/peripherals/`，需编译进 Renode：

    # 复制模型到 Renode 源码
    cp tests/renode/peripherals/*.cs \
      $RENODE_SRC/src/Infrastructure/src/Emulator/Peripherals/Peripherals/Miscellaneous/
    # 编译
    cd $RENODE_SRC && ./build.sh --no-gui

## Test Commands

    # 本地全套验证（提交前必须通过）
    bash scripts/ci-check.sh           # nxstyle + 编译 + QEMU
    bash scripts/renode-test.sh        # Renode 全套回归

    # 单个 ADR 测试
    renode-test tests/renode/tests/005-rcc.robot
```

---

## 实施顺序

| 优先级 | 任务 | 产出 |
|--------|------|------|
| 0 | 建立测试基础设施 | Robot 框架 + renode-test.sh + CI workflow |
| 1 | ADR-004: boot 回归锚 | 000-boot-regression.robot |
| 2 | ADR-005: RCC 时钟树 | STM32N6_RCC.cs + 驱动扩展 + 测试 |
| 3 | ADR-009: PWR | STM32N6_PWR.cs + 驱动 + 测试（RCC 依赖 PWR VOS） |
| 4 | ADR-006: GPIO | 验证 STM32_GPIOPort 兼容性 + 全端口 .repl |
| 5 | ADR-007: Cache | SCB 寄存器桩验证 |
| 6 | ADR-008: TCM | ITCM/DTCM MappedMemory + 链接脚本 |
| 7 | ADR-010: EXTI | STM32N6_EXTI.cs + GPIO→NVIC 链路 |
| 8 | ADR-011: GPDMA | STM32N6_GPDMA.cs（最复杂） |
| 9+ | ADR-012~017 | 按依赖顺序 |

---

## 关键参考文件

| 用途 | 路径 |
|------|------|
| 寄存器定义源 | `SoftwarePackage/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include/stm32n647xx.h` |
| HAL 行为参考 | `SoftwarePackage/STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Src/` |
| 裸机示例 | `SoftwarePackage/Projects/` |
| NuttX H7 模板 | `nuttx/arch/arm/src/stm32h7/` |
| Renode RCC 模型参考 | `renode/src/.../Peripherals/Miscellaneous/STM32H7_RCC.cs` |
| Renode SPI 状态机参考 | `renode/src/.../Peripherals/SPI/STM32H7_SPI.cs` |
| Renode DMA 参考 | `renode/src/.../Peripherals/DMA/STM32DMA.cs` |
| 上游 UART 测试 | `apps/testing/drivers/drivertest_uart.c` |
| Robot 测试模板 | `renode/tests/platforms/nucleo_wba52cg.robot` |
