# Renode L2/L3 Driver Correctness

Platform: `stm32n647x0.repl`  
Models: `peripherals/*.cs`  
Suites: `tests/*.robot`

Canonical fidelity matrix and robot↔ADR map for this contest repo.
Do **not** mass-rename Robot suites to match ADR numbers — document the
mapping here instead.

## Path layout

| Variable / path | Absolute (this machine) | Role |
|-----------------|-------------------------|------|
| `OPENVELA_ROOT` | `/home/takumi/mi/open-velao-contest` | Top-level sibling root |
| `WORKSPACE` | `$OPENVELA_ROOT/ctrl_future` | NuttX build root (`build.sh`) |
| `RENODE_SRC` | `$OPENVELA_ROOT/renode` | Renode source + `renode-test` (**not** under `ctrl_future`) |
| `SOFTWARE_PACKAGE` | `$OPENVELA_ROOT/SoftwarePackage` | ST HAL / CMSIS / examples |
| CMSIS | `$SOFTWARE_PACKAGE/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include/stm32n647xx.h` | Register map source of truth |
| This tree | `$WORKSPACE/contest2026_137_CtrlFuture/tests/renode/` | `.repl`, C# models, Robot suites |

Firmware for Renode is the **nsh** (STM32N6) image, not `nsh-qemu`.

## Phase status

### Phase-1 (current wave gate)

Model L1/L2/L3 Robot suites cover Wave 0–3 peripherals used by the
STM32N6 NuttX drivers. Tags:

| Tag | Meaning |
|-----|---------|
| `L1-register` | Base/register readable |
| `L2-state` | Control/status bit behavior |
| `L3-functional` | End-to-end path under firmware / model functional path |
| `boot-regression` | Boot still reaches `nsh>` |

Always-green gate suites:

- `000-boot-regression.robot`
- `000-smoke.robot`
- `001-csharp-model.robot`

USART1 L3 is already proven by `012-uart.robot`:

- `USART1 Console Works` — NSH `help` over stock `STM32F7_USART`
- `USART1 Hello Builtin Works` — builtin `hello` output

SPI and I2C L3 models (`013-spi.robot`, `014-i2c.robot`) are ready
for future drivertest binding.

#### Phase-1 exit checklist (Task 11) — PASSED 2026-07-14

| Gate | Evidence |
|------|----------|
| `scripts/renode-test.sh` default paths | Resolves `RENODE_SRC=$OPENVELA_ROOT/renode` |
| SPI in `.repl` + model + ≥ L2 | `spi1`–`spi6` `STM32N6_SPI`; `013-spi` L2+L3 |
| RNG DRDY/RNGEN | `039-rng` L2 (DRDY) + IRQ path |
| GPDMA mem2mem L3 | `011-gpdma` L3-functional cases |
| I2C master flags L2+ | `014-i2c` L2 + L3 |
| Every MMIO NuttX driver ≥1 L2-state | See matrix; RCC/GPIO/PWR/UART tagged |
| Full Robot suite green | **217 pass / 0 fail** (`robot_output.xml`, 2026-07-14 snapshot; see Completeness Campaign and CMSIS Alignment Campaign below for later counts) |
| `000-boot-regression` green | First-class suite PASS |
| CLAUDE.md paths | `RENODE_SRC`, `SOFTWARE_PACKAGE` correct |

**MMIO driver scope:** `stm32n6_{rcc,gpio,pwr,exti,dma,spi,i2c,iwdg,rtc,xspi,sdmmc,ethernet,sai,fdcan,otg,ltdc,dcmipp,rng,lowputc,serial}.c`.  
**Excluded (not peripheral MMIO models):** `start`, `allocateheap`, `irq` (NVIC), `timerisr` (SysTick core).

### Phase-2 (deferred)

Full cmocka drivertest e2e on Renode is deferred:

- `drivertest_uart` (apps/testing/drivers)
- SPI/I2C drivertest binaries

**Rationale**

1. Contest isolation: only this repo may be modified; enabling
   upstream CMOCKA/TESTING_DRIVER_TEST needs apps/nuttx packaging
   outside the contest boundary.
2. Binary size: cmocka drivertest images risk exceeding the CI
   `<1MB` limit on the nsh image.
3. Coverage gap is small: USART1 console + builtin path already
   exercise the L3 UART stack under Renode.

When Phase-2 opens, add a tiny contest-local drivertest app (or a
dedicated test defconfig) and Robot cases that
`Write Line To Uart drivertest_*` then `Wait For Line On Uart PASS`.

## Fidelity matrix (Phase-1)

Max fidelity is the highest tier the model/suite currently supports
(not every case in the suite is that tier).

| Model | Robot suite(s) | ADR link | Max fidelity | Notes |
|-------|----------------|----------|--------------|-------|
| stock `STM32F7_USART` (USART1) | `012-uart` | ADR-002 / ADR-012 | **L3** | Console + hello builtin |
| `STM32N6_RCC` | `005-rcc` | ADR-005 | **L2+** | Clock ready flags |
| `STM32N6_PWR` | `009-pwr` | ADR-009 | L1/L2 | Voltage scaling bits |
| `STM32N6_EXTI` | `010-exti` | ADR-010 | **L2** | Soft-trigger IRQ path |
| `STM32N6_GPDMA` | `011-gpdma` | ADR-011 | **L3** | mem2mem copy |
| `STM32N6_SPI` | `013-spi` | ADR-013 | **L3** | TX/RX FIFO loopback |
| `STM32N6_I2C` | `014-i2c` | ADR-014 | **L3** | Master flags + mock slave path |
| `STM32N6_IWDG` | `015-iwdg` | ADR-015 | **L2** | KR refresh / timeout sim |
| `STM32N6_RTC` | `016-rtc` | ADR-016 | **L2** | INIT / WUTR-style waits |
| `STM32N6_XSPI` | `018-xspi`, `019-xspi-boot` | ADR-018 / ADR-019 | **L2** | Map / status |
| `STM32N6_SDMMC` | `020-sdmmc` | ADR-020 | **L3** | Probe/CMD path (no full card FS) |
| `STM32N6_EMAC` | `021-emac` | ADR-022 | **L2** | MAC init; robot# ≠ ADR# |
| `STM32N6_SAI` | `022-sai` | — (no dedicated ADR) | **L2** | CMSIS IMR/SR/CLRFR/DR |
| `STM32N6_FDCAN` | `023-fdcan` | ADR-024 | **L2** | CMSIS CCCR @ 0x18; NuttX CCCR map bug out of scope |
| `STM32N6_OTG` | `024-otg` | ADR-023 | **L2** | Core ID + reset; robot# ≠ ADR# |
| `STM32N6_LTDC` | `025-ltdc` | ADR-031 | **L2** | Layer CR @ 0x10C/0x20C (CMSIS N6) |
| `STM32N6_DCMIPP` | `026-dcmipp` | ADR-033 | **L2** | Pipe enable |
| `STM32N6_RNG` | `039-rng` | ADR-037 | **L2** | DRDY/RNGEN + IRQ; robot# ≠ ADR# |
| `STM32N6_HPDMA` | `027-hpdma`, `027-hpdma-upgrade` | ADR-025 | **L3** | mem2mem + IRQ; channel register offsets realigned to CMSIS `DMA_Channel_TypeDef` (CCR@+0x14, CTR2@+0x44, CSAR@+0x4C, CDAR@+0x50, ...), matching GPDMA's layout |
| `STM32N6_TIM` | `028-tim`, `029-tim2` | ADR-026 / ADR-027 | L1 | |
| `STM32N6_LPTIM` | `030-lptim` | ADR-028 | L1 | |
| `STM32N6_ADC` | `031-adc` | ADR-029 | **L2-state** | CFGR1/CFGR2/PCSEL/IER/JSQR/AWD1LTR/AWD1HTR; offsets realigned to CMSIS `ADC_TypeDef` (JSQR@0x4C, not the previous fictitious 0x70) |
| `STM32N6_DTS` | `032-dts` | ADR-030 | L1 | |
| `STM32N6_DMA2D` | `033-dma2d` | ADR-032 | L1 | |
| `STM32N6_CSI` | `034-csi` | ADR-034 | L1 | |
| `STM32N6_VENC` | `035-venc` | ADR-035 | L1 | |
| `STM32N6_NPU` | `036-npu` | ADR-036 | L1 | |
| `STM32N6_CRYP` | `037-cryp` | ADR-037 | L1 | Crypto block; RNG is separate model |
| `STM32N6_OTP` | `038-otp` | ADR-038 | L1 | |

**NuttX drivers with L2+ models after this wave:**  
exti, gpdma, hpdma, spi, i2c, iwdg, rtc, xspi, sdmmc, ethernet, sai,
otg, fdcan, ltdc, dcmipp, rng, uart (stock), rcc, pwr.

## Robot suite number ≠ ADR number

Robot suite prefixes were assigned as suites landed; they are **not**
guaranteed to equal the ROADMAP ADR number. Prefer this table over
filename arithmetic. Do not mass-rename suites (breaks history, CI
filters, and local muscle memory).

| Robot suite | ADR (ROADMAP) | Aligned? | Notes |
|-------------|---------------|----------|-------|
| `013-spi` | ADR-013 | yes | |
| `014-i2c` | ADR-014 | yes | |
| `021-emac` | ADR-022 | **no** | Ethernet GMAC |
| `022-sai` | — | n/a | SAI has no dedicated ROADMAP ADR |
| `023-fdcan` | ADR-024 | **no** | FDCAN |
| `024-otg` | ADR-023 | **no** | USB OTG HS (suite 024 vs ADR-023) |
| `025-ltdc` | ADR-031 | **no** | Older plans sometimes said 030; ROADMAP is 031 |
| `026-dcmipp` | ADR-033 | **no** | Camera pipeline |
| `039-rng` | ADR-037 | **no** | Crypto ADR includes RNG; suite 039 free of cryp 037 |

Aligned suites (robot prefix == ADR) for the common path:
`005-rcc`, `009-pwr`, `010-exti`, `011-gpdma`, `012-uart`, `013-spi`,
`014-i2c`, `015-iwdg`, `016-rtc`, `018-xspi`, `019-xspi-boot`,
`020-sdmmc`.

## Completeness Campaign (Gaps 1–8) — EXIT 2026-07-14

All eight Renode completeness gaps closed in a single campaign.

| Gap | Description | Status | Evidence |
|-----|-------------|--------|----------|
| 1 | Tests are mostly model MMIO, not driver behavior | **CLOSED** | `042-driver-sequences`, `012-uart`, `044-firmware-l3` |
| 2 | IRQ wiring incomplete | **CLOSED** | `040-irq-wiring` (6/6 IRQ tests); SPI1–6, GPDMA, EXTI, SDMMC, HPDMA, I2C1, RNG wired |
| 3 | L3 narrow; no multi-peripheral | **CLOSED** | `041-multi-peripheral` (SPI+GPDMA), `020-sdmmc` data path |
| 4 | Instance coverage thin | **CLOSED** | `014-i2c` I2C2–4, `013-spi` SPI2/SPI6, `012-uart` USART2/6 |
| 5 | Correctness bugs (HPDMA base) | **CLOSED** | `027-hpdma` on `0x48020000`, `043-base-audit` |
| 6 | Tag/matrix noise | **CLOSED** | `039-rng` Force Tags fixed; matrix matches reality |
| 7 | Uneven model depth | **CLOSED** | All NuttX MMIO drivers ≥ L2; L1 listed for non-drivers |
| 8 | Phase-2 drivertest deferred | **CLOSED** | `044-firmware-l3` contest-local path (Approach B) |

**Deferred-with-reason (no follow-on wave planned):**

| Item | Reason |
|------|--------|
| Full cmocka `drivertest_*` from apps/testing | Contest isolation + binary size limit |
| DMAMUX / peripheral-triggered DMA | No NuttX DMAMUX driver in tree |
| HPDMA CH1–15 IRQ | CH0 proves pattern; expand if driver uses more |
| TIM/ADC/LPTIM/DTS/DMA2D/CSI/VENC/NPU/CRYP/OTP L3 | No NuttX driver consuming them yet — leave L1 |

**Full gate (2026-07-14 snapshot):** 42 suites, all PASS. Baseline
was 217 pass; count exceeded after adding IRQ wiring,
multi-peripheral, driver-sequence, and firmware-L3 suites. See
"CMSIS Alignment Campaign" below for the current snapshot.

## CMSIS Alignment Campaign — EXIT 2026-07-16

Cross-checked every driver register offset/bitfield and every
Renode model against the authoritative CMSIS `stm32n647xx.h`
(and, where upstream NuttX has a matching STM32N657 port, against
`apache/nuttx`) instead of the earlier STM32H7-derived assumptions
recorded in `CHANGELOG.md` `[0.6.0]`. That STM32H7 reference
decision is superseded — see the CHANGELOG entry added alongside
this campaign.

| Peripheral | Bug found | Fix |
|---|---|---|
| RCC | CFGR1 @ wrong offset; RTCEN in wrong register; PLL tree not matching real IC divider network | Full rewrite of `hardware/stm32_rcc.h` + `stm32n6_rcc.c` + `STM32N6_RCC.cs`, ported from upstream `stm32_stdclockconfig()` |
| SPI/I2C/SAI/IWDG/OTG/RTC | Systematic base address errors | Corrected against CMSIS; SPI/I2C switch statements completed for all instances |
| FDCAN | `CCCR` at 0x000 instead of 0x018 (and cascading IR/IE/ILS/ILE/TXBAR offsets) | Realigned to CMSIS `FDCAN_GlobalTypeDef` |
| SDMMC | `WAITRESP`/`CPSMEN` bitfields at wrong bit positions | Corrected to CMSIS `SDMMC_TypeDef.CMD` layout |
| XSPI | Entire write-path register block (TCR/IR/ABR/LPTR/WP*) misaligned | Corrected to CMSIS `XSPI_TypeDef` |
| EMAC | MAC address and DMA descriptor registers were fictitious offsets, never matching CMSIS `ETH_TypeDef` | Corrected to real `MACA0HR`/`MACA0LR`/`DMACTXDLAR`/`DMACRXDLAR` offsets |
| OTG | `GRSTCTL.RXFFLSH` never set during FIFO flush (only `TXFFLSH`) | Added missing bit |
| ADC (Renode model only, no NuttX driver) | Missing `IER`; `JSQR` at fictitious 0x70 (CMSIS reserved space); watchdog thresholds at fictitious 0x20/0x24 | Rewrote to CMSIS `ADC_TypeDef`; fidelity raised L1 → L2-state |
| HPDMA (Renode model only) | Compact non-CMSIS per-channel layout instead of the real `DMA_Channel_TypeDef` shared with GPDMA | Realigned every channel register to its CMSIS offset |

**Placeholder models documented (not fixed, since CMSIS does not
publish these registers at all):** CSI, OTP, DTS, VENC, CRYP, NPU.
Each model's header comment and the corresponding ADR
(ADR-035/036/037) now explicitly states the layout is team-defined
and not an authoritative reference.

**Full gate (2026-07-16 snapshot):** 47 suites (`tests/renode/tests/*.robot`),
283 test cases, all PASS, 0 fail.

## Quick run

```bash
# From $WORKSPACE (ctrl_future)
bash contest2026_137_CtrlFuture/scripts/renode-test.sh
```

Manual equivalent:

```bash
REPO=$WORKSPACE/contest2026_137_CtrlFuture
RENODE_SRC=/home/takumi/mi/open-velao-contest/renode
cp $REPO/tests/renode/peripherals/*.cs \
  $RENODE_SRC/src/Infrastructure/src/Emulator/Peripherals/Peripherals/Miscellaneous/
cd $RENODE_SRC && ./build.sh --no-gui
$RENODE_SRC/renode-test $REPO/tests/renode/tests/*.robot \
  --results $REPO/tests/renode/results
```
