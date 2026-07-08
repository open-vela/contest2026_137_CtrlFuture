# ADR-NNN: <标题>

## 状态

Proposed

## Phase

P?

## 目标

> GitHub Issue: #NN
> 验证级别: BUILD / QEMU / MEASURED

一句话说明做完后系统获得什么能力。

## 上下文

为什么需要这个模块，面临什么问题。引用 datasheet 章节。

## 决策

选了什么方案。

## 实施步骤

1. [ ] 步骤一（预期产出）
2. [ ] 步骤二
3. [ ] ...

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

- `arch/stm32n6/src/stm32n6_xxx.c` — 驱动实现
- `arch/stm32n6/src/stm32n6_xxx.h` — 头文件
- `board/contest_board/src/stm32n6_xxx.c` — 板级注册（如需要）

## 参考

- DS14791 Section X.Y (page range)
- NuttX 参考: `arch/arm/src/stm32h7/stm32_xxx.c`
