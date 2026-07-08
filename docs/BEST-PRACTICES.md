# CtrlFuture 开发最佳实践

> STM32N647X0 (Cortex-M55) openvela/NuttX 板级适配项目工程规范
> 2026 首届 openvela AI 硬件开发者大赛 · 队伍 #137

---

## 一、编码规范（NuttX nxstyle）

### 1.1 核心规则

| 规则 | 要求 |
|------|------|
| 行宽 | **78 列**（不是 80） |
| 缩进 | 2 空格，禁止 tab |
| 花括号 | Allman 风格（独占一行，缩进 2 格） |
| 指针星号 | `char *p`（靠变量） |
| 命名 | 全局函数 `board_` / `stm32_` 前缀，宏全大写 |
| 文件路径注释 | 第 2 行写 NuttX 逻辑路径 `boards/arm/stm32n6/contest2026_137_board/...` |
| License | 每个文件第一行 `SPDX-License-Identifier: Apache-2.0` + ASF header |
| Section 注释 | 固定格式：`Included Files` / `Pre-processor Definitions` / `Public Functions` |
| 函数注释 | 必须有 `Name:` + `Description:` 块 |
| 执行权限 | `.c/.h` 文件禁止 +x |

### 1.2 条件格式示例

```c
if (ret < 0)
  {
    syslog(LOG_ERR, "ERROR: something failed: %d\n", ret);
    return ret;
  }
```

### 1.3 工具链

```bash
# 编译 nxstyle（只需一次）
cd /home/takumi/mi/open-velao-contest/ctrl_future
make -C nuttx/tools -f Makefile.host nxstyle

# 检查单个文件
nuttx/tools/nxstyle <file.c>

# 自动格式化
uncrustify -c nuttx/tools/uncrustify.cfg --replace --no-backup <file.c>

# 综合检查（nxstyle + codespell + license）
nuttx/tools/checkpatch.sh -f <file.c>
```

---

## 二、自动化守护

### 2.1 守护分层

| 层级 | 时机 | 手段 | 守护内容 |
|------|------|------|----------|
| 编译期 | 每次 build | gcc + ld | 类型安全、链接错误 |
| 静态分析 | commit 前 | nxstyle + cppcheck | 编码规范、潜在 bug |
| 运行时断言 | 代码内 | DEBUGASSERT | 时序、状态机、硬件就绪 |
| 自动化测试 | 本地脚本 | QEMU + expect | 功能正确、启动成功 |
| AI Review | PR 阶段 | Claude Code | 设计合理性、遗漏场景 |
| 真机验证 | 里程碑 | 示波器 + 手工 | 时钟、电气、功耗 |

### 2.2 一键检查脚本

每次 commit 前运行 `scripts/ci-check.sh`，覆盖：

1. 双目标编译通过（nsh + nsh-qemu）
2. 二进制体积 < 阈值
3. QEMU 冒烟测试（NSH 启动成功）
4. defconfig 一致性（无意外漂移）
5. nxstyle 编码规范
6. 内存使用报告

### 2.3 Pre-commit 流程

```
编写代码
  │
  ▼
uncrustify 自动格式化 ← 处理缩进、花括号、空格
  │
  ▼
nxstyle 检查 ← 行宽、命名、section 结构
  │
  ▼
scripts/ci-check.sh ← 编译 + 体积 + QEMU 冒烟
  │
  ▼
AI Review（复杂变更时） ← 寄存器安全、时序、设计
  │
  ▼
git commit -s
```

---

## 三、嵌入式编程规范

### 3.1 寄存器操作防御性编程

```c
/* 读-改-写必须在临界区内 */
static inline void reg_set_bits(uintptr_t reg, uint32_t mask)
{
  irqstate_t flags = enter_critical_section();
  putreg32(getreg32(reg) | mask, reg);
  leave_critical_section(flags);
}

/* 硬件等待必须有超时，永远不能死等 */
static int wait_flag(uintptr_t reg, uint32_t flag,
                     uint32_t timeout_us)
{
  uint32_t start = up_perf_gettime();
  while (!(getreg32(reg) & flag))
    {
      if (up_perf_gettime() - start > timeout_us)
        {
          return -ETIMEDOUT;
        }
    }

  return OK;
}
```

### 3.2 启动时序守护

```c
void stm32_board_initialize(void)
{
  /* 电源必须在时钟切换前稳定 */

  stm32_pwr_smps_enable();
  DEBUGASSERT(stm32_pwr_smps_ready());

  /* PLL lock 后才能切 CPU 时钟源 */

  stm32_rcc_pll1_enable();
  DEBUGASSERT(stm32_rcc_pll1_locked());

  stm32_rcc_switch_cpuclk(RCC_CPUSW_IC1);
}
```

**原则：** 用 `DEBUGASSERT` 把时序依赖变成 loud failure。`CONFIG_DEBUG_ASSERTIONS=y` 已开启。

### 3.3 init/uninit 对称性

每个 `xxx_initialize()` 必须有对应的 `xxx_uninitialize()`：

```c
int stm32_usart1_initialize(void);
void stm32_usart1_uninitialize(void);
```

即使当前不用低功耗模式，也预留接口。

### 3.4 文件拆分规范

```
board/contest_board/src/
├── board_bringup.c        # 入口编排（不碰寄存器）
├── stm32n6_clockconfig.c  # RCC + PLL
├── stm32n6_power.c        # SMPS + 电压域
├── stm32n6_serial.c       # USART 引脚/中断
├── stm32n6_gpio.c         # GPIO AF 映射表
└── stm32n6_npu.c          # Neural-ART NPU（后续）
```

`board_bringup.c` 是"指挥"，其他文件是"执行者"。

### 3.5 DMA/Cache 一致性

```c
/* DMA 发送前：clean cache */
up_clean_dcache((uintptr_t)buf,
                (uintptr_t)buf + len);
dma_start_tx(buf, len);

/* DMA 接收后：invalidate cache */
dma_wait_rx_complete();
up_invalidate_dcache((uintptr_t)buf,
                     (uintptr_t)buf + len);
```

### 3.6 外设状态机规范

```c
enum npu_state_e
{
  NPU_STATE_OFF,
  NPU_STATE_LOADING,
  NPU_STATE_READY,
  NPU_STATE_INFERRING,
  NPU_STATE_ERROR,
};
```

非法状态转移用 `DEBUGASSERT` 拦截。

### 3.7 引脚分配集中管理

```c
/* board/contest_board/include/board_pinmap.h
 * 所有引脚分配集中于此，防止冲突
 */

#define BOARD_PIN_USART1_TX  (GPIO_PORTE | GPIO_PIN5 | GPIO_AF7)
#define BOARD_PIN_USART1_RX  (GPIO_PORTE | GPIO_PIN6 | GPIO_AF7)
#define BOARD_PIN_SPI1_SCK   (GPIO_PORTA | GPIO_PIN5 | GPIO_AF5)
```

### 3.8 时钟树运行时自检

```c
void board_verify_clocks(void)
{
  uint32_t cycles_per_ms = up_perf_getfreq() / 1000;

  /* CPU @ 200MHz → 1ms ≈ 200000 cycles */

  DEBUGASSERT(cycles_per_ms > 180000 &&
              cycles_per_ms < 220000);

  syslog(LOG_INFO, "CPU clock: ~%lu MHz\n",
         (unsigned long)(cycles_per_ms / 1000));
}
```

### 3.9 调试基础设施

```c
#ifdef CONFIG_DEBUG_BOARD
#  define brdinfo(fmt, ...) \
     syslog(LOG_INFO, fmt, ##__VA_ARGS__)
#  define brderr(fmt, ...) \
     syslog(LOG_ERR, fmt, ##__VA_ARGS__)
#else
#  define brdinfo(...)
#  define brderr(...)
#endif
```

---

## 四、ADR 记录规范

### 4.1 何时写 ADR

- 选择了一种方案而放弃了其他方案时
- 硬件限制影响了软件设计时
- 踩坑后发现的关键约束

### 4.2 模板

```markdown
# ADR-NNN: <标题>

## 状态
Proposed / Accepted / Superseded by ADR-XXX

## 上下文
面临什么问题，有哪些约束。

## 决策
选择了什么方案。

## 理由
为什么选这个方案，其他方案为什么不行。

## 影响
对后续开发有什么约束或注意事项。
```

### 4.3 推荐记录的决策

| 编号 | 主题 |
|------|------|
| ADR-001 | DEV boot 模式 vs XSPI Flash 启动 |
| ADR-002 | 双目标条件编译策略 |
| ADR-003 | 时钟树配置（PLL1 M=4 N=50） |
| ADR-004 | Cache 策略（全关/全开/按区域） |
| ADR-005 | NPU 驱动适配路线 |

### 4.4 代码引用

代码中引用 ADR：

```c
/* ADR-003: 200MHz CPU clock — HSI/4*50=800MHz VCO, IC1/4 */
#define STM32_CPUCLK_FREQUENCY  200000000ul
```

---

## 五、驱动验证矩阵

### 5.1 验证层级标注

| 标签 | 含义 |
|------|------|
| BUILD | 仅编译通过 |
| QEMU | QEMU 中功能验证 |
| MEASURED | 真机验证通过 |
| PENDING | 未开始 |

### 5.2 矩阵模板

| 驱动 | BUILD | QEMU | MEASURED | 备注 |
|------|-------|------|----------|------|
| USART1 | PASS | PASS | - | PE5/PE6 AF7 |
| RCC/PLL1 | PASS | N/A | - | 需真机验证频率 |
| GPIO | PASS | N/A | - | |
| NPU | - | - | - | 未开始 |

---

## 六、CHANGELOG 规范

```markdown
# CHANGELOG

## [Unreleased]

### Added
- USART1 驱动（PE5-TX/PE6-RX AF7）[QEMU: PASS]
- 三阶段板级初始化

### Fixed
- board.h 行宽超 78 列

### Changed
- 文件路径注释改用 NuttX 逻辑路径
```

每条记录附带验证层级标注。

---

## 七、提交规范

1. 每个 commit 是**最小功能变更**（一个驱动 = 一个 commit）
2. 必须 `git commit -s`（sign-off）
3. commit message 不含 AI 相关信息
4. 提交前必须通过 `scripts/ci-check.sh`
5. 格式：`<type>(<scope>): <subject>`

```
feat(usart): add USART1 driver with PE5/PE6 AF7 pin config

Signed-off-by: changting27 <changting27@outlook.com>
```

类型：`feat` / `fix` / `refactor` / `docs` / `chore`

---

## 八、真机验证 Checklist

QEMU 无法覆盖的验证项：

| 验证项 | 方法 | 工具 |
|--------|------|------|
| 时钟频率 | SysTick 计数 / MCO 输出 | 示波器 |
| 串口波特率 | 实际收发 | 逻辑分析仪 |
| 电压域 | VDDIO2/3 电平 | 万用表 |
| 中断延迟 | GPIO toggle + handler | 示波器 |
| DEV boot | 冷启动 10 次 | ST-Link + 脚本 |
| Cache 一致性 | DMA 大块传输 CRC | 软件校验 |
| 功耗 | Stop/Standby 电流 | 电流表 |

---

## 九、比赛交付包

提交时打包自验证 bundle：

```
dist/
├── nuttx-nsh.bin       # 真机固件
├── nuttx-qemu.bin      # QEMU 固件
├── BUILD-LOG.txt       # 编译日志
├── QEMU-VERIFY.txt     # 冒烟测试输出
├── SHA256SUMS          # 所有文件哈希
└── VERIFY.sh           # 评委一键验证
```

---

## 十、核心理念

> **让自动化替你记住不变量。**

- nxstyle 记住编码风格不变
- 体积守护记住二进制不膨胀
- QEMU 冒烟记住系统能启动
- defconfig 守护记住配置不漂移
- DEBUGASSERT 记住时序不乱序
- ADR 记住决策不遗忘

人会忘，自动化不会。
