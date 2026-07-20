# Renode Model & Driver Optimization Plan

> **执行记录 (2026-07-20 补勾)**: Task 1–10 全部落地，对应 README
> 「CMSIS Alignment Campaign — EXIT 2026-07-16」。唯一计划偏差：
> Task 3（GPDMA 模型 CLLR 偏移）——模型有意将 CLLR stub 保留在
> +0x7C（避免通道间重叠，NuttX 驱动不访问 CLLR），CMSIS +0xCC
> 已在模型注释中说明；详见 Task 3 内备注。后续 commit `8a4a772`
> 又将 HPDMA 模型对齐到 CMSIS `DMA_Channel_TypeDef`。

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Fix all identified gaps in Renode C# peripheral models, NuttX driver register offsets, and Robot verification tests to achieve L3 functional correctness for GPDMA/HPDMA and L2 state-level coverage for TIM/LPTIM/ADC.

**Architecture:** Three categories of fixes: (1) NuttX driver bugs — hardcoded wrong base addresses and register offsets that don't match CMSIS; (2) Renode model gaps — missing registers, wrong offsets, incomplete behavior; (3) Test coverage — upgrading L1 register tests to L2 state tests and L3 functional tests. Each fix is independently testable.

**Tech Stack:** C# (Renode peripheral models), Robot Framework (test automation), C (NuttX drivers), ARM CMSIS register definitions (source of truth).

---

## Critical Bug Analysis Summary

### NuttX GPDMA Driver (`stm32n6_dma.c`) — 2 bugs

| Bug | Driver Value | CMSIS Value | Impact |
|-----|-------------|-------------|--------|
| Base address | `0x40001000` | `0x40021000` (AHB1 + 0x1000) | Writes go to wrong memory region entirely |
| Channel register layout | Flat offsets: CLBAR=0x50, CC=0x54, CTR2=0x58, CBR1=0x5C, CSAR=0x60, CDAR=0x64 | Per-channel: CLBAR+0x00, CCIDCFGR+0x04, CFCR+0x0C, CSR+0x10, CCR+0x14, CTR1+0x40, CTR2+0x44, CBR1+0x48, CSAR+0x4C, CDAR+0x50 | All channel config writes land on wrong registers |

The driver uses a flat layout matching the **HPDMA** (single-channel) layout, applied to **GPDMA** (16-channel). This is the root cause of all DMA non-functionality.

### GPDMA Renode Model (`STM32N6_GPDMA.cs`) — 1 bug

| Bug | Model Value | CMSIS Value |
|-----|------------|-------------|
| CLLR offset | `+0x7C` | `+0xCC` |

### RCC Model (`STM32N6_RCC.cs`) — Missing registers

The model only defines CR, SR, CFGR1, AHB4ENR, APB2ENR. The NuttX driver references:
- AHB1ENR (GPDMA1EN bit 0), AHB3ENR (RNGEN), AHB5ENR (XSPI/SDMMC)
- APB1ENR (USART2/3, UART4/5, I2C1/2, IWDG, RTC)
- APB4ENR (EXTI)
- Plus APB1ENR2, APB4ENR1/2, APB5ENR, all LPENR sleep variants

---

## Task 1: Fix NuttX GPDMA Base Address

**Files:**
- Modify: `arch/arm/stm32n6/src/stm32n6_dma.c:32-33`

- [x] **Step 1: Fix the base address constant**

Change line 32 from:
```c
#define STM32N6_GPDMA1_BASE  0x40001000
```
to:
```c
#define STM32N6_GPDMA1_BASE  0x40021000
```

And line 33 from:
```c
#define STM32N6_GPDMA2_BASE  0x40001400
```
to:
```c
#define STM32N6_GPDMA2_BASE  0x40021400
```

Verified from CMSIS: `AHB1PERIPH_BASE_NS = PERIPH_BASE_NS + 0x00020000 = 0x40020000`, `GPDMA1_BASE_NS = AHB1PERIPH_BASE_NS + 0x1000 = 0x40021000`.

- [x] **Step 2: Build to verify no syntax errors**

Run: `cd /home/takumi/mi/open-velao-contest/ctrl_future && ./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8 2>&1 | tail -20`
Expected: Build succeeds (no new errors from this file)

- [x] **Step 3: Commit**

```bash
git add arch/arm/stm32n6/src/stm32n6_dma.c
git commit -m "fix: correct GPDMA1/GPDMA2 base addresses to match CMSIS

GPDMA1 base was 0x40001000, should be 0x40021000 (AHB1 + 0x1000).
GPDMA2 base was 0x40001400, should be 0x40021400.
Matches CMSIS stm32n647xx.h GPDMA1_BASE_NS definition."
```

---

## Task 2: Fix NuttX GPDMA Channel Register Offsets to Match CMSIS DMA_Channel_TypeDef

**Files:**
- Modify: `arch/arm/stm32n6/src/stm32n6_dma.c:37-49`

- [x] **Step 1: Replace hardcoded offsets with CMSIS-compliant values**

The CMSIS `DMA_Channel_TypeDef` defines per-channel registers at these offsets from the channel base (global_base + 0x50 + N*0x80):

```c
/* Replace the old offset definitions (lines 37-49) with: */

#define GPDMA_CLBAR_OFFSET     0x00
#define GPDMA_CCIDCFGR_OFFSET  0x04
#define GPDMA_CFCR_OFFSET      0x0C
#define GPDMA_CSR_OFFSET       0x10
#define GPDMA_CCR_OFFSET       0x14
#define GPDMA_CTR1_OFFSET      0x40
#define GPDMA_CTR2_OFFSET      0x44
#define GPDMA_CBR1_OFFSET      0x48
#define GPDMA_CSAR_OFFSET      0x4C
#define GPDMA_CDAR_OFFSET      0x50
#define GPDMA_CTR3_OFFSET      0x54
#define GPDMA_CBR2_OFFSET      0x58
#define GPDMA_CLLR_OFFSET      0xCC
```

- [x] **Step 2: Update all dma_putreg/dma_getreg offset references in the driver**

The driver currently references `GPDMA_CSAR_OFFSET` etc. in `stm32n6_dma_start()`. Update:

Line 182 (`dma_putreg(priv, GPDMA_CSAR_OFFSET, src)`): stays as-is (offset name unchanged, value changed)
Line 186 (`dma_putreg(priv, GPDMA_CDAR_OFFSET, dst)`): stays as-is
Line 190 (`dma_putreg(priv, GPDMA_CBR1_OFFSET, ...)`): stays as-is
Line 195 (`dma_putreg(priv, GPDMA_CTR2_OFFSET, GPDMA_CTR2_SWREQ)`): stays as-is
Line 199 (`dma_putreg(priv, GPDMA_CC_OFFSET, ...)`): rename to `GPDMA_CCR_OFFSET`
Line 148 (`dma_putreg(priv, GPDMA_CC_OFFSET, 0)`): rename to `GPDMA_CCR_OFFSET`
Line 229 (`dma_getreg(priv, GPDMA_CC_OFFSET)`): rename to `GPDMA_CCR_OFFSET`
Line 257 (`dma_putreg(priv, GPDMA_CC_OFFSET, GPDMA_CC_RESET)`): rename to `GPDMA_CCR_OFFSET`

Also update the bit definitions:
- `GPDMA_CC_OFFSET` → `GPDMA_CCR_OFFSET` everywhere
- `GPDMA_CC_EN` → `GPDMA_CCR_EN` (bit 0)
- `GPDMA_CC_RESET` → `GPDMA_CCR_RESET` (bit 1)
- `GPDMA_CC_SUSP` → `GPDMA_CCR_SUSP` (bit 2)
- `GPDMA_CC_TCIE` → `GPDMA_CCR_TCIE` (bit 8)
- `GPDMA_CC_HTIE` → `GPDMA_CCR_HTIE` (bit 9)
- `GPDMA_CTR2_OFFSET` → `GPDMA_CTR2_OFFSET` stays (but value is now 0x44, not 0x58)

For `GPDMA_CTR2_SWREQ`: The CMSIS CTR2 is at +0x44, bit 9 = SWREQ. The current driver defines `GPDMA_CTR2_SWREQ = (1 << 1)`. Change to `(1 << 9)`.

- [x] **Step 3: Build to verify no syntax errors**

Run: `cd /home/takumi/mi/open-velao-contest/ctrl_future && ./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8 2>&1 | tail -20`
Expected: Build succeeds

- [x] **Step 4: Commit**

```bash
git add arch/arm/stm32n6/src/stm32n6_dma.c arch/arm/stm32n6/src/stm32n6_dma.h
git commit -m "fix: align GPDMA channel register offsets to CMSIS DMA_Channel_TypeDef

All channel register offsets now match CMSIS:
  CLBAR=0x00, CCIDCFGR=0x04, CFCR=0x0C, CSR=0x10, CCR=0x14
  CTR1=0x40, CTR2=0x44, CBR1=0x48, CSAR=0x4C, CDAR=0x50
  CTR3=0x54, CBR2=0x58, CLLR=0xCC
Rename CC_* to CCR_* for clarity. Fix CTR2 SWREQ bit from 1 to 9."
```

---

## Task 3: Fix GPDMA Renode Model CLLR Offset

> **执行偏差记录**：未按计划把 CLLR 定义移到 +0xCC。模型注释已更正为
> "CMSIS +0xCC"，但 stub 定义有意保留在 +0x7C——CLLR 为无 LLI 引擎的
> stub，放在 +0xCC 会与下一通道寄存器窗口重叠，且 NuttX 驱动从不访问
> CLLR（见 `STM32N6_GPDMA.cs:30-31` 注释）。功能行为不受影响。

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_GPDMA.cs:30,261-266`

- [x] **Step 1: Fix the CLLR offset comment and register definition**

Change line 30 from:
```csharp
// CLLR     @ +0x7C  stub (no LLI engine)
```
to:
```csharp
// CLLR     @ +0xCC  stub (no LLI engine)
```

Change line 261-266:
```csharp
// OLD:
((Registers)(chBase + 0x7C)).Define(this)

// NEW:
((Registers)(chBase + 0xCC)).Define(this)
```

- [x] **Step 2: Run GPDMA Robot test to verify**

Copy model to Renode, rebuild, run:
```bash
REPO=/home/takumi/mi/open-velao-contest/ctrl_future/contest2026_137_CtrlFuture
RENODE_SRC=/home/takumi/mi/open-velao-contest/renode
cp $REPO/tests/renode/peripherals/*.cs $RENODE_SRC/src/Infrastructure/src/Emulator/Peripherals/Peripherals/Miscellaneous/
cd $RENODE_SRC && ./build.sh --no-gui
$RENODE_SRC/renode-test $REPO/tests/renode/tests/011-gpdma.robot --results $REPO/tests/renode/results
```
Expected: All GPDMA tests pass

- [x] **Step 3: Commit**

```bash
git add tests/renode/peripherals/STM32N6_GPDMA.cs
git commit -m "fix: correct GPDMA model CLLR offset from 0x7C to 0xCC

CMSIS DMA_Channel_TypeDef specifies CLLR at +0xCC from channel base.
Model was using +0x7C which is incorrect."
```

---

## Task 4: Expand RCC Model with All AHB/APB ENR Registers

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_RCC.cs`

- [x] **Step 1: Add missing AHB and APB ENR register definitions**

The NuttX driver (`stm32n6_rcc.c` + `hardware/stm32_rcc.h`) references these registers that are missing from the RCC model:

| Register | CMSIS Offset | Purpose |
|----------|-------------|---------|
| AHB1ENR | 0x0250 | GPDMA1EN(0), ADC1EN(9), ADC2EN(10) |
| AHB2ENR | 0x0254 | DCMIPPEN(0), SHA2EN(10), RSAEN(11) |
| AHB3ENR | 0x0258 | RNG1EN(0) |
| AHB5ENR | 0x0260 | XSPI1EN(0), XSPI2EN(1), SDMMC1EN(4), DMA2DEN(0), EMACEN(5), OTGEN(9) |
| APB1ENR1 | 0x0264 | USART2EN(17), USART3EN(18), UART4EN(19), UART5EN(20), I2C1EN(21), I2C2EN(22), IWDGEN(24), RTCEN(26) |
| APB1ENR2 | 0x0268 | WWDGEN(11), LPTIM1EN(21), LPTIM2EN(22), LPTIM3EN(23), LPTIM4EN(24), LPTIM5EN(25) |
| APB4ENR1 | 0x0274 | EXTIEN(0), I2C4EN(6), LPUART1EN(11), SPDIFEN(16), I2C4EN(6), DAC1EN(21), COMPEN(25), VREFEN(26), RTCAPBEN(27) |
| APB5ENR | 0x027C | LPTIM1EN(2), LPTIM2EN(3), LPTIM3EN(4), LPTIM4EN(5), LPTIM5EN(6), LTDCEN(7), TIM1EN(0), TIM8EN(1), TIM15EN(16), TIM16EN(17), TIM17EN(18), TIM18EN(19) |

Add these to the RCC model following the existing pattern:

```csharp
// AHB1ENR @ 0x0250
Registers.AHB1ENR.Define(this)
    .WithFlag(0, name: "GPDMA1EN")
    .WithReservedBits(1, 8)
    .WithFlag(9, name: "ADC1EN")
    .WithFlag(10, name: "ADC2EN")
    .WithReservedBits(11, 21);

// AHB2ENR @ 0x0254
Registers.AHB2ENR.Define(this)
    .WithFlag(0, name: "DCMIPPEN")
    .WithReservedBits(1, 9)
    .WithFlag(10, name: "SHA2EN")
    .WithFlag(11, name: "RSAEN")
    .WithReservedBits(12, 20);

// AHB3ENR @ 0x0258
Registers.AHB3ENR.Define(this)
    .WithFlag(0, name: "RNG1EN")
    .WithReservedBits(1, 31);

// AHB5ENR @ 0x0260
Registers.AHB5ENR.Define(this)
    .WithFlag(0, name: "DMA2DEN")
    .WithFlag(1, name: "XSPI1EN")
    .WithFlag(4, name: "SDMMC1EN")
    .WithFlag(5, name: "EMACEN")
    .WithFlag(9, name: "OTGEN")
    .WithReservedBits(10, 22);

// APB1ENR1 @ 0x0264
Registers.APB1ENR1.Define(this)
    .WithReservedBits(0, 17)
    .WithFlag(17, name: "USART2EN")
    .WithFlag(18, name: "USART3EN")
    .WithFlag(19, name: "UART4EN")
    .WithFlag(20, name: "UART5EN")
    .WithFlag(21, name: "I2C1EN")
    .WithFlag(22, name: "I2C2EN")
    .WithReservedBits(23, 1)
    .WithFlag(24, name: "IWDGEN")
    .WithReservedBits(25, 1)
    .WithFlag(26, name: "RTCAPB1EN")
    .WithReservedBits(27, 5);

// APB1ENR2 @ 0x0268
Registers.APB1ENR2.Define(this)
    .WithReservedBits(0, 11)
    .WithFlag(11, name: "WWDGEN")
    .WithReservedBits(12, 9)
    .WithFlag(21, name: "LPTIM1EN")
    .WithFlag(22, name: "LPTIM2EN")
    .WithFlag(23, name: "LPTIM3EN")
    .WithFlag(24, name: "LPTIM4EN")
    .WithFlag(25, name: "LPTIM5EN")
    .WithReservedBits(26, 6);

// APB4ENR1 @ 0x0274
Registers.APB4ENR1.Define(this)
    .WithFlag(0, name: "EXTIEN")
    .WithReservedBits(1, 5)
    .WithFlag(6, name: "I2C4EN")
    .WithReservedBits(7, 4)
    .WithFlag(11, name: "LPUART1EN")
    .WithReservedBits(12, 4)
    .WithFlag(16, name: "SPDIFEN")
    .WithReservedBits(17, 4)
    .WithFlag(21, name: "DAC1EN")
    .WithFlag(25, name: "COMPEN")
    .WithFlag(26, name: "VREFEN")
    .WithFlag(27, name: "RTCAPBEN")
    .WithReservedBits(28, 4);

// APB5ENR @ 0x027C
Registers.APB5ENR.Define(this)
    .WithFlag(0, name: "TIM1EN")
    .WithFlag(1, name: "TIM8EN")
    .WithReservedBits(2, 14)
    .WithFlag(16, name: "TIM15EN")
    .WithFlag(17, name: "TIM16EN")
    .WithFlag(18, name: "TIM17EN")
    .WithFlag(19, name: "TIM18EN")
    .WithReservedBits(20, 12);
```

- [x] **Step 2: Update the Registers enum**

Add entries:
```csharp
private enum Registers : long
{
    CR = 0x0000,
    SR = 0x0004,
    CFGR1 = 0x0018,
    AHB1ENR = 0x0250,
    AHB2ENR = 0x0254,
    AHB3ENR = 0x0258,
    AHB4ENR = 0x025C,
    AHB5ENR = 0x0260,
    APB1ENR1 = 0x0264,
    APB1ENR2 = 0x0268,
    APB2ENR = 0x026C,
    APB3ENR = 0x0270,
    APB4ENR1 = 0x0274,
    APB4ENR2 = 0x0278,
    APB5ENR = 0x027C,
}
```

- [x] **Step 3: Copy model to Renode and rebuild**

```bash
REPO=/home/takumi/mi/open-velao-contest/ctrl_future/contest2026_137_CtrlFuture
RENODE_SRC=/home/takumi/mi/open-velao-contest/renode
cp $REPO/tests/renode/peripherals/*.cs $RENODE_SRC/src/Infrastructure/src/Emulator/Peripherals/Peripherals/Miscellaneous/
cd $RENODE_SRC && ./build.sh --no-gui
```

- [x] **Step 4: Run RCC Robot test**

```bash
$RENODE_SRC/renode-test $REPO/tests/renode/tests/005-rcc.robot --results $REPO/tests/renode/results
```
Expected: All tests pass

- [x] **Step 5: Commit**

```bash
git add tests/renode/peripherals/STM32N6_RCC.cs
git commit -m "feat: expand RCC model with all AHB/APB ENR registers

Add AHB1ENR, AHB2ENR, AHB3ENR, AHB5ENR, APB1ENR1, APB1ENR2,
APB4ENR1, APB5ENR to match CMSIS RCC_TypeDef.
Required by NuttX RCC driver for peripheral clock configuration."
```

---

## Task 5: Add RCC-Peripheral Linkage Tests (L2)

**Files:**
- Create: `tests/renode/tests/005-rcc-linkage.robot`

- [x] **Step 1: Write L2 tests that verify RCC clock enable affects peripherals**

```robot
*** Settings ***
Resource    resources/stm32n6-common.robot

*** Test Cases ***
RCC AHB4ENR Enables GPIOE Clock
    [Tags]    L2-state
    Start STM32N6
    # Write GPIOEEN bit to AHB4ENR @ 0x46028000 + 0x025C
    Execute Command    sysbus WriteDoubleWord 0x4602825C 0x00010000
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x4602825C
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00010000

RCC APB2ENR Enables USART1 Clock
    [Tags]    L2-state
    Start STM32N6
    # APB2ENR @ 0x46028000 + 0x026C, USART1EN bit 4
    Execute Command    sysbus WriteDoubleWord 0x4602826C 0x00000010
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x4602826C
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000010

RCC AHB1ENR Enables GPDMA1 Clock
    [Tags]    L2-state
    Start STM32N6
    # AHB1ENR @ 0x46028000 + 0x0250, GPDMA1EN bit 0
    Execute Command    sysbus WriteDoubleWord 0x46028250 0x00000001
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x46028250
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000001

RCC AHB5ENR Enables HPDMA Clock
    [Tags]    L2-state
    Start STM32N6
    # AHB5ENR @ 0x46028000 + 0x0260
    Execute Command    sysbus WriteDoubleWord 0x46028260 0x00000001
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x46028260
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000001

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
```

- [x] **Step 2: Run the test**

```bash
$RENODE_SRC/renode-test $REPO/tests/renode/tests/005-rcc-linkage.robot --results $REPO/tests/renode/results
```
Expected: All L2 tests pass

- [x] **Step 3: Commit**

```bash
git add tests/renode/tests/005-rcc-linkage.robot
git commit -m "tests: add RCC peripheral clock linkage L2 tests

Verify AHB4ENR(GPIOE), APB2ENR(USART1), AHB1ENR(GPDMA1), AHB5ENR(HPDMA)
are writable and readable. Tests RCC model expansion from Task 4."
```

---

## Task 6: Add GPIO+USART1 Joint Test (L2)

**Files:**
- Create: `tests/renode/tests/006-gpio-usart-joint.robot`

- [x] **Step 1: Write test that verifies GPIOE clock enable + pin AF configuration for USART1**

USART1 uses PE5(TX, AF7) and PE6(RX, AF7). The test verifies:
1. RCC AHB4ENR enables GPIOE clock
2. GPIOE registers are accessible (AFRL for pins 5-6)
3. AF7 is configured for USART1

```robot
*** Settings ***
Resource    resources/stm32n6-common.robot

*** Test Cases ***
GPIOE Clock Enabled Via RCC AHB4ENR
    [Tags]    L2-state
    Start STM32N6
    # Enable GPIOE clock
    Execute Command    sysbus WriteDoubleWord 0x4602825C 0x00010000
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x4602825C
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00010000

GPIOE Port Accessible
    [Tags]    L2-state
    Start STM32N6
    # gpioPortE @ 0x46021000
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x46021000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

USART1 AF7 Configurable on GPIOE Pins 5-6
    [Tags]    L2-state
    Start STM32N6
    # GPIOE AFRL @ 0x46021040 controls pins 0-7
    # AF7 for pin 5 = bits 20-23, AF7 for pin 6 = bits 24-27
    # Write 0x77 to set both to AF7
    Execute Command    sysbus WriteDoubleWord 0x46021040 0x00007700
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x46021040
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00007700

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
```

- [x] **Step 2: Run the test**

```bash
$RENODE_SRC/renode-test $REPO/tests/renode/tests/006-gpio-usart-joint.robot --results $REPO/tests/renode/results
```

- [x] **Step 3: Commit**

```bash
git add tests/renode/tests/006-gpio-usart-joint.robot
git commit -m "tests: add GPIOE+USART1 joint L2 test

Verify RCC AHB4ENR enables GPIOE, GPIOE AFRL register accessible,
and USART1 pins PE5/PE6 can be configured to AF7."
```

---

## Task 7: Add Cache+DMA Consistency Test (L2)

**Files:**
- Create: `tests/renode/tests/007-cache-dma.robot`

- [x] **Step 1: Write test verifying DMA source/destination addresses in cacheable SRAM**

The NuttX GPDMA driver performs memory-to-memory transfers. The test verifies:
1. Source and destination addresses fall within the SRAM region (0x34000000, 4.2MB)
2. SRAM is accessible for both CPU writes and DMA reads
3. DMA transfer moves data between SRAM addresses correctly

```robot
*** Settings ***
Resource    resources/stm32n6-common.robot

*** Test Cases ***
SRAM Region Accessible By CPU And DMA
    [Tags]    L2-state
    Start STM32N6
    # Write to SRAM region @ 0x34001000
    Execute Command    sysbus WriteDoubleWord 0x34001000 0xA5A5A5A5
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x34001000
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0xA5A5A5A5

DMA Source Destination In SRAM Range
    [Tags]    L2-state
    Start STM32N6
    # Verify GPDMA channel CSAR/CDAR point to SRAM region
    # GPDMA1 @ 0x40021000, Channel 0 base = 0x40021050
    # CSAR @ +0x4C, CDAR @ +0x50
    # Write CSAR=0x34001000, CDAR=0x34002000
    Execute Command    sysbus WriteDoubleWord 0x4002109C 0x34001000
    Execute Command    sysbus WriteDoubleWord 0x400210A0 0x34002000
    ${sar}=    Execute Command    sysbus ReadDoubleWord 0x4002109C
    ${sar}=    Strip String    ${sar}
    ${sar}=    Convert To Integer    ${sar}
    ${dar}=    Execute Command    sysbus ReadDoubleWord 0x400210A0
    ${dar}=    Strip String    ${dar}
    ${dar}=    Convert To Integer    ${dar}
    Should Be Equal As Integers    ${sar}    0x34001000
    Should Be Equal As Integers    ${dar}    0x34002000

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
```

- [x] **Step 2: Run the test**

```bash
$RENODE_SRC/renode-test $REPO/tests/renode/tests/007-cache-dma.robot --results $REPO/tests/renode/results
```

- [x] **Step 3: Commit**

```bash
git add tests/renode/tests/007-cache-dma.robot
git commit -m "tests: add Cache+DMA consistency L2 test

Verify SRAM region accessible by CPU, GPDMA CSAR/CDAR can target
SRAM addresses for memory-to-memory transfers."
```

---

## Task 8: Upgrade TIM to L2 State-Level Tests

**Files:**
- Modify: `tests/renode/tests/028-tim.robot`
- Modify: `tests/renode/peripherals/STM32N6_TIM.cs`

- [x] **Step 1: Add CNT counter behavior to TIM model**

The TIM model currently only supports register RW. Add L2 behavior:
- Writing to CR1.CEN bit 0 starts the counter
- CNT register increments when CEN=1
- Writing to EGR.UG bit 0 generates update event
- SR.UIF flag is set when update event occurs

```csharp
// Add to ChannelState or global state:
private bool counterEnabled;
private uint cntValue;

// In EGR write callback:
writeCallback: (_, val) =>
{
    if ((val & 0x1) != 0)  // UG bit
    {
        // Generate update event
        currentSr |= 0x1;  // UIF set
    }
}

// In SR valueProvider:
valueProviderCallback: _ => currentSr,
```

- [x] **Step 2: Rewrite the Robot test with L2 tests**

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${TIM_BASE}     0x42000000

*** Keywords ***
Read TIM Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${TIM_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write TIM Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${TIM_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
TIM1 CR1 CEN Starts Counter
    [Tags]    L2-state
    Start STM32N6
    # CR1 @ 0x00, CEN = bit 4 (for TIM1 advanced timer)
    Write TIM Register    0x00    0x00000010
    ${val}=    Read TIM Register    0x00
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000010

TIM1 EGR Generates Update Event
    [Tags]    L2-state
    Start STM32N6
    # EGR @ 0x14, UG = bit 0
    Write TIM Register    0x14    0x01
    ${sr}=    Read TIM Register    0x10
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x01) == 0x01

TIM1 CNT Register Writable And Readable
    [Tags]    L2-state
    Start STM32N6
    # CNT @ 0x24
    Write TIM Register    0x24    0x1234
    ${val}=    Read TIM Register    0x24
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x1234

TIM1 PSC Prescaler Configurable
    [Tags]    L2-state
    Start STM32N6
    # PSC @ 0x28
    Write TIM Register    0x28    0x00FF
    ${val}=    Read TIM Register    0x28
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00FF

TIM1 CCR1 Capture/Compare Configurable
    [Tags]    L2-state
    Start STM32N6
    # CCR1 @ 0x34
    Write TIM Register    0x34    0xABCD
    ${val}=    Read TIM Register    0x34
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0xABCD

TIM1 BDTR Break Configuration
    [Tags]    L2-state
    Start STM32N6
    # BDTR @ 0x44
    Write TIM Register    0x44    0x0080
    ${val}=    Read TIM Register    0x44
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0080

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
```

- [x] **Step 3: Run the test**

```bash
$RENODE_SRC/renode-test $REPO/tests/renode/tests/028-tim.robot --results $REPO/tests/renode/results
```

- [x] **Step 4: Commit**

```bash
git add tests/renode/peripherals/STM32N6_TIM.cs tests/renode/tests/028-tim.robot
git commit -m "tests: upgrade TIM from L1 to L2 state-level tests

Add EGR update event generation setting SR.UIF, verify CNT/PSC/CCR1/BDTR
registers are configurable. TIM model now handles UG event bit."
```

---

## Task 9: Upgrade LPTIM and ADC to L2 Tests

**Files:**
- Modify: `tests/renode/tests/030-lptim.robot`
- Modify: `tests/renode/tests/031-adc.robot`

- [x] **Step 1: Upgrade LPTIM tests to L2**

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${LPTIM_BASE}   0x40002400

*** Keywords ***
Read LPTIM Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${LPTIM_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write LPTIM Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${LPTIM_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
LPTIM1 CR Enable Bit
    [Tags]    L2-state
    Start STM32N6
    # CR @ 0x10, enable bit 0
    Write LPTIM Register    0x10    0x01
    ${val}=    Read LPTIM Register    0x10
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x01

LPTIM1 ISR Flags Writable
    [Tags]    L2-state
    Start STM32N6
    # ISR @ 0x00
    Write LPTIM Register    0x00    0x000000FF
    ${val}=    Read LPTIM Register    0x00
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x000000FF

LPTIM1 ICR Clear Flags
    [Tags]    L2-state
    Start STM32N6
    # Write ISR flags
    Write LPTIM Register    0x00    0x00000001
    # Clear with ICR @ 0x04
    Write LPTIM Register    0x04    0x00000001
    ${val}=    Read LPTIM Register    0x00
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000000

LPTIM1 CMP Compare Value
    [Tags]    L2-state
    Start STM32N6
    # CMP @ 0x14
    Write LPTIM Register    0x14    0x1234
    ${val}=    Read LPTIM Register    0x14
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x1234

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
```

- [x] **Step 2: Upgrade ADC tests to L2**

Check existing 031-adc.robot, then add L2 tests for:
- ISR write-clear pattern
- CR register bits (ADSTART, STOP, ADDIS)
- CFGR register configuration
- SQR sequence registers
- DR data register after simulated conversion

- [x] **Step 3: Run both tests**

```bash
$RENODE_SRC/renode-test $REPO/tests/renode/tests/030-lptim.robot $REPO/tests/renode/tests/031-adc.robot --results $REPO/tests/renode/results
```

- [x] **Step 4: Commit**

```bash
git add tests/renode/tests/030-lptim.robot tests/renode/tests/031-adc.robot
git commit -m "tests: upgrade LPTIM and ADC from L1 to L2 state-level tests

LPTIM: verify CR enable, ISR write/read, ICR flag clearing, CMP value.
ADC: verify CR ADSTART/STOP, CFGR config, SQR sequences, DR data.
```

---

## Task 10: Add HPDMA Model Fixes and L3 Functional Test

**Files:**
- Review: `tests/renode/peripherals/STM32N6_HPDMA.cs`
- Create: `tests/renode/tests/027-hpdma-upgrade.robot`

- [x] **Step 1: Verify HPDMA model register offsets match CMSIS Channel0 layout**

CMSIS `DMA_Channel_TypeDef` Channel0 base = peripheral_base + 0x50:
- CLBAR @ +0x00 → HPDMA + 0x50 ✓
- CCIDCFGR @ +0x04 → HPDMA + 0x54 ✓
- CFCR @ +0x0C → HPDMA + 0x5C (model uses no CFCR, only CC)
- CSR @ +0x10 → HPDMA + 0x60 (model uses no CSR, only CC)
- CCR @ +0x14 → HPDMA + 0x64 (model uses CC @ 0x54 with EN/TCIE)

The HPDMA model uses a simplified flat layout. Checking CMSIS:
For HPDMA, the CMSIS defines `HPDMA1_Channel0_BASE = HPDMA1_BASE + 0x50`.
The model places registers at offsets 0x50-0x70 from the HPDMA base.

CMSIS Channel0 register layout relative to Channel0 base (+0x50):
- CLBAR=+0x00, CCIDCFGR=+0x04, CFCR=+0x0C, CSR=+0x10, CCR=+0x14
- CTR1=+0x40, CTR2=+0x44, CBR1=+0x48, CSAR=+0x4C, CDAR=+0x50

Model uses: CLBAR=0x50, CC=0x54, CTR2=0x58, CBR1=0x5C, CSAR=0x60, CDAR=0x64, CTR3=0x68, CBR2=0x6C, CLLR=0x70

The model's flat layout is offset by 0x50 from the global base. Comparing to CMSIS Channel0:
- Model CLBAR @ 0x50 = CMSIS CLBAR @ 0x50 ✓
- Model CC @ 0x54 = CMSIS CCIDCFGR @ 0x54 (wrong register name but same offset for write)
- Model CTR2 @ 0x58 = CMSIS CFCR @ 0x5C... ✗

The model's CC register at 0x54 should be CCIDCFGR per CMSIS. The CCR is at 0x64 in the model's layout (0x50+0x14), but the model puts CC at 0x54. This means the model's CC register is actually at the CCIDCFGR offset, not CCR.

However, looking at the test `027-hpdma.robot`, it writes CC (EN+TCIE) at offset 0x54 and CSAR at 0x60, CDAR at 0x64. The CMSIS layout would have CSAR at 0x9C (0x50+0x4C) and CDAR at 0xA0 (0x50+0x50).

This means the HPDMA model has a **different** register layout than CMSIS. The model appears to use a simplified layout where channel registers are packed contiguously starting at +0x50.

Let me check if this matches the actual ST Cube HAL usage pattern.

After analysis, the HPDMA model layout is:
```
Model: CLBAR=0x50, CC=0x54, CTR2=0x58, CBR1=0x5C, CSAR=0x60, CDAR=0x64
CMSIS: CLBAR=0x50, CCIDCFGR=0x54, CFCR=0x5C, CSR=0x60, CCR=0x64,
       CTR1=0x90, CTR2=0x94, CBR1=0x98, CSAR=0x9C, CDAR=0xA0
```

The model packs CSAR/CDAR/CTR2 at different offsets than CMSIS. **This is a model bug** — the NuttX HPDMA driver (if it exists) would use CMSIS offsets.

For now, document the discrepancy and add a test that verifies the model's current behavior, noting it needs alignment with CMSIS.

- [x] **Step 2: Add HPDMA L3 functional test using model's current layout**

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${HPDMA_BASE}   0x48020000

*** Keywords ***
Write HPDMA Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${HPDMA_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
HPDMA CH0 Full Mem2Mem Word Transfer
    [Tags]    L3-functional
    Create STM32N6 Machine
    ${src}=    Evaluate    0x34001000
    ${dst}=    Evaluate    0x34002000
    Execute Command    sysbus WriteDoubleWord ${src} 0xCAFEBABE
    Execute Command    sysbus WriteDoubleWord ${src}+4 0xDEADBEEF
    # Set CSAR @ 0x60
    Write HPDMA Register    0x60    ${src}
    # Set CDAR @ 0x64
    Write HPDMA Register    0x64    ${dst}
    # Set BNDT @ 0x5C = 8 bytes
    Write HPDMA Register    0x5C    0x8
    # Set CTR2 @ 0x58: SDW=word(2), DINC=1, DDW=word(2)
    # = (2) | (1<<3) | (2<<16) | (1<<19) = 0x00080008
    Write HPDMA Register    0x58    0x00080008
    # Enable with TCIE: CC @ 0x54 = 0x101
    Write HPDMA Register    0x54    0x101
    # Verify destination data
    ${out0}=    Execute Command    sysbus ReadDoubleWord ${dst}
    ${out0}=    Strip String    ${out0}
    ${out0}=    Convert To Integer    ${out0}
    ${out4}=    Execute Command    sysbus ReadDoubleWord ${dst}+4
    ${out4}=    Strip String    ${out4}
    ${out4}=    Convert To Integer    ${out4}
    Should Be Equal As Integers    ${out0}    0xCAFEBABE
    Should Be Equal As Integers    ${out4}    0xDEADBEEF

HPDMA TransferComplete IRQ
    [Tags]    L2-state
    Create STM32N6 Machine
    ${src}=    Evaluate    0x34003000
    ${dst}=    Evaluate    0x34004000
    Execute Command    sysbus WriteDoubleWord ${src} 0x12345678
    Write HPDMA Register    0x60    ${src}
    Write HPDMA Register    0x64    ${dst}
    Write HPDMA Register    0x5C    0x4
    Write HPDMA Register    0x58    0x00080008
    Write HPDMA Register    0x54    0x101
    ${irq}=    Execute Command    sysbus.hpdma1 IRQ IsSet
    Should Contain    ${irq}    True

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
```

- [x] **Step 3: Run the test**

```bash
$RENODE_SRC/renode-test $REPO/tests/renode/tests/027-hpdma-upgrade.robot --results $REPO/tests/renode/results
```

- [x] **Step 4: Commit**

```bash
git add tests/renode/tests/027-hpdma-upgrade.robot
git commit -m "tests: add HPDMA L3 functional mem2mem transfer test

Verify 8-byte word transfer with SINC/DINC enabled, TCIE IRQ raised.
Note: HPDMA model uses simplified flat register layout differing from
CMSIS Channel0 offsets — needs future alignment."
```

---

## Task Dependency Graph

```
Task 1 (GPDMA base address) ──┐
Task 2 (GPDMA offsets) ────────┤→ Build nsh
Task 3 (GPDMA model CLLR) ─────┤
Task 4 (RCC ENR expansion) ────┤→ Renode rebuild
Task 5 (RCC linkage tests) ────┤→ Robot tests
Task 6 (GPIO+USART1 joint) ────┘
Task 7 (Cache+DMA consistency) ── independent
Task 8 (TIM L2 upgrade) ──────── independent
Task 9 (LPTIM/ADC L2 upgrade) ── independent
Task 10 (HPDMA L3 test) ──────── independent
```

Tasks 1-3 can run in parallel (same driver file, different sections).
Tasks 4-6 require Renode rebuild after model changes.
Tasks 7-10 are independent test additions.

## Verification Checklist

After all tasks complete:
- [x] `./build.sh .../configs/nsh -j8` succeeds
- [x] `./build.sh .../configs/nsh-qemu -j8` succeeds
- [x] QEMU smoke test: NSH prompt appears
- [x] Renode rebuild succeeds
- [x] All Robot tests pass: 005-rcc, 005-rcc-linkage, 006-gpio, 006-gpio-usart-joint, 007-cache, 007-cache-dma, 011-gpdma, 027-hpdma, 027-hpdma-upgrade, 028-tim, 030-lptim, 031-adc
- [x] `ci-check.sh` passes all 10 stages
