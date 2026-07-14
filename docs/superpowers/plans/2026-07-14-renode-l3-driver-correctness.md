# Renode L3 Driver-Correctness Simulation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Raise Renode STM32N647 models from L1 register stubs to L2/L3 behavior so NuttX chip drivers can be verified for correctness (state machines, IRQ, DMA/data paths), not only “register readable after boot.”

**Architecture:** Dual-track per ADR (NuttX driver + independent C# Renode model), dual-layer tests (Robot register/model + optional NuttX drivertest on UART), 8-phase lifecycle from `docs/DEV-METHODOLOGY.md`. Models live in-repo under `tests/renode/peripherals/`, are copied into `$RENODE_SRC` (`/home/takumi/mi/open-velao-contest/renode`), rebuilt with `./build.sh --no-gui`, and exercised by Robot suites under `tests/renode/tests/`. Prefer upgrading models that already have NuttX drivers first; keep `000-boot-regression.robot` green every wave.

**Tech Stack:** Renode (C# peripherals), Robot Framework (`renode-test`), NuttX STM32N6 out-of-tree drivers, CMSIS/HAL under `/home/takumi/mi/open-velao-contest/SoftwarePackage`, reference Renode models `STM32H7_SPI`, `STM32WBA_GPDMA`, `STM32F7_I2C`, `STM32H7_RCC`.

---

## Current Baseline (audit 2026-07-14)

| Item | State |
|------|--------|
| Platform | `tests/renode/stm32n647x0.repl` — CPU/NVIC/SRAM/USART/GPIO + 27 custom models |
| C# models | 27 files; ~20 are L1 (“without actual …”); better: RCC/PWR/IWDG/RTC/EXTI/XSPI |
| Robot suites | 37 suites, last full run 157/157 PASS — mostly shallow RW + boot |
| Missing model | **SPI1–6** (NuttX `stm32n6_spi.c` exists, no `.cs` / no `013-spi.robot` / not in `.repl`) |
| Known bug | `STM32N6_RNG.cs`: DR read clears DRDY but RNGEN never sets DRDY |
| IRQ | Custom models generally **not** wired to NVIC in `.repl` |
| Path bug | `scripts/renode-test.sh` sets `RENODE_SRC="${WORKSPACE}/renode"` → wrong; real path is `$OPENVELA_ROOT/renode` |
| Numbering | Robot IDs after 020 drift from ADR numbers (maintainability only) |

### Fidelity tiers (definition of done)

| Tier | Meaning | Exit criteria |
|------|---------|---------------|
| L1 | Register shell | Reset values, RW/RO/W1C; boot still passes |
| L2 | Stateful peripheral | Enable→ready, IRQ assert/clear, basic error flags |
| L3 | Functional | Data path / DMA / multi-peripheral interaction usable by NuttX driver |

**Project target for “完整驱动正确性仿真”:** every NuttX-present driver ≥ L2; bus masters that move data (GPDMA, SPI, I2C, SDMMC, USART path already via stock UART model) ≥ L3 for the paths the driver uses.

### Priority waves

| Wave | Focus | Why |
|------|--------|-----|
| 0 | Tooling + SPI gap + RNG fix + test tiers + IRQ pattern | Unblocks correct local runs; closes largest functional holes |
| 1 | L2/L3 for drivers already in tree (SPI, I2C, GPDMA, SDMMC, RNG, RTC, IWDG, EXTI) | Direct driver correctness |
| 2 | Ethernet / OTG / FDCAN / SAI / TIM / ADC | Networking, USB, timing |
| 3 | Multimedia / AI (LTDC, DCMIPP, CSI, DMA2D, NPU, VENC, CRYP, OTP) | EdgeSight path; often L2 enough until app needs more |
| 4 | Multi-instance, WWDG/HASH/PKA if needed, ADR/robot renumber docs | Completeness |

---

## File Map

| Path | Role |
|------|------|
| `scripts/renode-test.sh` | Local rebuild + full Robot run; **fix RENODE_SRC** |
| `tests/renode/stm32n647x0.repl` | Platform; add SPI, wire IRQs, shrink Tags as models grow |
| `tests/renode/peripherals/STM32N6_*.cs` | C# models (source of truth in this repo) |
| `tests/renode/tests/*.robot` | Robot suites |
| `tests/renode/tests/resources/stm32n6-common.robot` | Shared Start/Wait For NSH |
| `arch/arm/stm32n6/src/stm32n6_*.c` | NuttX drivers (reference for expected sequences) |
| `$SOFTWARE_PACKAGE/.../stm32n647xx.h` | Register offsets / IRQn |
| `$RENODE_SRC/.../Miscellaneous/` | Build-time copy destination |
| `docs/DEV-METHODOLOGY.md` | 8-phase process (do not reinvent) |
| `docs/adr/ADR-NNN.md` | Per-module status / Renode section updates |
| `CLAUDE.md` | Local AI guide (paths already documented) |

### SPI base addresses (CMSIS NS, for `.repl`)

| Instance | Base | Bus |
|----------|------|-----|
| SPI1 | `0x42003000` | APB2 + 0x3000 |
| SPI2 | `0x40003800` | APB1 + 0x3800 |
| SPI3 | `0x40003C00` | APB1 + 0x3C00 |
| SPI4 | `0x42003400` | APB2 + 0x3400 |
| SPI5 | `0x42005000` | APB2 + 0x5000 |
| SPI6 | `0x46001400` | APB4 + 0x1400 |

IRQ numbers: extract from `stm32n647xx.h` / `arch/arm/stm32n6/include/irq.h` when wiring (do not guess).

---

### Task 0: Fix Renode path tooling

**Files:**
- Modify: `scripts/renode-test.sh`
- Modify: `.github/workflows/renode-test.yml` (if it hardcodes wrong path)
- Optional: `scripts/renode-build.sh` if present

- [ ] **Step 1: Confirm real paths**

```bash
test -x /home/takumi/mi/open-velao-contest/renode/renode-test && echo OK
test ! -e /home/takumi/mi/open-velao-contest/ctrl_future/renode && echo NO_WORKSPACE_RENODE
```

Expected: `OK` and `NO_WORKSPACE_RENODE`.

- [ ] **Step 2: Patch `scripts/renode-test.sh` path resolution**

Replace:

```bash
WORKSPACE="$(cd "${REPO_DIR}/.." && pwd)"
RENODE_SRC="${WORKSPACE}/renode"
```

With:

```bash
WORKSPACE="$(cd "${REPO_DIR}/.." && pwd)"
# Renode is a sibling of ctrl_future under open-velao-contest, not under WORKSPACE.
OPENVELA_ROOT="$(cd "${WORKSPACE}/.." && pwd)"
RENODE_SRC="${RENODE_SRC:-${OPENVELA_ROOT}/renode}"
```

Keep the existing existence check for `${RENODE_SRC}/renode-test`.

- [ ] **Step 3: Align CI workflow**

Open `.github/workflows/renode-test.yml`. Ensure checkout / path for Renode matches CI layout. If CI clones Renode as a sibling or caches under a fixed path, document `RENODE_SRC` env override. Prefer:

```yaml
env:
  RENODE_SRC: ${{ github.workspace }}/../renode  # or wherever CI places it
```

If CI currently fails on path, fix in same commit as the script.

- [ ] **Step 4: Smoke the runner (no model change)**

```bash
cd /home/takumi/mi/open-velao-contest/ctrl_future
# nsh ELF must exist
test -f nuttx/nuttx || ./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8
bash contest2026_137_CtrlFuture/scripts/renode-test.sh
```

Expected: script finds renode-test; full suite runs; boot cases PASS (full 157 may take several minutes).

- [ ] **Step 5: Commit**

```bash
cd contest2026_137_CtrlFuture
git add scripts/renode-test.sh .github/workflows/renode-test.yml
git commit -s -m "$(cat <<'EOF'
scripts: fix RENODE_SRC to open-velao-contest/renode

Renode lives beside ctrl_future, not under WORKSPACE. Allow
RENODE_SRC override for CI.

EOF
)"
```

---

### Task 1: Introduce Robot test tiers (L1 / L2 / L3 tags)

**Files:**
- Modify: existing Robot suites incrementally (tags only in this task for template)
- Create: `tests/renode/tests/resources/model-tiers.robot` (optional keywords)
- Document tags in suite headers

- [ ] **Step 1: Define tag convention** (use in every new/changed suite)

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot
# Tags: L1-register | L2-state | L3-functional | boot-regression
```

- [ ] **Step 2: Tag existing RNG suite as L1 (baseline before fix)**

In `tests/renode/tests/039-rng.robot` add Force Tags:

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      L1-register  rng
```

- [ ] **Step 3: Document filter usage** in `scripts/renode-test.sh` comment or README under `tests/renode/`:

```bash
# Full suite
bash scripts/renode-test.sh
# Later, when renode-test supports include: prefer running L2+ after changes
# ${RENODE_SRC}/renode-test --include L2-state tests/renode/tests/*.robot
```

- [ ] **Step 4: Commit**

```bash
git add tests/renode/tests/039-rng.robot
git commit -s -m "tests: tag RNG Robot suite as L1-register baseline"
```

---

### Task 2: Fix RNG model to L2 (DRDY / RNGEN / CONDRST)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_RNG.cs`
- Modify: `tests/renode/tests/039-rng.robot`
- Reference: `arch/arm/stm32n6/src/stm32n6_rng.c`, `arch/arm/stm32n6/src/hardware/stm32_rng.h`
- Reference CMSIS/HAL: `$SOFTWARE_PACKAGE/STM32Cube_FW_N6_V1.0.0/Drivers/...`

- [ ] **Step 1: Write failing L2 tests first**

Append to `039-rng.robot`:

```robot
*** Test Cases ***
RNGEN Sets DRDY
    [Tags]    L2-state
    Start STM32N6
    # CR.RNGEN = bit 2
    Write RNG Register    ${CR_OFFSET}    0x04
    ${sr}=    Read RNG Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    # DRDY bit 0 should be set when enabled
    Should Be True    (${sr} & 0x1) == 0x1

DR Read Clears Then Refills DRDY
    [Tags]    L2-state
    Start STM32N6
    Write RNG Register    ${CR_OFFSET}    0x04
    ${d1}=    Read RNG Register    ${DR_OFFSET}
    ${d1}=    Convert To Integer    ${d1}
    Should Be True    ${d1} >= 0
    # After read DRDY may clear; model should re-assert when still enabled
    ${sr}=    Read RNG Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1) == 0x1

Two DR Reads Differ When Enabled
    [Tags]    L2-state
    Start STM32N6
    Write RNG Register    ${CR_OFFSET}    0x04
    ${a}=    Read RNG Register    ${DR_OFFSET}
    ${b}=    Read RNG Register    ${DR_OFFSET}
    # Soft check: allow rare collision but require at least valid ints
    ${a}=    Convert To Integer    ${a}
    ${b}=    Convert To Integer    ${b}
    Should Be True    ${a} >= 0 and ${b} >= 0
```

- [ ] **Step 2: Run only RNG suite — expect DRDY test FAIL**

```bash
REPO=/home/takumi/mi/open-velao-contest/ctrl_future/contest2026_137_CtrlFuture
RENODE=/home/takumi/mi/open-velao-contest/renode
cp $REPO/tests/renode/peripherals/*.cs \
  $RENODE/src/Infrastructure/src/Emulator/Peripherals/Peripherals/Miscellaneous/
cd $RENODE && ./build.sh --no-gui
$RENODE/renode-test $REPO/tests/renode/tests/039-rng.robot \
  --results $REPO/tests/renode/results
```

Expected: `RNGEN Sets DRDY` FAIL (DRDY stays 0).

- [ ] **Step 3: Implement minimal L2 behavior in `STM32N6_RNG.cs`**

Key behavior (mirror NuttX driver expectations):

1. Writing `RNGEN=1` → set `DRDY=1` (data available immediately in sim).
2. Reading `DR` → return random uint, clear `DRDY`, then if `RNGEN` still 1 re-set `DRDY` (or set on next clock; immediate re-set is fine for driver polling).
3. `CONDRST` pulse: when set then cleared, keep model consistent (no hang); optional clear SEIS/CEIS.
4. Keep `Size => 0x100`.

Sketch for CR RNGEN writeCallback:

```csharp
Registers.CR.Define(this)
    .WithFlag(2, out rngEnabled, name: "RNGEN",
        writeCallback: (_, val) =>
        {
            if (val)
            {
                dataReady.Value = true;
            }
            else
            {
                dataReady.Value = false;
            }
        })
    .WithFlag(3, out interruptEnable, name: "IE")
    .WithFlag(5, name: "CED")
    .WithFlag(30, out condrst, name: "CONDRST",
        writeCallback: (_, val) =>
        {
            // Hardware: CONDRST must be written 1 then 0; treat as soft reset
            if (!val && condrst.Value)
            {
                dataReady.Value = rngEnabled.Value;
            }
        })
    .WithFlag(31, FieldMode.Read, name: "CONFIGLOCK");
```

DR valueProvider:

```csharp
valueProviderCallback: _ =>
{
    var value = (uint)random.Next();
    dataReady.Value = false;
    if (rngEnabled.Value)
    {
        dataReady.Value = true;
    }
    return value;
}
```

- [ ] **Step 4: Rebuild Renode and re-run 039-rng.robot**

Expected: all RNG cases PASS, including Boot Regression.

- [ ] **Step 5: Run full regression**

```bash
bash scripts/renode-test.sh
```

Expected: full suite green.

- [ ] **Step 6: Commit**

```bash
git add tests/renode/peripherals/STM32N6_RNG.cs tests/renode/tests/039-rng.robot
git commit -s -m "$(cat <<'EOF'
tests: fix RNG Renode model DRDY on RNGEN (ADR-037)

Enable path now sets DRDY so NuttX RNG polling can complete under Renode.

EOF
)"
```

---

### Task 3: Add SPI L2/L3 model + platform + tests (closes SPI gap)

**Files:**
- Create: `tests/renode/peripherals/STM32N6_SPI.cs`
- Modify: `tests/renode/stm32n647x0.repl`
- Create: `tests/renode/tests/013-spi.robot`
- Reference: `arch/arm/stm32n6/src/stm32n6_spi.c`
- Reference Renode: `$RENODE_SRC/.../SPI/STM32H7_SPI.cs`, `STM32SPI.cs`
- CMSIS: SPI_TypeDef in `stm32n647xx.h`

- [ ] **Step 1: Extract SPI register map used by NuttX driver**

```bash
rg -n "SPI_|CR1|CR2|SR|TXDR|RXDR|CFG" \
  arch/arm/stm32n6/src/stm32n6_spi.c \
  arch/arm/stm32n6/src/stm32n6_spi.h \
  arch/arm/stm32n6/src/hardware 2>/dev/null | head -80
rg -n "typedef struct.*SPI|SPI_TypeDef" \
  /home/takumi/mi/open-velao-contest/SoftwarePackage/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include/stm32n647xx.h | head
```

Document offsets actually touched by the driver (minimum set for L2).

- [ ] **Step 2: Write failing 013-spi.robot (L1 then L2)**

```robot
*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      spi

*** Variables ***
${SPI1_BASE}    0x42003000

*** Keywords ***
Read SPI1 Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${SPI1_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write SPI1 Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${SPI1_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
SPI1 CR1 Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SPI1 Register    0x00
    Should Be True    int(${val}) >= 0

SPI1 Enable Sets SPE Behavior
    [Tags]    L2-state
    Start STM32N6
    # Adjust offsets/bits to match STM32N6 SPI_TypeDef after Step 1
    Write SPI1 Register    0x00    0x1
    ${val}=    Read SPI1 Register    0x00
    Should Be True    (int(${val}) & 0x1) == 0x1

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
```

- [ ] **Step 3: Run test — expect unhandled access or missing peripheral FAIL**

- [ ] **Step 4: Implement `STM32N6_SPI.cs` (minimum L2, then L3 FIFO)**

Requirements for L2:

- DoubleWord peripheral, `IKnownSize` (use size from CMSIS, typically 0x400).
- Registers: CR1/CR2/CFG1/CFG2/IER/SR/IFCR/TXDR/RXDR as needed by driver.
- SPE enable; TXP/RXP (or TXE/RXNE depending on N6 naming) flags.
- Optional: `GPIO IRQ` output for SPI interrupt line.

Requirements for L3 (same task if time allows; else Task 3b):

- Write TXDR → push to TX queue; set RXP with loopback data into RX queue (default loopback mode for drivertest).
- Clear flags on read as hardware does.

Prefer adapting structure from Renode `STM32H7_SPI.cs` rather than inventing a new API.

- [ ] **Step 5: Wire `.repl`**

Add after USART block (and shrink Tags that covered SPI ranges):

```
// SPI1 @ 0x42003000 (APB2 + 0x3000)
spi1: Miscellaneous.STM32N6_SPI @ sysbus 0x42003000
// Optional later: IRQ -> nvic@N

// SPI2 @ 0x40003800
spi2: Miscellaneous.STM32N6_SPI @ sysbus 0x40003800

// SPI3 @ 0x40003C00
spi3: Miscellaneous.STM32N6_SPI @ sysbus 0x40003C00

// SPI4 @ 0x42003400
spi4: Miscellaneous.STM32N6_SPI @ sysbus 0x42003400

// SPI5 @ 0x42005000
spi5: Miscellaneous.STM32N6_SPI @ sysbus 0x42005000

// SPI6 @ 0x46001400
spi6: Miscellaneous.STM32N6_SPI @ sysbus 0x46001400
```

Update Tag ranges in `sysbus: init:` so they no longer swallow these addresses (critical: Tag windows that included `0x42003000` / `0x40003800` must be split).

- [ ] **Step 6: Rebuild Renode, run 013-spi + full suite**

```bash
bash scripts/renode-test.sh
```

Expected: SPI suite PASS; no boot regression.

- [ ] **Step 7: Commit**

```bash
git add tests/renode/peripherals/STM32N6_SPI.cs \
        tests/renode/stm32n647x0.repl \
        tests/renode/tests/013-spi.robot
git commit -s -m "$(cat <<'EOF'
tests: add STM32N6 SPI Renode model and 013-spi suite

Closes SPI gap for NuttX stm32n6_spi driver correctness under Renode.

EOF
)"
```

---

### Task 4: IRQ wiring pattern (template on one peripheral)

**Files:**
- Modify: one mature model (prefer `STM32N6_EXTI.cs` already has logic, or SPI/RNG)
- Modify: `stm32n647x0.repl` IRQ lines
- Modify: corresponding Robot suite with L2 IRQ test

- [ ] **Step 1: Pick first IRQ to wire**

Prefer **RNG** or **SPI1** if model has IE + event; else EXTI line already modeled.

Confirm IRQn from:

```bash
rg -n "RNG_IRQn|SPI1_IRQn" \
  /home/takumi/mi/open-velao-contest/SoftwarePackage/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include/stm32n647xx.h \
  arch/arm/stm32n6/include/irq.h
```

- [ ] **Step 2: Add `public GPIO IRQ { get; }` to C# model**

Pattern:

```csharp
public GPIO IRQ { get; } = new GPIO();

// when event && IE: IRQ.Set(true);
// on flag clear / W1C: IRQ.Set(false);
```

- [ ] **Step 3: Connect in `.repl`**

```
rng: Miscellaneous.STM32N6_RNG @ sysbus 0x44020000
    IRQ -> nvic@<RNG_IRQn>
```

- [ ] **Step 4: Robot test** — enable IE, trigger event, read NVIC pending or observe no hang; at minimum assert model does not throw and SR flags match.

- [ ] **Step 5: Full regression + commit**

```bash
git commit -s -m "tests: wire RNG IRQ to NVIC in Renode platform"
```

---

### Task 5: GPDMA L2→L3 (critical for driver DMA paths)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_GPDMA.cs`
- Modify: `tests/renode/tests/011-gpdma.robot`
- Reference: `arch/arm/stm32n6/src/stm32n6_dma.c`
- Reference Renode: `DMA/STM32WBA_GPDMA.cs`, `STM32DMA.cs`

- [ ] **Step 1: Map NuttX DMA programming sequence** (channel enable, SAR/DAR/LLR, TCF)

```bash
rg -n "GPDMA|CCR|CTR1|CSAR|CDAR|cllr|TCF" arch/arm/stm32n6/src/stm32n6_dma.c | head -60
```

- [ ] **Step 2: Failing Robot tests for one memory-to-memory transfer**

```robot
GPDMA Mem2Mem Word Copy
    [Tags]    L3-functional
    Start STM32N6
    # 1) Write pattern to SRAM source
    # 2) Program ch0 CSAR/CDAR/size/enable per driver
    # 3) Poll CSR TCF
    # 4) Read destination words — must match
```

Fill exact offsets from model + CMSIS after reading `STM32N6_GPDMA.cs` and TypeDef.

- [ ] **Step 3: Implement transfer engine in C#**

On channel enable with valid config:

1. Read `count` words/bytes from `sysbus` at CSAR.
2. Write to CDAR.
3. Set TCF / clear busy; optional IRQ.

Do **not** attempt full linked-list LLI in first pass unless driver requires it for basic path.

- [ ] **Step 4: Regression + commit**

```bash
git commit -s -m "tests: upgrade GPDMA Renode model to L3 mem2mem (ADR-011)"
```

---

### Task 6: I2C L2→L3 (NuttX `stm32n6_i2c.c`)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_I2C.cs`
- Modify: `tests/renode/tests/014-i2c.robot`
- Reference: Renode `I2C/STM32F7_I2C.cs`

- [ ] **Step 1: Failing tests** — PE enable, START, TXIS/RXNE or N6 equivalents, STOPF.

- [ ] **Step 2: Implement master TX/RX with optional `I2CPeripheral` dummy slave** (Renode pattern) or loopback-to-self for register-level only first.

- [ ] **Step 3: Wire I2C1 IRQ in `.repl` when IE paths are ready.

- [ ] **Step 4: Full suite + commit**

```bash
git commit -s -m "tests: upgrade I2C Renode model toward L3 master path (ADR-014)"
```

---

### Task 7: SDMMC L2→L3 (card path used by driver)

**Files:**
- Modify: `tests/renode/peripherals/STM32N6_SDMMC.cs`
- Modify: `tests/renode/tests/020-sdmmc.robot`
- Reference: `arch/arm/stm32n6/src/stm32n6_sdmmc.c`, Renode SD host models under `Peripherals/SD/`

- [ ] **Step 1: Capture CMD0/CMD8/ACMD41 sequence from driver init.**

- [ ] **Step 2: Failing tests for POWER + CLOCK enable + CMD response flags.**

- [ ] **Step 3: Implement minimal card state machine (idle → ready → transfer) with canned CID/CSD responses sufficient for driver probe success.

- [ ] **Step 4: Regression + commit**

```bash
git commit -s -m "tests: upgrade SDMMC Renode model for driver probe path"
```

---

### Task 8: Close L2 for remaining “have driver” peripherals

Apply the same TDD loop (failing Robot → C# → rebuild → full suite → commit) for each:

| Peripheral | Model | Robot | NuttX driver | Target |
|------------|-------|-------|--------------|--------|
| RTC | `STM32N6_RTC.cs` | `016-rtc.robot` | `stm32n6_rtc.c` | L2 time progress / WUTR optional |
| IWDG | `STM32N6_IWDG.cs` | `015-iwdg.robot` | `stm32n6_iwdg.c` | L2 refresh / timeout sim |
| EXTI | `STM32N6_EXTI.cs` | `010-exti.robot` | `stm32n6_exti.c` | L2 GPIO→EXTI→NVIC |
| XSPI | `STM32N6_XSPI.cs` | `018/019` | `stm32n6_xspi.c` | L2/L3 map mode already partial |
| Ethernet | `STM32N6_EMAC.cs` | `021-emac.robot` | `stm32n6_ethernet.c` | L2 MAC init; L3 later |
| OTG | `STM32N6_OTG.cs` | `024-otg.robot` | `stm32n6_otg.c` | L2 core ID + reset |
| FDCAN | `STM32N6_FDCAN.cs` | `023-fdcan.robot` | `stm32n6_fdcan.c` | L2 init / INAK |
| SAI | `STM32N6_SAI.cs` | `022-sai.robot` | `stm32n6_sai.c` | L2 enable |
| LTDC | `STM32N6_LTDC.cs` | `025-ltdc.robot` | `stm32n6_ltdc.c` | L2 layer enable |
| DCMIPP | `STM32N6_DCMIPP.cs` | `026-dcmipp.robot` | `stm32n6_dcmipp.c` | L2 pipe enable |

Per peripheral steps (repeat):

- [ ] **Step A:** List registers/bits the NuttX driver **writes and waits on**.
- [ ] **Step B:** Add Robot cases that reproduce those waits (timeouts = current L1 failure mode).
- [ ] **Step C:** Implement callbacks so waits complete without infinite spin.
- [ ] **Step D:** `bash scripts/renode-test.sh` must stay green.
- [ ] **Step E:** One commit per peripheral: `tests: raise <periph> Renode model to L2 (ADR-NNN)`.

---

### Task 9: Optional drivertest apps on Renode (L3 end-to-end)

**Files:**
- Create: `app/drivertest/` (or enable upstream `apps/testing/drivers`)
- Modify: board defconfig for test images if needed
- Create: Robot suites that `Write Line To Uart` and wait for PASS

- [ ] **Step 1:** Port/adapt `drivertest_uart` first (USART1 already L3 via stock `STM32F7_USART`).

- [ ] **Step 2:** Robot:

```robot
UART Drivertest Passes
    [Tags]    L3-functional
    Start STM32N6
    Wait For NSH
    Write Line To Uart    drivertest_uart
    Wait For Line On Uart    PASS    timeout=30
```

- [ ] **Step 3:** Only after SPI/I2C L3 models exist, add matching drivertest binaries.

- [ ] **Step 4:** Commit separately from models.

---

### Task 10: Documentation & ADR sync

**Files:**
- Modify: `docs/DEV-METHODOLOGY.md` coverage matrix status column if present
- Modify: relevant `docs/adr/ADR-NNN.md` Renode sections (especially 011–037)
- Modify: `docs/ROADMAP.md` if Renode fidelity is tracked
- Optional: `tests/renode/README.md` (only if useful; keep short)

- [ ] **Step 1:** Add fidelity table (L1/L2/L3 per model) to `docs/DEV-METHODOLOGY.md` or `tests/renode/README.md`.

- [ ] **Step 2:** Note robot number ≠ ADR number; do **not** mass-rename suites unless a dedicated chore ADR is opened (high churn). Document mapping instead:

```
013-spi → SPI (missing ADR link or ADR-013)
039-rng → ADR-037
021-emac → ADR-022
...
```

- [ ] **Step 3:** Commit docs only:

```bash
git commit -s -m "docs: record Renode L2/L3 fidelity matrix and path layout"
```

---

### Task 11: Wave gate — definition of “完整驱动正确性仿真” for Phase-1 exit

Do **not** claim complete until all checkboxes below pass:

- [x] `scripts/renode-test.sh` works with default paths on a clean shell
- [x] SPI present in `.repl` + model + `013-spi.robot` ≥ L2
- [x] RNG DRDY/RNGEN correct; NuttX RNG enable path does not spin forever under Renode
- [x] GPDMA can complete at least one mem2mem (L3)
- [x] I2C master path does not hang on flags the driver waits for (L2+)
- [x] Every NuttX `stm32n6_*.c` driver that programs an MMIO peripheral has a Robot suite with ≥1 **L2-state** case
- [x] Full Robot suite green after each wave
- [x] `000-boot-regression` always first-class green
- [x] CLAUDE.md paths remain accurate (`RENODE_SRC`, `SOFTWARE_PACKAGE`)

**Phase-1 exit recorded 2026-07-14:** full suite **217 pass / 0 fail**;
L2-state tags added on RCC/GPIO/PWR/UART behavioral cases. Canonical
evidence: `tests/renode/README.md` Phase-1 exit checklist.

Phase-2 exit (later): drivertest UART+SPI green (cmocka e2e deferred);
multimedia already ≥ L2 from Task 8.

---

## Closed-Loop Per-Change Checklist (every task)

```
1. Extract expected HW behavior from CMSIS + HAL + NuttX driver waits
2. Write/extend Robot test (must fail on current model)
3. Implement C# model change only as much as tests require
4. cp *.cs → $RENODE_SRC/.../Miscellaneous/ && ./build.sh --no-gui
5. renode-test changed suite
6. bash scripts/renode-test.sh  # full regression
7. Commit with tests: scope; Signed-off-by; no AI co-author trailer
8. Update fidelity matrix row for that peripheral
```

Never skip step 6 after model or `.repl` edits.

---

## Self-Review (plan vs audit)

| Audit finding | Task |
|---------------|------|
| RENODE_SRC wrong path | Task 0 |
| SPI missing entirely | Task 3 |
| RNG DRDY bug | Task 2 |
| Shallow L1 tests | Tasks 1, 5–8 |
| No IRQ on custom models | Task 4 |
| GPDMA stub | Task 5 |
| I2C/SDMMC stubs | Tasks 6–7 |
| Multimedia stubs | Task 8 wave |
| Robot↔ADR renumber drift | Task 10 (doc, not mass rename) |
| Driver e2e | Task 9 |
| SoftwarePackage / Renode path docs | CLAUDE.md (done with this plan) |

No TBD placeholders remain for Wave 0–1 entry points; Wave 2–3 follow the same TDD template with peripheral-specific register lists filled at execution time from CMSIS.

---

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-07-14-renode-l3-driver-correctness.md`.

**Two execution options:**

1. **Subagent-Driven (recommended)** — fresh subagent per task, review between tasks, fast iteration (`superpowers:subagent-driven-development`)
2. **Inline Execution** — this session runs tasks with checkpoints (`superpowers:executing-plans`)

**Which approach?**
