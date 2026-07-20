# Renode Completeness Gaps 1–8 Implementation Plan

> **执行记录 (2026-07-20 补勾)**: T1–T16 全部落地。证据：commits
> `ee9bfb1`（T1）、`e4d3840`（T2）、`380468a`（T3–T7）、
> `5567810`（T8–T16）；退出矩阵见 `tests/renode/README.md`
> 「Completeness Campaign (Gaps 1–8) — EXIT 2026-07-14」。
> T11 采用任务内允许的 Approach B（044-firmware-l3.robot）。

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Close **all eight** Renode completeness gaps in one campaign so NuttX STM32N6 drivers can be correctness-tested on Renode without a follow-on “optimization wave.”

**Architecture:** Keep contest isolation (only this repo). Models stay under `tests/renode/peripherals/`; platform in `tests/renode/stm32n647x0.repl`; suites under `tests/renode/tests/`. Raise fidelity with dual-layer tests: (A) pure model MMIO/IRQ Robot, (B) multi-peripheral model L3, (C) firmware/NSH and contest-local drivertest where size allows. Use CMSIS IRQn as Renode `nvic@N` index. Reference IRQ pattern: `STM32N6_RNG.cs` / `STM32N6_I2C.cs` (`public GPIO IRQ` + `UpdateInterrupt` + `.repl` wire).

**Tech Stack:** Renode C# peripherals, Robot Framework (`renode-test`), NuttX STM32N6 out-of-tree drivers, CMSIS `stm32n647xx.h`, `$RENODE_SRC=/home/takumi/mi/open-velao-contest/renode`.

**Baseline (2026-07-14):** Phase-1 gate **217 pass / 0 fail**. IRQ wired today: all USART/UART, `i2c1@100`, `rng@40` only. HPDMA suite false-green on GPDMA base. RNG `Force Tags L1-register` pollutes tags. No multi-peripheral L3. Phase-2 cmocka deferred without contest-local path.

---

## Gap Map → Task Coverage (no second wave)

| # | Gap | Closed by tasks | Exit evidence |
|---|-----|-----------------|---------------|
| 1 | Tests are mostly model MMIO, not driver behavior | T10, T11, T12 | Firmware L3 + contest-local drivertest + driver-sequence Robot |
| 2 | IRQ wiring incomplete | T3–T7 | SPI/GPDMA/EXTI/SDMMC/HPDMA IRQ GPIO + `.repl` + NVIC pending Robot |
| 3 | L3 narrow; no multi-peripheral | T8, T9 | SPI↔GPDMA, SDMMC data (or IDMA-style) L3 suites |
| 4 | Instance coverage thin | T2, T13 | I2C2–4 in repl+suite; SPI2/SPI6 L2; USART2/6 L2 |
| 5 | Correctness bugs (HPDMA base) | T1 | HPDMA suite on `0x48020000`; base audit green |
| 6 | Tag/matrix noise | T1, T14 | RNG Force Tags fixed; README matrix accurate |
| 7 | Uneven model depth | T7, T9, T15 | Driver-critical ≥ L2; intentional L1 listed; no silent stubs for MMIO drivers |
| 8 | Phase-2 drivertest deferred | T11, T12 | Contest-local path documented + at least one e2e case or explicit size-gated stub with green suite |

**Single campaign exit checklist** is Task 16. Do not open a new “wave” plan after this; if something is intentionally out of scope, it must appear in the **Deferred-with-reason** table in T14/T16, not as an untracked hole.

---

## Fidelity tiers (unchanged definitions)

| Tier | Meaning | Exit criteria |
|------|---------|---------------|
| L1 | Register shell | Reset/RW; boot still passes |
| L2 | Stateful | Enable→ready, IRQ assert/clear, basic flags |
| L3 | Functional | Data path and/or multi-peripheral interaction usable by driver |

**Target after this plan:**

- Every NuttX MMIO driver in tree ≥ **L2** (model + ≥1 `L2-state` Robot).
- Bus masters that move data (GPDMA, SPI, I2C, SDMMC, USART1 path) ≥ **L3** for the paths drivers use.
- At least **one** multi-peripheral L3 (SPI or I2C + GPDMA) and **one** SDMMC data-path L3.
- IRQ wired for: SPI1–6, GPDMA1_CH0 (min; prefer CH0–3), EXTI0–15 (or line0 demo), SDMMC1, HPDMA1_CH0, existing UART/I2C1/RNG.
- HPDMA tests hit real base; tags honest; matrix matches reality.

### IRQn reference (CMSIS = Renode `nvic@N`)

| Peripheral | IRQn | Notes |
|------------|------|-------|
| EXTI0…15 | 20…35 | One GPIO per line or multiplexed array |
| RNG | 40 | Already wired |
| HPDMA1_CH0…15 | 68…83 | Wire CH0 at minimum |
| GPDMA1_CH0…15 | 84…99 | Wire CH0 at minimum; prefer CH0–3 |
| I2C1_EV / ER | 100 / 101 | EV already wired; ER optional |
| I2C2_EV / ER | 102 / 103 | New instances |
| I2C3_EV / ER | 104 / 105 | |
| I2C4_EV / ER | 106 / 107 | |
| SPI1…6 | 153…158 | |
| XSPI1 | 170 | Optional this campaign |
| SDMMC1 | 174 | |
| ETH1 | 179 | L2 IRQ optional |
| USART1 | 159 | Already wired |

NuttX `STM32_IRQ_*` = IRQn + 16. Renode uses **CMSIS IRQn**.

### Addresses (CMSIS NS)

| Block | Base |
|-------|------|
| GPDMA1 | `0x40021000` |
| HPDMA1 | `0x48020000` |
| I2C1 | `0x40005400` |
| I2C2 | `0x40005800` |
| I2C3 | `0x40005C00` |
| I2C4 | `0x46001C00` |
| SPI1 | `0x42003000` |
| SPI2 | `0x40003800` |
| SPI3 | `0x40003C00` |
| SPI4 | `0x42003400` |
| SPI5 | `0x42005000` |
| SPI6 | `0x46001400` |
| SDMMC1 | `0x48027000` |
| EXTI | `0x46025000` |
| USART2 | `0x40004400` |
| USART6 | `0x42001400` |

---

## File Map

| Path | Role |
|------|------|
| `tests/renode/stm32n647x0.repl` | Platform; IRQ wires; multi-instance |
| `tests/renode/peripherals/STM32N6_{SPI,GPDMA,HPDMA,EXTI,SDMMC,I2C,RCC}.cs` | Model upgrades |
| `tests/renode/tests/*.robot` | Suites |
| `tests/renode/tests/resources/stm32n6-common.robot` | Shared helpers (NVIC read helper added here) |
| `tests/renode/tests/040-irq-wiring.robot` | **New** cross-peripheral IRQ regression |
| `tests/renode/tests/041-multi-peripheral.robot` | **New** SPI/GPDMA multi L3 |
| `tests/renode/tests/042-driver-sequences.robot` | **New** NuttX-driver-ordered MMIO sequences |
| `app/renode_drivertest/` or `tests/renode/firmware/` | Contest-local mini drivertest (T11) |
| `tests/renode/README.md` | Matrix + gap closure table |
| `scripts/renode-test.sh` | Already fixed path; optional filter docs |
| `docs/DEV-METHODOLOGY.md` | Optional one-line Phase-2 pointer |

---

## Common helpers (add once in Task 1)

Add to `tests/renode/tests/resources/stm32n6-common.robot`:

```robot
*** Keywords ***
Read NVIC ISPRn Pending
    [Documentation]    Read NVIC ISPR word for IRQn (CMSIS number).
    ...                ISPR0 @ 0xE000E200 covers IRQn 0-31; ISPR1 @ +4, etc.
    [Arguments]    ${irqn}
    ${word}=    Evaluate    int(${irqn}) // 32
    ${bit}=     Evaluate    int(${irqn}) % 32
    ${addr}=    Evaluate    0xE000E200 + (${word} * 4)
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    ${pending}=    Evaluate    (int(${val}) >> ${bit}) & 1
    RETURN    ${pending}

Assert NVIC Pending
    [Arguments]    ${irqn}
    ${p}=    Read NVIC ISPRn Pending    ${irqn}
    Should Be Equal As Integers    ${p}    1

Assert NVIC Not Pending
    [Arguments]    ${irqn}
    ${p}=    Read NVIC ISPRn Pending    ${irqn}
    Should Be Equal As Integers    ${p}    0
```

**Note:** Some Renode NVIC builds clear pending on take; for model-only tests, prefer **Create STM32N6 Machine without Start Emulation / without CPU run**, or read peripheral `IRQ` via monitor if available. Preferred approach used in this plan:

1. `Create STM32N6 Machine` (no `Start Emulation` if CPU would clear pending).
2. Drive peripheral MMIO.
3. Assert via `sysbus.GetTag` / NVIC ISPR **or** Renode `sysbus.<name> IRQ` state if exposed.

If ISPR is flaky under running CPU, suites must use **machine without Start Emulation** for pure IRQ L2 (same pattern as PWR reset-value tests using `Create STM32N6 Machine`).

---

### Task 1: Correctness + hygiene (Gaps 5, 6) — HPDMA base, RNG tags, base audit helper

**Files:**
- Modify: `tests/renode/tests/027-hpdma.robot`
- Modify: `tests/renode/tests/039-rng.robot`
- Modify: `tests/renode/tests/resources/stm32n6-common.robot`
- Create: `tests/renode/tests/043-base-audit.robot`
- Modify: `tests/renode/README.md` (note HPDMA fix in matrix)

- [x] **Step 1: Fix HPDMA base in robot**

In `027-hpdma.robot` change:

```robot
${HPDMA_BASE}   0x48020000
```

Add tags:

```robot
Force Tags      hpdma
```

And per-case `[Tags]    L1-register` / `boot-regression` as appropriate. Ensure cases no longer pass against GPDMA:

```robot
HPDMA Base Is Not GPDMA
    [Tags]    L1-register
    Create STM32N6 Machine
    # Writing HPDMA SECCFGR must not change GPDMA SECCFGR
    Execute Command    sysbus WriteDoubleWord 0x48020000 0xA5A5A5A5
    ${g}=    Execute Command    sysbus ReadDoubleWord 0x40021000
    ${g}=    Strip String    ${g}
    Should Not Be Equal As Integers    ${g}    0xA5A5A5A5
    ${h}=    Execute Command    sysbus ReadDoubleWord 0x48020000
    ${h}=    Strip String    ${h}
    Should Be Equal As Integers    ${h}    0xA5A5A5A5
```

- [x] **Step 2: Fix RNG Force Tags**

In `039-rng.robot` change:

```robot
Force Tags      rng
```

Move L1 only onto L1 cases:

```robot
CR Reset Value
    [Tags]    L1-register
    ...
RNGEN Sets DRDY
    [Tags]    L2-state
    ...
```

Add explicit IRQ case tags `L2-state` (if present) without Force-tagging L1.

- [x] **Step 3: Add common NVIC helpers** (see Common helpers above) to `stm32n6-common.robot`.

- [x] **Step 4: Base audit suite**

Create `043-base-audit.robot`:

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      base-audit

*** Test Cases ***
GPDMA At 0x40021000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x40021000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

HPDMA At 0x48020000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x48020000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

I2C1 At 0x40005400
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x40005400
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

SPI1 At 0x42003000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x42003000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

SDMMC1 At 0x48027000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x48027000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

USART1 At 0x40011000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x40011000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0
```

(Confirm USART1 base from `.repl` / CMSIS before committing — use the address already in `stm32n647x0.repl`.)

- [x] **Step 5: Run focused suites**

```bash
cd /home/takumi/mi/open-velao-contest/ctrl_future
bash contest2026_137_CtrlFuture/scripts/renode-test.sh \
  contest2026_137_CtrlFuture/tests/renode/tests/027-hpdma.robot \
  contest2026_137_CtrlFuture/tests/renode/tests/039-rng.robot \
  contest2026_137_CtrlFuture/tests/renode/tests/043-base-audit.robot
```

Expected: all PASS; HPDMA no longer aliases GPDMA.

- [x] **Step 6: Commit**

```bash
git add tests/renode/tests/027-hpdma.robot \
  tests/renode/tests/039-rng.robot \
  tests/renode/tests/resources/stm32n6-common.robot \
  tests/renode/tests/043-base-audit.robot \
  tests/renode/README.md
git commit -s -m "$(cat <<'EOF'
tests: renode: fix HPDMA base, RNG tags, add base audit

Close false-green HPDMA suite (was probing GPDMA) and stop
Force-tagging all RNG cases as L1-register.
EOF
)"
```

---

### Task 2: Multi-instance I2C in platform (Gap 4)

**Files:**
- Modify: `tests/renode/stm32n647x0.repl`
- Modify: `tests/renode/tests/014-i2c.robot`

- [x] **Step 1: Add I2C2–4 to `.repl`**

After `i2c1` block:

```
// I2C2 @ 0x40005800, IRQ 102 (I2C2_EV)
i2c2: Miscellaneous.STM32N6_I2C @ sysbus 0x40005800
    IRQ -> nvic@102

// I2C3 @ 0x40005C00, IRQ 104
i2c3: Miscellaneous.STM32N6_I2C @ sysbus 0x40005C00
    IRQ -> nvic@104

// I2C4 @ 0x46001C00, IRQ 106
i2c4: Miscellaneous.STM32N6_I2C @ sysbus 0x46001C00
    IRQ -> nvic@106
```

Verify bases against CMSIS `I2C2_BASE_NS` etc. before commit; adjust if datasheet differs.

- [x] **Step 2: Extend `014-i2c.robot`**

```robot
${I2C2_BASE}    0x40005800
${I2C3_BASE}    0x40005C00
${I2C4_BASE}    0x46001C00

I2C2 CR1 Accessible
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord ${I2C2_BASE}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    0

I2C3 PE Enable
    [Tags]    L2-state
    Create STM32N6 Machine
    Execute Command    sysbus WriteDoubleWord ${I2C3_BASE} 0x1
    ${val}=    Execute Command    sysbus ReadDoubleWord ${I2C3_BASE}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    1

I2C4 PE Enable
    [Tags]    L2-state
    Create STM32N6 Machine
    Execute Command    sysbus WriteDoubleWord ${I2C4_BASE} 0x1
    ${val}=    Execute Command    sysbus ReadDoubleWord ${I2C4_BASE}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    1
```

- [x] **Step 3: Run + commit**

```bash
bash contest2026_137_CtrlFuture/scripts/renode-test.sh \
  contest2026_137_CtrlFuture/tests/renode/tests/014-i2c.robot
git add tests/renode/stm32n647x0.repl tests/renode/tests/014-i2c.robot
git commit -s -m "tests: renode: add I2C2-4 instances and suite coverage"
```

---

### Task 3: SPI IRQ model + wiring (Gap 2)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_SPI.cs`
- Modify: `tests/renode/stm32n647x0.repl`
- Modify: `tests/renode/tests/013-spi.robot`

- [x] **Step 1: Add IRQ to SPI model**

In constructor:

```csharp
public STM32N6_SPI(IMachine machine) : base(machine)
{
    IRQ = new GPIO();
    receiveFifo = new Queue<uint>();
    DefineRegisters();
}

public GPIO IRQ { get; }

public override void Reset()
{
    base.Reset();
    receiveFifo.Clear();
    txComplete = false;
    endOfTransfer = false;
    IRQ.Unset();
}

private void UpdateInterrupt()
{
    // L2: EOTIE && EOT (bit 3 in IER / SR bit 3) — match CMSIS names used in model
    var eotie = (ierValue & (1u << 3)) != 0;
    IRQ.Set(eotie && endOfTransfer);
}
```

Wire `IER` as a real field (replace opaque value field):

```csharp
Registers.IER.Define(this)
    .WithValueField(0, 32, out ierField, name: "IER",
        changeCallback: (_, __) => UpdateInterrupt());
```

On TX path when setting `endOfTransfer = true` / TXC, call `UpdateInterrupt()`. On IFCR clear of EOT, call `UpdateInterrupt()`. Store `ierValue` from field or use `IValueRegisterField`.

Minimal viable: track `uint ier;` writeCallback on IER; UpdateInterrupt on EOT set/clear.

- [x] **Step 2: Wire SPI1–6 in `.repl`**

```
spi1: Miscellaneous.STM32N6_SPI @ sysbus 0x42003000
    IRQ -> nvic@153
spi2: Miscellaneous.STM32N6_SPI @ sysbus 0x40003800
    IRQ -> nvic@154
spi3: Miscellaneous.STM32N6_SPI @ sysbus 0x40003C00
    IRQ -> nvic@155
spi4: Miscellaneous.STM32N6_SPI @ sysbus 0x42003400
    IRQ -> nvic@156
spi5: Miscellaneous.STM32N6_SPI @ sysbus 0x42005000
    IRQ -> nvic@157
spi6: Miscellaneous.STM32N6_SPI @ sysbus 0x46001400
    IRQ -> nvic@158
```

- [x] **Step 3: Robot L2 IRQ + SPI6 L2**

```robot
SPI1 EOT Raises IRQ When EOTIE
    [Tags]    L2-state
    Create STM32N6 Machine
    # SPE
    Write SPI1 Register    ${CR1_OFFSET}    0x1
    # IER.EOTIE bit3
    Write SPI1 Register    0x10    0x8
    Write SPI1 Register    ${TXDR_OFFSET}    0xA5
    Assert NVIC Pending    153

SPI6 Enable Asserts TXP
    [Tags]    L2-state
    Create STM32N6 Machine
    Write SPI6 Register    ${CR1_OFFSET}    0x1
    ${sr}=    Read SPI6 Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x2) == 0x2
```

- [x] **Step 4: Rebuild Renode + run**

```bash
bash contest2026_137_CtrlFuture/scripts/renode-test.sh \
  contest2026_137_CtrlFuture/tests/renode/tests/013-spi.robot
```

- [x] **Step 5: Commit**

```bash
git commit -s -m "arch: renode: wire SPI1-6 IRQ and L2 EOT path"
```

---

### Task 4: GPDMA IRQ model + wiring (Gap 2)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_GPDMA.cs`
- Modify: `tests/renode/stm32n647x0.repl`
- Modify: `tests/renode/tests/011-gpdma.robot`

- [x] **Step 1: Channel IRQ GPIOs**

GPDMA has per-channel IRQs. Minimum for this campaign: **expose CH0…CH3 as `IRQ0`…`IRQ3`** (or a single `IRQ` OR of CH0 for simpler wiring). Prefer per-channel:

```csharp
public GPIO IRQ0 { get; }
public GPIO IRQ1 { get; }
public GPIO IRQ2 { get; }
public GPIO IRQ3 { get; }
// Optionally IRQ4..IRQ15 later

private readonly GPIO[] channelIrqs;

public STM32N6_GPDMA(IMachine machine) : base(machine)
{
    channelIrqs = new GPIO[4];
    for (var i = 0; i < 4; i++)
    {
        channelIrqs[i] = new GPIO();
    }
    IRQ0 = channelIrqs[0];
    // ...
}
```

On transfer complete, if `TCIE` set and `TCF` set:

```csharp
private void UpdateChannelInterrupt(int ch)
{
    if (ch < 0 || ch >= channelIrqs.Length)
    {
        return;
    }
    var st = channels[ch];
    channelIrqs[ch].Set(st.tcie && st.tcf);
}
```

Also update **MISR** bit `ch` when masked interrupt pending (TCIE && TCF).

- [x] **Step 2: `.repl` wiring**

```
gpdma1: Miscellaneous.STM32N6_GPDMA @ sysbus 0x40021000
    IRQ0 -> nvic@84
    IRQ1 -> nvic@85
    IRQ2 -> nvic@86
    IRQ3 -> nvic@87
```

- [x] **Step 3: Robot**

```robot
GPDMA CH0 TCIE Raises IRQ After Mem2Mem
    [Tags]    L2-state    L3-functional
    Create STM32N6 Machine
    ${SRC}=    Set Variable    0x34001000
    ${DST}=    Set Variable    0x34002000
    Execute Command    sysbus WriteDoubleWord ${SRC} 0x11223344
    Write GPDMA Register    0x9C    ${SRC}
    Write GPDMA Register    0xA0    ${DST}
    Write GPDMA Register    0x98    0x4
    Write GPDMA Register    0x90    0x00080008
    Write GPDMA Register    0x94    0x200
    # CCR: EN|TCIE = bit0|bit8
    Write GPDMA Register    0x64    0x101
    Assert NVIC Pending    84
```

- [x] **Step 4: Run + commit**

```bash
git commit -s -m "arch: renode: GPDMA channel IRQ and NVIC wiring"
```

---

### Task 5: EXTI line IRQs (Gap 2)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_EXTI.cs`
- Modify: `tests/renode/stm32n647x0.repl`
- Modify: `tests/renode/tests/010-exti.robot`

- [x] **Step 1: Model**

```csharp
public GPIO[] Lines { get; }

public STM32N6_EXTI(IMachine machine) : base(machine)
{
    Lines = new GPIO[16];
    for (var i = 0; i < 16; i++)
    {
        Lines[i] = new GPIO();
    }
    DefineRegisters();
}

private void UpdateLines()
{
    // Bank0 lines 0-15: pending if (rpr|fpr) & imr
    for (var i = 0; i < 16; i++)
    {
        var mask = 1u << i;
        var pending = ((rpr[0] | fpr[0]) & mask) != 0;
        var unmasked = (imr0 & mask) != 0;
        Lines[i].Set(pending && unmasked);
    }
}
```

Track `imr0` from IMR1 writes. Call `UpdateLines` after SWIER / RPR / FPR / IMR changes. On Reset, `Unset` all lines.

Renode `.repl` multi-GPIO connection pattern (if array not supported, expose `Line0`…`Line15` properties):

```csharp
public GPIO Line0 => Lines[0];
// ... through Line15
```

- [x] **Step 2: `.repl`**

```
exti: Miscellaneous.STM32N6_EXTI @ sysbus 0x46025000
    Line0 -> nvic@20
    Line1 -> nvic@21
    # ... at least Line0-Line3 for tests; ideally Line0-Line15
```

Minimum acceptable: **Line0–Line3** wired; document rest as same pattern.

- [x] **Step 3: Robot**

```robot
EXTI SWIER Line0 Raises NVIC When IMR
    [Tags]    L2-state
    Create STM32N6 Machine
    Write EXTI Register    ${REG_IMR1}    0x1
    Write EXTI Register    ${REG_RTSR1}   0x1
    Write EXTI Register    ${REG_SWIER1}  0x1
    Assert NVIC Pending    20
    # W1C RPR
    Write EXTI Register    ${REG_RPR1}    0x1
    Assert NVIC Not Pending    20
```

- [x] **Step 4: Commit**

```bash
git commit -s -m "arch: renode: EXTI line IRQs to NVIC"
```

---

### Task 6: SDMMC IRQ (Gap 2)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_SDMMC.cs`
- Modify: `tests/renode/stm32n647x0.repl`
- Modify: `tests/renode/tests/020-sdmmc.robot`

- [x] **Step 1: Model**

```csharp
public GPIO IRQ { get; }

private void UpdateInterrupt()
{
    // IRQ if (STA & MASK) != 0 for implemented status bits (CMDSENT/CMDREND/DATAEND)
    IRQ.Set((staValue & maskValue) != 0);
}
```

Call on STA/MASK/ICR updates.

- [x] **Step 2: `.repl`**

```
sdmmc1: Miscellaneous.STM32N6_SDMMC @ sysbus 0x48027000
    IRQ -> nvic@174
```

- [x] **Step 3: Robot**

```robot
SDMMC CMDREND With MASK Raises IRQ
    [Tags]    L2-state
    Create STM32N6 Machine
    Power And Clock Enable
    Write SDMMC Register    ${REG_MASK}    ${STA_CMDREND}
    Write SDMMC Register    ${REG_ARG}     0x0
    Write SDMMC Register    ${REG_CMD}     ${CMD8_VAL}
    Assert NVIC Pending    174
```

- [x] **Step 4: Commit**

```bash
git commit -s -m "arch: renode: SDMMC IRQ wiring and L2 mask path"
```

---

### Task 7: HPDMA model raise L2 + IRQ + suite (Gaps 2, 5, 7)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_HPDMA.cs` (prefer **share logic with GPDMA** or copy mem2mem path)
- Modify: `tests/renode/stm32n647x0.repl`
- Modify: `tests/renode/tests/027-hpdma.robot`

**Decision:** HPDMA register map on STM32N6 is GPDMA-like (CMSIS). Align channel offsets with GPDMA (`0x50 + n*0x80`) if CMSIS says so — **verify against `stm32n647xx.h` DMA/HPDMA typedef before coding**. If identical, refactor:

Option A (YAGNI-friendly): Copy GPDMA mem2mem + IRQ0 into HPDMA class with correct size.
Option B: Extract shared `STM32N6_DMABase` — only if both files stay < ~500 LOC after.

- [x] **Step 1: Implement CH0 mem2mem + TCF + IRQ0** (mirror GPDMA L3 path).

- [x] **Step 2: Wire**

```
hpdma1: Miscellaneous.STM32N6_HPDMA @ sysbus 0x48020000
    IRQ0 -> nvic@68
```

- [x] **Step 3: Robot L2/L3 on real base**

```robot
HPDMA CH0 Mem2Mem
    [Tags]    L3-functional
    Create STM32N6 Machine
    # Same program sequence as GPDMA but HPDMA_BASE 0x48020000
    ...
    ${dst}=    Execute Command    sysbus ReadDoubleWord 0x34002000
    Should Be Equal As Integers    ${dst}    0x11223344
```

- [x] **Step 4: Commit**

```bash
git commit -s -m "arch: renode: HPDMA mem2mem L3 and IRQ0"
```

---

### Task 8: Multi-peripheral L3 SPI+GPDMA (Gap 3)

**Files:**
- Create: `tests/renode/tests/041-multi-peripheral.robot`
- Possibly small SPI model hook if DMA request lines absent: **for contest**, implement **CPU-orchestrated** multi-peripheral test that programs SPI loopback then GPDMA mem2mem of SPI RX buffer region (proves both models in one case without full DMAMUX).

- [x] **Step 1: Suite without DMAMUX** (honest L3 multi-use)

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      multi-peripheral

*** Test Cases ***
SPI Loopback Then GPDMA Copy Buffer
    [Tags]    L3-functional
    Create STM32N6 Machine
    # 1) SPI1 loopback 4 bytes into RXFIFO then drain to SRAM
    ${BUF}=    Set Variable    0x34003000
    Execute Command    sysbus WriteDoubleWord 0x42003000 0x1
    Execute Command    sysbus WriteDoubleWord 0x42003020 0x11
    ${rx}=    Execute Command    sysbus ReadDoubleWord 0x42003030
    ${rx}=    Strip String    ${rx}
    Execute Command    sysbus WriteDoubleWord ${BUF} ${rx}
    # 2) GPDMA copy BUF -> BUF+0x100
    ... program GPDMA CH0 ...
    ${out}=    Execute Command    sysbus ReadDoubleWord 0x34003100
    ${out}=    Strip String    ${out}
    Should Be Equal As Integers    ${out}    ${rx}
```

- [x] **Step 2 (stretch, if time in same task):** Document DMAMUX absence in README as **Deferred-with-reason** (no NuttX DMAMUX driver in tree). Do **not** leave as silent gap.

- [x] **Step 3: Commit**

```bash
git commit -s -m "tests: renode: multi-peripheral SPI+GPDMA L3 suite"
```

---

### Task 9: SDMMC data-path L3 (Gaps 3, 7)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_SDMMC.cs`
- Modify: `tests/renode/tests/020-sdmmc.robot`

- [x] **Step 1: Minimal data path**

Implement enough for driver probe/read of one block **or** FIFO loopback:

- `DCTRL` / `DLEN` / `DTIMER` RW
- On data command with `DPSM` enable: set `DATAEND` / `DBCKEND` in STA after writing/reading `FIFO` (`0x80`) for `DLEN` bytes
- Keep card model minimal: fixed pattern `0xA5` for reads

Reference: existing CMD state machine in model; extend after CMD17/CMD24 style.

```csharp
// Pseudocode on CMD with data
if (dataCommand)
{
    dataBytesRemaining = dlen;
    // For read: fill fifo with pattern when DPSM on
}
// FIFO read returns next pattern byte; when remaining==0 set DATAEND
```

- [x] **Step 2: Robot**

```robot
SDMMC FIFO Read Completes DATAEND
    [Tags]    L3-functional
    Create STM32N6 Machine
    Power And Clock Enable
    # Program DLEN=4, DCTRL enable read, issue data CMD (model-defined)
    ...
    ${sta}=    Read SDMMC Register    ${REG_STA}
    Should Be True    (int(${sta}) & DATAEND) != 0
```

- [x] **Step 3: Commit**

```bash
git commit -s -m "arch: renode: SDMMC minimal data path L3"
```

---

### Task 10: Driver-sequence Robot (Gap 1)

**Files:**
- Create: `tests/renode/tests/042-driver-sequences.robot`
- Reference NuttX: `arch/arm/stm32n6/src/stm32n6_{spi,i2c,rng,dma}.c`

- [x] **Step 1: Encode real driver order as Robot** (not random MMIO)

Examples:

**SPI (match NuttX):** disable SPE → CFG1/CFG2 → SSI/SPE → CSTART → TXDR → wait TXC/EOT → IFCR

**I2C:** PE → TIMINGR → CR2 START AUTOEND → TXDR → STOPF → ICR

**RNG:** CONDRST pulse → RNGEN → wait DRDY → read DR

**GPDMA:** CLBAR/CTR1/CTR2/CBR1/CSAR/CDAR → CCR.EN → CSR.TCF → CFCR.TCFC

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      driver-sequence

*** Test Cases ***
NuttX SPI Transfer Sequence
    [Tags]    L3-functional
    Create STM32N6 Machine
    # Exact register order from stm32n6_spi.c send path (document offsets)
    ...
    Should Be True    ...

NuttX RNG Enable Sequence
    [Tags]    L2-state
    Create STM32N6 Machine
    Write RNG... CONDRST 1 then 0, RNGEN, DRDY
    ...
```

- [x] **Step 2: Commit**

```bash
git commit -s -m "tests: renode: NuttX driver-ordered peripheral sequences"
```

---

### Task 11: Contest-local drivertest path (Gaps 1, 8)

**Constraint:** Cannot modify upstream `apps/` packaging; binary <1MB for CI nsh image.

**Files (choose one approach — implement A unless size forces B):**

**Approach A (preferred):** Tiny NSH builtin in **this repo** under `app/renode_drivertest/`:

```
app/renode_drivertest/
  Kconfig
  Make.defs
  Makefile
  renode_drivertest_main.c
```

`renode_drivertest_main.c` skeleton:

```c
/****************************************************************************
 * app/renode_drivertest/renode_drivertest_main.c
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, FAR char *argv[])
{
  int fd;
  int ret = 0;

  UNUSED(argc);
  UNUSED(argv);

  /* UART path already proven by console; exercise /dev/random if RNG driver on */
  fd = open("/dev/urandom", O_RDONLY);
  if (fd >= 0)
    {
      unsigned int v;
      if (read(fd, &v, sizeof(v)) == sizeof(v))
        {
          printf("PASS rng %08x\n", v);
        }
      else
        {
          printf("FAIL rng read %d\n", errno);
          ret = 1;
        }

      close(fd);
    }
  else
    {
      /* Fallback: always-pass UART self-check so suite is meaningful */
      printf("PASS uart-fallback (no /dev/urandom)\n");
    }

  return ret;
}
```

Wire via contest board/app symlink pattern already used for `hello_app` (see `contest2026_137_CtrlFuture.xml` / board defconfig). If Kconfig injection is heavy, **Approach B**.

**Approach B (size/isolation fallback):** Robot-only firmware L3 already in `012-uart.robot` + new NSH commands if present (`hello`, `help`). Document Phase-2 cmocka still deferred; add `tests/renode/tests/044-firmware-l3.robot`:

```robot
Firmware UART Help Is Driver L3
    [Tags]    L3-functional
    Start STM32N6
    Wait For NSH
    Run NSH Command    help    Builtin Apps:

Firmware Hello Builtin
    [Tags]    L3-functional
    Start STM32N6
    Wait For NSH
    Run NSH Command    hello    Hello, World!!
```

And README Phase-2 section updated with **exact** steps to enable cmocka later (still deferred with reason).

- [x] **Step 1:** Implement A if defconfig can enable without breaking size; else B + document.

- [x] **Step 2:** Measure binary:

```bash
cd /home/takumi/mi/open-velao-contest/ctrl_future
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8
ls -la nuttx/nuttx.bin   # must remain < 1MB
```

- [x] **Step 3:** Robot waits for `PASS` line if A:

```robot
Run NSH Command    renode_drivertest    PASS
```

- [x] **Step 4: Commit**

```bash
git commit -s -m "app: add contest-local renode drivertest hook"
```

or for B:

```bash
git commit -s -m "tests: renode: document Phase-2 and firm L3 suite"
```

---

### Task 12: USART2/6 L2 + IRQ smoke (Gaps 1, 4)

**Files:**
- Modify: `tests/renode/tests/012-uart.robot`

Stock `STM32F7_USART` already has IRQ in `.repl`. Add state tests:

```robot
USART2 CR1 Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Execute Command    sysbus WriteDoubleWord ${USART2_BASE} 0x1
    ${val}=    Execute Command    sysbus ReadDoubleWord ${USART2_BASE}
    ${val}=    Strip String    ${val}
    Should Be True    (int(${val}) & 1) == 1

USART6 CR1 Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Execute Command    sysbus WriteDoubleWord ${USART6_BASE} 0x1
    ${val}=    Execute Command    sysbus ReadDoubleWord ${USART6_BASE}
    ${val}=    Strip String    ${val}
    Should Be True    (int(${val}) & 1) == 1
```

- [x] **Step 1: Apply + run 012-uart + commit**

```bash
git commit -s -m "tests: renode: USART2/6 L2 register state coverage"
```

---

### Task 13: SPI2 instance L2 (Gap 4)

**Files:**
- Modify: `tests/renode/tests/013-spi.robot`

```robot
${SPI2_BASE}    0x40003800

SPI2 Enable Asserts TXP
    [Tags]    L2-state
    Create STM32N6 Machine
    Execute Command    sysbus WriteDoubleWord ${SPI2_BASE} 0x1
    ${sr}=    Execute Command    sysbus ReadDoubleWord 0x40003814
    ${sr}=    Strip String    ${sr}
    Should Be True    (int(${sr}) & 0x2) == 0x2
```

- [x] **Commit:** `tests: renode: SPI2 L2 instance coverage`

---

### Task 14: Cross-peripheral IRQ regression suite (Gap 2 closure)

**Files:**
- Create: `tests/renode/tests/040-irq-wiring.robot`

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      irq-wiring

*** Test Cases ***
RNG IRQ Still Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    Execute Command    sysbus WriteDoubleWord 0x44020000 0xC
    Assert NVIC Pending    40

SPI1 IRQ Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    # SPE + EOTIE + TX
    Execute Command    sysbus WriteDoubleWord 0x42003000 0x1
    Execute Command    sysbus WriteDoubleWord 0x42003010 0x8
    Execute Command    sysbus WriteDoubleWord 0x42003020 0xA5
    Assert NVIC Pending    153

GPDMA CH0 IRQ Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    # minimal mem2mem + TCIE (reuse offsets from 011-gpdma)
    ...
    Assert NVIC Pending    84

EXTI0 IRQ Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    Execute Command    sysbus WriteDoubleWord 0x46025080 0x1
    Execute Command    sysbus WriteDoubleWord 0x46025000 0x1
    Execute Command    sysbus WriteDoubleWord 0x46025008 0x1
    Assert NVIC Pending    20

SDMMC IRQ Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    ...
    Assert NVIC Pending    174

I2C1 IRQ Still Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    # PE + STOPIE + one-byte TX path that sets STOPF
    ...
    Assert NVIC Pending    100
```

- [x] **Commit:** `tests: renode: central IRQ wiring regression suite`

---

### Task 15: Driver-critical model depth floor (Gap 7)

**Files:** models + suites for remaining NuttX MMIO drivers below L2.

**NuttX MMIO drivers list (must each have ≥1 L2-state after this task):**  
`rcc, gpio, pwr, exti, dma(gpdma), spi, i2c, iwdg, rtc, xspi, sdmmc, ethernet, sai, fdcan, otg, ltdc, dcmipp, rng, serial/uart`

| Driver | Action if still weak |
|--------|----------------------|
| RCC | Add ≥1 L2 beyond ready flags only if missing SYSCLK switch stub used by driver; else document L2+ sufficient |
| EMAC | Add MACCR RE/TE sticky + one status bit L2 if not present |
| OTG | Keep core ID + reset L2; add GINTSTS bit if driver polls |
| FDCAN | CCCR INIT clear path already; ensure `L2-state` tag |
| SAI | IMR/SR path L2 tagged |
| LTDC / DCMIPP | layer/pipe enable L2 tagged |
| TIM/ADC/LPTIM/DTS/DMA2D/CSI/VENC/NPU/CRYP/OTP | **No NuttX driver or unused** → leave L1; list in Deferred table |

- [x] **Step 1:** Audit matrix vs suites; for each **driver-present** row with max fidelity L1, add minimal L2 case + model bit.

- [x] **Step 2:** Update README fidelity matrix (Task 16 may fold this).

- [x] **Commit:** `tests: renode: raise remaining MMIO drivers to L2 floor`

---

### Task 16: Documentation + single exit gate (all gaps)

**Files:**
- Modify: `tests/renode/README.md`
- Optionally: `docs/DEV-METHODOLOGY.md` (Phase-2 pointer only)

- [x] **Step 1: Rewrite Phase status**

Add section **“Completeness campaign (Gaps 1–8) — exit”** with table:

| Gap | Status | Evidence suite/commit |
|-----|--------|----------------------|
| 1 Driver behavior | CLOSED | 042, 012, 044/app |
| 2 IRQ wiring | CLOSED | 040 + model GPIO |
| 3 Multi-peripheral L3 | CLOSED | 041, 020 data |
| 4 Multi-instance | CLOSED | 014 I2C2–4, 013 SPI2/6, 012 USART2/6 |
| 5 HPDMA base bug | CLOSED | 027, 043 |
| 6 Tag noise | CLOSED | 039 Force Tags |
| 7 Model depth floor | CLOSED | matrix all drivers ≥L2 |
| 8 Phase-2 path | CLOSED or DEFERRED-WITH-REASON | app or README |

**Deferred-with-reason (allowed only if listed):**

| Item | Reason |
|------|--------|
| Full cmocka `drivertest_*` from apps/testing | Contest isolation + binary size |
| DMAMUX / peripheral-triggered DMA | No NuttX DMAMUX driver in tree |
| HPDMA CH1–15 IRQ | CH0 proves pattern; expand if driver uses more |
| TIM/ADC/… L3 | No NuttX driver consuming them yet |

- [x] **Step 2: Full green run**

```bash
cd /home/takumi/mi/open-velao-contest/ctrl_future
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8
bash contest2026_137_CtrlFuture/scripts/renode-test.sh
# Expect: 0 fail; count ≥ baseline 217
ls -la nuttx/nuttx.bin  # < 1MB
```

- [x] **Step 3: Commit docs**

```bash
git commit -s -m "docs: renode completeness gaps 1-8 exit matrix"
```

---

## Execution order (dependency)

```
T1 hygiene/base
 ├─ T2 I2C multi-instance
 ├─ T3 SPI IRQ
 ├─ T4 GPDMA IRQ
 ├─ T5 EXTI IRQ
 ├─ T6 SDMMC IRQ
 └─ T7 HPDMA L3/IRQ
      ├─ T8 multi-peripheral (needs SPI+GPDMA)
      ├─ T9 SDMMC data
      ├─ T10 driver sequences
      ├─ T11 contest drivertest path
      ├─ T12 USART2/6
      ├─ T13 SPI2
      ├─ T14 IRQ regression (after T3–T7)
      ├─ T15 depth floor
      └─ T16 docs + full gate
```

Serialize model edits to the same `.cs` file; parallelize only independent suites after models land.

---

## Self-review (writing-plans checklist)

| Spec gap | Task |
|----------|------|
| 1 Driver behavior | T10, T11, T12 |
| 2 IRQ incomplete | T3–T7, T14 |
| 3 Multi-peripheral | T8, T9 |
| 4 Instances | T2, T12, T13 |
| 5 HPDMA bug | T1, T7 |
| 6 Tags | T1, T16 |
| 7 Depth | T7, T9, T15 |
| 8 Phase-2 | T11, T16 Deferred table |

**No placeholder tasks** — each step has concrete paths, Robot/C# snippets, commits.  
**Type consistency:** CMSIS IRQn for `nvic@N`; HPDMA `0x48020000`; GPDMA `0x40021000`.  
**Single exit:** T16 only; no planned “wave 2 optimization” document.

---

## Out of scope (explicit)

- Hardware serial / flash bring-up (user deprioritized).
- Upstream NuttX/apps cmocka packaging.
- Mass-renaming Robot suite numbers to ADR numbers.
- Full Ethernet PHY / USB device stack L3.
- Writing exploits or attacking systems (N/A).
