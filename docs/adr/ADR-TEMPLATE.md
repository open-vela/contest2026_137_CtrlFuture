# ADR-NNN: <标题>

## 状态

<!-- 合法值: Proposed | Accepted | Deprecated -->
<!-- Proposed → Accepted: 实施完成并通过验证后 -->
<!-- Accepted → Deprecated: 被新 ADR 替代或功能移除时 -->

Proposed

## Phase

P?

## 目标

> GitHub Issue: #NN
> 验证级别: BUILD / QEMU / MEASURED

<!-- 验证级别定义:
  BUILD    — 编译通过 + nxstyle + 二进制体积 < 1MB
  QEMU     — 在 MPS3-AN547 上功能验证 (nsh-qemu 固件)
  MEASURED — 真机测量确认行为正确 (nsh 固件 + STM32N6 开发板)
-->

一句话说明做完后系统获得什么能力。

## 上下文

为什么需要这个模块，面临什么问题。引用 datasheet 章节。

## 决策

选了什么方案。

## 设计

<!-- 本节描述模块的架构设计，帮助后续开发者理解"代码为什么这样组织"。-->

### 模块架构

描述模块内部结构、关键数据结构、状态机（如有）。
可用 ASCII 图或表格说明组件关系。

### NuttX 框架对接

说明本模块如何融入 NuttX 的调用链：
- 入口函数（被谁调用，在启动链中的位置）
- 注册机制（driver_register / boardctl / irq_attach 等）
- 回调 / 中断处理流程

### 数据流

输入 → 处理 → 输出 的关键路径（可用 ASCII 图）。

## 实施步骤

1. [ ] 步骤一（预期产出）
2. [ ] 步骤二
3. [ ] ...

## 实现说明

<!-- 本节记录代码层面的关键决策，是 ADR 作为学习文件的核心内容。-->

### 关键寄存器配置

列出最重要的寄存器配置及原因（不是全部寄存器，
是"为什么这样写"的那几个关键位）。

### 代码模式

说明使用的 NuttX 代码模式（如 enter_critical_section /
sem_wait / work_queue 等），为什么选这个模式。

### 踩坑与解决

实施过程中遇到的问题及解决方案。
这一条对后续开发者价值最高，务必记录。

## 验证标准

- [ ] `ci-check.sh` 编译通过（BUILD）
- [ ] QEMU 功能测试通过（QEMU，如适用）
- [ ] 真机测量确认行为正确（MEASURED）
- 本模块特定的验收测试

## 理由

为什么选择此方案而非替代方案。权衡分析。

## 依赖

- **被阻塞**: ADR-NNN（原因）
- **阻塞**: ADR-NNN（原因）

## 文件

<!-- 标注每个文件是 [新增] 还是 [修改] -->

- `arch/arm/stm32n6/src/stm32n6_xxx.c` — [新增] 驱动实现
- `arch/arm/stm32n6/src/stm32n6_xxx.h` — [新增] 头文件
- `board/contest_board/src/stm32n6_xxx.c` — [修改] 板级注册（如需要）

## 参考

- DS14791 Section X.Y (page range)
- NuttX 参考: `arch/arm/src/stm32h7/stm32_xxx.c`

## 变更历史

<!-- 记录 ADR 状态变更的时间点和关联 commit -->

- YYYY-MM-DD: Proposed
