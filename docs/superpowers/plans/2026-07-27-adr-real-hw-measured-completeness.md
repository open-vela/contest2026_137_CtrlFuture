# ADR 真机 MEASURED 完整性回填计划

> 制定日期: 2026-07-27
> 前置事件: ADR-002/003/004 启动链已在真机 STM32N647 以 MEASURED 验证
> （见 `scripts/hw-verify.sh`、memory `project_stm32n6-dev-boot-launch`）。
> 目标: 每个 ADR 的功能点 100% 实现 + 100% 真机验证，消除所有遗留功能点，
> 且像 Renode 回归一样，真机测试用例可重复执行、防止功能退化。

---

## 0. 盘点结论（对照真实源码，非 ADR 文档状态字段）

盘点方法: 读全部 38 个 ADR + 逐一核对 `arch/arm/stm32n6/src/*.c` 实际内容
（`putreg32/getreg32` 计数、TODO/stub 计数、文件是否存在）+ 核对真机
`configs/nsh/defconfig` 实际启用项 + `board_bringup.c` 实际注册项。

### 0.1 关键事实（决定计划形态）

1. **真机 `nsh` defconfig 仅启用 `USART1 + RNG + MPU`**。
   其余"已实现"驱动全部未在真机固件中启用。
2. **`board_bringup.c` 真机路径只挂载 procfs/tmpfs**，不调用任何
   `stm32n6_xxx_initialize()`。→ 驱动代码存在但从未被系统实例化，
   这正是它们从未 MEASURED 的根因，也是核心"遗留功能点"。
3. **NuttX cmocka `drivertest` 框架在本 workspace 可达**
   （`apps/testing/cmocka` + `apps/testing/drivers/drivertest`），
   现成二进制覆盖: `uart / spidev_master / i2cdev_master / watchdog /
   rtc / adc / pwm / timer / block / framebuffer / gpio` 等。
   → 满足"参考 nuttx 官方框架"的要求；Renode README 曾把它列为
   Phase-2 deferred（理由: contest 隔离 + <1MB CI 体积），真机 SRAM
   4.2MB 已解除体积约束，隔离问题用独立 test defconfig 解决。
4. **ADR 文档与源码不一致（需纠偏）**:
   - ADR-013 SPI 文档写"暂不实现"，源码实为 409 行、15 处寄存器操作，已实现。
   - ADR-024 FDCAN 文档写"跳过实现"，源码实为 434 行、17 处寄存器操作，已实现。

### 0.2 三类 ADR 分层

**A 类 — 已实现有真实寄存器代码，仅 Renode 仿真验证，未真机 MEASURED（17 项）**

| ADR | 模块 | 源码行/寄存器操作 | 遗留缺口 |
|-----|------|------------------|----------|
| 005 | RCC | 有 / 多 | PLL2-4、LSI/LSE 延后；未测真机 PLL 频率 |
| 006 | GPIO | 有 | 仅当前需要的 ~20 引脚 AF |
| 007 | Cache | 有 | 未测真机性能 |
| 008 | TCM | 有 | 未测真机性能 |
| 009 | PWR | 有 / 8 | 低功耗模式延后；未测真机电压 |
| 010 | EXTI | 有 / 22 | — |
| 011 | GPDMA | 有 / 10 | — |
| 012 | 多 UART | 有 | USART2/3、UART4-8 未启用 |
| 013 | SPI | 409 / 15 | 文档 stale；未接真机外设 |
| 014 | I2C | 464 / 15 | I2C2/3 未用 |
| 015 | IWDG | 224 / 10 | 无 WWDG |
| 016 | RTC | 411 / 24 | 无闹钟中断、无 VBAT 掉电保持 |
| 017 | MPU | 有 | 仅 QEMU 验证 |
| 018/019 | XSPI | 364 / 14 | 无 XIP boot 冷启动 |
| 020 | SDMMC | 551 / 26 | 无热插拔 |
| 022 | Ethernet | 396 / 20 | 未测真机 PHY/ping |
| 023 | OTG | 有 | 决策低优先级 |
| 024 | FDCAN | 434 / 17 | 文档 stale |

**B 类 — 已真机 MEASURED（本 session 完成）**: ADR-002/003/004 启动 + USART1 控制台。

**C 类 — 真正未实现（.c 缺失或仅框架桩）**

| ADR | 模块 | 状态 | 决策处置（本计划已确认全部实现+真机验证） |
|-----|------|------|------|
| 021 | FMC | .c 缺失 | 实现（reinstate） |
| 025 | HPDMA | .c 缺失 | 实现（reinstate） |
| 026 | 高级 TIM | .c 缺失 | 实现（reinstate） |
| 027 | 通用 TIM | .c 缺失 | 实现（reinstate） |
| 028 | LPTIM | .c 缺失 | 实现（reinstate） |
| 029 | ADC | .c 缺失 | 实现（reinstate） |
| 030 | DTS | .c 缺失 | 实现 |
| 031 | LTDC | 仅 /dev/fb0 框架, 0 寄存器, 3 TODO | 实现寄存器编程 |
| 032 | DMA2D | .c 缺失 | 实现 |
| 033 | DCMIPP | 仅 /dev/video0 框架, 0 寄存器 | 实现寄存器编程 |
| 034 | CSI | .c 缺失 | 实现 |
| 035 | H.264 VENC | .c 缺失 | 实现 |
| 036 | NPU | 芯片侧 .c 缺失, app 侧 stub | 实现 |
| 037 | 加密 | 仅 RNG 完成 | 实现 HASH/AES |
| 038 | Secure Boot | .c 缺失 | 实现 |

---

## 1. 范围与顺序（已与用户确认）

- **重心**: A 类真机闭环回填优先 → 重启 N/A ADR 实现 → C 类主线驱动。
- **N/A ADR**（021/025/026/027/028/029）: 推翻先前裁剪决策，全部实现 + 真机验证。
- **测试形式**: NuttX cmocka `drivertest` 框架（真机运行断言 PASS）。
- **总目标**: 全部 38 ADR 达到功能 100% 实现 + 真机 MEASURED，无遗留功能点。

执行顺序: Phase 0 → Phase 1(A 类) → Phase 2(重启 N/A) → Phase 3(C 类主线)
→ Phase 4(C 类重型)。从第一个待办 ADR（ADR-005）开始。

---

## 2. Phase 0 — 测试基础设施 + 驱动接线地基（阻塞后续全部）

没有这步，A 类驱动无法被真机 MEASURED（当前它们根本没被 init）。

**0.1 真机测试 defconfig**
- 新增 `board/contest_board/configs/nsh-test/defconfig`（或在 `nsh` 上叠加），
  启用 `CONFIG_TESTING_CMOCKA`、`CONFIG_TESTING_DRIVER_TEST` 及分项
  （`TESTING_DRIVER_GPIO` 等），保留 USART1 控制台不变。
- 验证 test 固件仍 < SRAM 预算，QEMU/Renode 回归不受影响（test config 独立）。

**0.2 驱动接线**
- 在 `board_bringup.c` 为每个 A 类驱动加 Kconfig 门控的 `_initialize()` 调用
  （SPI/I2C/DMA/RTC/IWDG/Ethernet/SDMMC/XSPI/EXTI...），注册 `/dev/*` 节点。
- 对称补 `_uninitialize()`（遵循 CLAUDE.md 嵌入式规则 4）。

**0.3 真机测试驱动器扩展**
- 扩展 `scripts/hw-verify.sh`（本地 gitignore）: 除 ABCD/NSH/hello 断言外，
  在 NSH 下逐个运行 `drivertest_*` 二进制，grep `PASS` / 无 `FAIL`。
- 形成"真机回归套件"清单，像 Renode Robot 一样可重复跑、防退化。

**0.4 ADR 文档纠偏**
- ADR-013/024 状态与源码对齐（删除"暂不实现"表述，标注真实已实现）。
- ROADMAP「验证」列区分「目标级别」与「已达级别」，真机达标后更新。

**Phase 0 退出标准**: nsh-test 固件编译通过 + 真机能跑起至少 1 个
`drivertest_simple` PASS + 至少 1 个真实驱动（如 RTC）init 成功。

---

## 3. Phase 1 — A 类真机 MEASURED 回填（ADR-005..024）

每个 ADR 遵循 CLAUDE.md「Per-ADR Closed-Loop Lifecycle」，但因代码已存在，
重心在 **接线 + 真机测试用例 + MEASURED 验证**，而非重写驱动。

### 组 1A — 有现成 cmocka 二进制（最快）
| ADR | 现成测试二进制 | 真机断言要点 |
|-----|---------------|-------------|
| 012 多 UART | `drivertest_uart` | 环回收发字节一致 |
| 013 SPI | `drivertest_spidev_master` | TXDR→RXDR 环回 |
| 014 I2C | `drivertest_i2cdev_master` | 扫描/读写从设备 |
| 015 IWDG | `drivertest_watchdog` | 超时复位 / 喂狗不复位 |
| 016 RTC | `drivertest_rtc` | 读写时间递增 |

### 组 1B — NSH 命令或轻量自写断言
| ADR | 真机验证方式 |
|-----|-------------|
| 005 RCC | NSH 下读时钟 API / MCO 输出；`date` 依赖时钟正确 |
| 006 GPIO | `drivertest_gpio`（输出/输入/中断回环） |
| 007 Cache | 启动即启用，boot 到 nsh> 即证明不崩；DMA 一致性由 011 覆盖 |
| 008 TCM | 读写 ITCM/DTCM 地址；对比热路径耗时 |
| 009 PWR | Voltage Scale 切换后系统稳定；boot 正常 |
| 010 EXTI | 软触发 EXTI line，ISR 命中计数 |
| 011 GPDMA | mem2mem 拷贝 + DCACHE 一致性断言 |
| 017 MPU | 触发越界访问 → MemManage 异常捕获 |

### 组 1C — 存储/通信（需外设条件，真机环境已具备板载资源时验证）
| ADR | 真机验证方式 |
|-----|-------------|
| 018/019 XSPI | Flash ID 读取 + 读写一致；XIP boot 冷启动（019） |
| 020 SDMMC | `drivertest_block` + mount FAT + 读写文件 |
| 022 Ethernet | `ifconfig` + `ping` 网关 + TCP 连接 |
| 023 OTG | CDC 枚举（低优先级，可后置） |
| 024 FDCAN | 环回帧收发（需 CAN 收发器，无则记 BUILD+仿真+接线就绪） |

**遗留功能点补齐（本 Phase 内一并完成，达成 100% 功能）**:
- 016 RTC: 补闹钟中断路径（无 VBAT 硬件时 VBAT 保持记为环境限制）。
- 015: 若板级需要则补 WWDG；否则 ADR 明确标注仅 IWDG（决策裁剪，非遗留）。
- 012: 按需启用 USART2/3、UART4-8 并纳入 `drivertest_uart` 多实例。
- 005: 推进 PLL1 上真机（示波器/MCO 测频），PLL2-4/LSE 按依赖模块需要补。

---

## 4. Phase 2 — 重启先前裁剪的 N/A ADR（021/025/026/027/028/029）

这些 .c 文件缺失，需从零实现（参照 `stm32h7/` 同名驱动 + CMSIS
`stm32n647xx.h`），再接线 + 真机测试。

| ADR | 新增文件 | 测试二进制 |
|-----|---------|-----------|
| 026 高级 TIM | `stm32n6_tim.c/.h` | `drivertest_pwm` / `drivertest_timer` |
| 027 通用 TIM | 扩展 `stm32n6_tim.c` | `drivertest_timer`（周期中断） |
| 028 LPTIM | `stm32n6_lptim.c/.h` | `drivertest_oneshot` / 自写 |
| 029 ADC | `stm32n6_adc.c/.h` | `drivertest_adc` |
| 021 FMC | `stm32n6_fmc.c/.h` | 外部 SRAM 读写（需硬件）；无则接线就绪+仿真 |
| 025 HPDMA | `stm32n6_hpdma.c/.h` | mem2mem（复用 GPDMA drivertest 思路） |

> 每个新驱动先过 driver-code-reviewer（6 维检查表），零 FAIL 后再接线。
> Renode 侧已有对应 C# 模型（TIM/LPTIM/ADC/HPDMA），本 Phase 使其从
> L1 → 有真实 NuttX 驱动消费，可升级 Robot 用例至 L2/L3 防退化。

---

## 5. Phase 3 — C 类主线缺失驱动（EdgeSight 核心路径）

| ADR | 工作 | 测试 |
|-----|------|------|
| 030 DTS | 新写 `stm32n6_dts.c`，接 `/dev/temp0` | 读结温合理值(20-40°C) |
| 037 HASH/AES | 补 `stm32n6_hash.c` / `stm32n6_aes.c`（RNG 已完成） | SHA-256 向量 / AES 向量 |
| 031 LTDC | 实现 3 处 TODO 寄存器编程（时序 + Layer 帧缓冲地址） | `drivertest_framebuffer` + 目视纯色/渐变 |
| 033 DCMIPP | 打通 `CMW_CAMERA_*` 硬件调用 + REQBUFS + 帧中断 | 采一帧，校验非全零 |
| 032 DMA2D | 新写 `stm32n6_dma2d.c` | 矩形填充/混合结果校验 |

---

## 6. Phase 4 — C 类重型 P4（工作量数周，末位）

| ADR | 说明 |
|-----|------|
| 034 CSI-2 | D-PHY + lane 配置，喂给 DCMIPP |
| 035 H.264 VENC | 编码输出有效 NAL；Renode 模型现为占位 |
| 036 NPU | 芯片侧 `stm32n6_npu.c` + 替换 app 侧 stub；模型权重 XIP 加载 |
| 038 Secure Boot | 签名验证 + OTP 根密钥；需真机 OTP 谨慎操作 |

> 036/034/035 需真机摄像头 + 模型权重外部条件；038 涉及不可逆 OTP，
> 真机验证前必须与用户确认（安全护栏）。

---

## 7. 每 ADR 通用闭环（防退化，对齐 Renode 回归精神）

1. （C 类）写驱动 → driver-code-reviewer 零 FAIL。
2. 接线进 `board_bringup.c` + test defconfig 启用。
3. 本地 E1: `ci-check.sh`（nxstyle + 双目标编译）+ `renode-test.sh` 全绿。
4. 本地 E2 真机 MEASURED: `hw-verify.sh` 跑对应 `drivertest_*` 断言 PASS。
5. 新真机测试用例加入"真机回归套件"清单（每次改动重跑，防功能退化）。
6. commit（`-s`、英文、无 AI 标记）→ merge develop → 更新 ADR/ROADMAP 状态。
7. 同步上游 → push → PR（`pr-create.sh`）→ 后台 Monitor CI → 更新 issue。

---

## 8. 风险与依赖

- **外设硬件条件**: SPI/I2C/CAN/Camera 等的真机 L3 断言依赖板载/外器件；
  无器件时该 ADR 记为"接线就绪 + 仿真 L3 + BUILD"，并在 ROADMAP 明确标注
  受限原因（不谎报 MEASURED）。
- **contest 隔离**: 启用 cmocka/drivertest 若触及 apps/nuttx 公共修改，
  需单独 PR 到 `dev-ai-contest-2026`（CLAUDE.md contest 规则）。优先用
  独立 test defconfig 把改动限制在本仓库边界内。
- **两跳 SSH 真机链路**: 沿用 `hw-verify.sh` 已验证的 MSYS 三陷阱规避
  （见 memory `reference_remote-hw-debug`）。
- **038 OTP 不可逆**: 真机执行前强制用户确认。
- **ABCD 调试打印**: 保留至全部驱动调试完成，听用户命令再删。

---

## 9. 立即起步（Phase 0 首个动作）

从 ADR-005 之前的地基做起:
1. 建 `configs/nsh-test/defconfig`，启用 cmocka + drivertest_simple。
2. 真机验证 `drivertest_simple` PASS（打通"真机能跑 cmocka"这一根本前提）。
3. 接线 RTC（最简、有现成 `drivertest_rtc`）作为第一个真机 MEASURED 回填样例。
4. 跑通后固化进 `hw-verify.sh` 真机回归套件，再横向铺开组 1A。
