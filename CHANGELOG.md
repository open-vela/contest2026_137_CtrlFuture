# CHANGELOG

All notable changes to this project will be documented in this file.

## [0.8.0] - 2026-07-16

### Added
- Renode test coverage for 20 previously-untested `.repl` peripheral
  instances that had zero Robot suite coverage:
  - `045-uart-instances.robot`: USART3/UART4/5/7/8/9/USART10, L1
    register access, L2 CR1/BRR writes, and L3 TDR-write-reaches-
    backend checks via a Terminal Tester bound to each instance.
  - `046-spi-instances.robot`: SPI3/4/5, L1/L2 register behavior
    plus L3 real TXDR->RXDR byte loopback (same path already
    proven for SPI1/SPI6).
  - `047-gpio-instances.robot`: GPIO ports B/C/D/F/G/H/N/O/P/Q, L1
    register access and L2 output-write behavior (L3 pin-level
    simulation is not reachable without adding GPIO Connections
    wiring to the .repl, which is out of scope for a coverage fix).
- Raised `STM32N6_ADC` Renode model fidelity from L2-state to
  L3-functional: added a test-only `SIMDR` injection register so
  `ADEN -> ADSTART -> EOC -> DR` can be exercised end-to-end
  instead of always reading back 0.
- Raised `STM32N6_DTS` Renode model fidelity from L1 to L2/L3: added
  a test-only `SIMTEMPR` injection register so a
  `CFGR1.START -> TEMPHYSR` measurement cycle can be exercised,
  instead of every register being a static read/write field with
  no interaction.

### Fixed
- `031-adc.robot`: a pre-existing test named "ADC1 CR ADSTART Bit"
  actually wrote CMSIS `ADEN` (bit 0), not `ADSTART` (bit 2);
  renamed and corrected the assertion.
- `STM32N6_GPDMA.cs`/`STM32N6_HPDMA.cs`: documented that DMA
  transfers complete synchronously inside the `CCR.EN` write
  callback, so there is no mid-transfer CPU-observable state --
  matching the "no real X" disclosure already used by the SDMMC/
  EMAC/FDCAN models.
- `019-xspi-boot.robot`: was the only suite with no `[Tags]` at
  all; added tags and replaced a vacuous `int(val) >= 0` check
  with a real write/read round-trip.
- `STM32N6_FDCAN.cs`: removed a stale comment claiming
  `stm32n6_fdcan.c` still had the wrong `CCCR` offset; that driver
  bug was already fixed in the `[0.7.0]` CMSIS alignment work.

### Changed
- Renode/Robot suite grew from 47 to 50 suites (351 test cases, up
  from 283, all passing); see `tests/renode/README.md` "Coverage
  and Fidelity Optimization Wave" for the full breakdown.

## [0.7.0] - 2026-07-16

### Fixed
- Systematic register offset/bitfield audit of all STM32N6 chip
  drivers against the authoritative CMSIS `stm32n647xx.h` header
  (and, where available, the upstream `apache/nuttx` STM32N657
  port), superseding the `[0.6.0]` decision to reference STM32H7 —
  that reference turned out to be an incorrect data source for
  several peripherals (see "Superseded decision" below).
- RCC: `CFGR1` was at the wrong offset, `RTCEN` was in the wrong
  register, and the PLL/IC divider tree did not match the real
  hardware network. Rewrote `hardware/stm32_rcc.h`,
  `stm32n6_rcc.c`, and the `STM32N6_RCC` Renode model from scratch,
  porting the upstream `stm32_stdclockconfig()` logic.
- SPI/I2C/SAI/IWDG/OTG/RTC: systematic base address errors
  corrected against CMSIS; SPI/I2C driver switch statements
  completed for all instances (SPI1-6, I2C1-4).
- FDCAN: `CCCR` was mapped at offset 0x000 instead of the real
  0x018 (with cascading errors in `IR`/`IE`/`ILS`/`ILE`/`TXBAR`).
- SDMMC: `CMD.WAITRESP`/`CMD.CPSMEN` bitfields were at the wrong
  bit positions.
- XSPI: the entire write-path register block
  (`TCR`/`IR`/`ABR`/`LPTR`/`WP*`) was misaligned, including a
  functional bug where `LPTR` writes actually landed on `ABR`.
- EMAC: MAC address and DMA descriptor registers used fictitious
  offsets that never matched CMSIS `ETH_TypeDef`; corrected to the
  real `MACA0HR`/`MACA0LR`/`DMACTXDLAR`/`DMACRXDLAR` registers.
- OTG: FIFO flush never set `GRSTCTL.RXFFLSH`, only `TXFFLSH`, so
  the Rx FIFO was never actually flushed on init.
- Renode models (no matching NuttX driver bug, simulation-only):
  `STM32N6_ADC` was missing `IER` and had `JSQR`/watchdog threshold
  registers at fictitious offsets inside CMSIS-reserved space;
  `STM32N6_HPDMA` used a compact non-CMSIS per-channel layout
  instead of the real `DMA_Channel_TypeDef` shared with GPDMA.

### Changed
- Renode/Robot test suite grew from 42 to 47 suites (283 test
  cases, all passing) to cover the above fixes; see
  `tests/renode/README.md` "CMSIS Alignment Campaign" for the full
  before/after table.
- Documented remaining placeholder Renode register layouts (CSI,
  OTP, DTS, VENC, CRYP, NPU) as team-defined and not CMSIS-derived,
  since CMSIS does not publish register maps for these blocks —
  see model header comments and ADR-035/036/037.

### Superseded decision
- The `[0.6.0]` technical decision "Ethernet/SDMMC/XSPI/LTDC:
  NuttX native drivers (reference STM32H7)" is superseded. STM32H7
  register maps do not reliably match STM32N6 for these
  peripherals (confirmed multiple base address and offset
  mismatches during this audit). CMSIS `stm32n647xx.h` and, for
  peripherals with an existing upstream port, `apache/nuttx`
  STM32N657 sources are now the only authoritative references for
  STM32N6 driver work.

## [0.6.0] - 2026-07-11

### Added
- Chip drivers: DCMIPP camera (stm32n6_dcmipp.c/h) and LTDC display (stm32n6_ltdc.c/h)
  - Dual-pipe camera (display + NN), /dev/video0 V4L2 interface
  - Dual-layer display with double-buffered foreground, /dev/fb0
- EdgeSight new modules:
  - sensor_fusion.c/h: multi-modal decision fusion (vision+audio+PIR+smoke+temp)
  - env_sensor.c/h: environmental sensor driver (SHT30 + MQ-2 + PIR)
  - npu_pipeline.c/h: dual-model inference coordinator (YOLO + MoveNet)
  - edgesight_cmd.c/h: NSH command interface (status/config/log/reset/help)
  - test_sensor_fusion.c: 8-scenario unit test (ALL PASS)
- Board integration: DCMIPP + LTDC init in board_bringup.c with Kconfig guards

### Fixed
- Kconfig guard: compile DCMIPP/LTDC only when CONFIG_VIDEO/CONFIG_VIDEO_FB enabled
- Fixes nsh build failure where video headers were included but framework not enabled

## [0.5.0] - 2026-07-10

### Added
- EdgeSight application skeleton (app/edgesight/) with full pipeline architecture
  - fall_detect.c/h: pose-keypoint-based fall detection algorithm (3-metric fusion)
  - postprocess.c/h: YOLO NMS + MoveNet heatmap parsing routines
  - edgesight_main.c: 7-step pipeline orchestrator (camera→NPU→detect→display→record→network)
  - camera_hal.h: DCMIPP dual-pipeline (display + NN) abstraction
  - npu_hal.h: stedgeai NPU runtime abstraction (dual-model)
  - display_hal.h: LTDC dual-layer + GPU2D rendering abstraction
  - recorder_hal.h: H.264 VENC + SD card event recording abstraction
  - network_hal.h: Ethernet + MQTT alert publishing abstraction
  - memory_map.h: STM32N6 SRAM/Flash buffer allocation plan
  - event_log.c/h: ring buffer event logging with SD card flush
  - config.c/h: runtime configuration (key=value parser from SD card)
  - perf_stats.c/h: performance tracking (FPS, inference time, rolling avg)
  - alert_msg.c/h: MQTT JSON payload formatter (fall/recovery/heartbeat)
  - edgesight.cfg.example: sample configuration file for SD card
  - test_fall_detect.c: 5-scenario unit test (ALL PASS on host)
  - Kconfig, Makefile, CMakeLists.txt, Make.defs, README.md
- 800MHz clock configuration skeleton (board/contest_board/src/stm32n6_clockconfig.c)
- 800MHz clock defines in board.h (CONFIG_EDGESIGHT_CLOCK_800MHZ)
- EdgeSight defconfig (board/contest_board/configs/edgesight/)
- Manifest linkfile for edgesight app
- ST official reference code analysis report (docs/st-reference-analysis.md)
- All 38 ADR documents fully populated with implementation decisions

### Analysis Complete
- STM32CubeN6 HAL drivers: all EdgeSight peripherals covered
- VENC_RTSP_Server example: H.264 + Ethernet + RTSP + Audio + 800MHz
- ObjectDetection example: Camera + ISP + NPU + Display full pipeline
- stedgeai toolchain: generate command + NPU deploy format understood
- AI models confirmed: YOLO-X nano (1.6MB) + MoveNet Lightning (2.7MB)
- Both models have pre-compiled NPU binaries ready for deployment

### Technical Decisions
- Dual-model serial inference: YOLO detect → crop → MoveNet pose → rule-based fall
- Fall detection: torso angle + COG height + bbox ratio, 3-frame confirmation
- NPU/Camera/VENC: integrate ST proprietary libs (not rewrite)
- Ethernet/SDMMC/XSPI/LTDC: NuttX native drivers (reference STM32H7)

## [0.4.0] - 2026-07-09

### Added
- STM32N6 chip driver under arch/arm/stm32n6/ (NuttX upstream layout)
  - RCC clock config (HSI 64MHz), GPIO, USART1 serial, NVIC IRQ
  - SysTick system timer, heap allocator, start code
- ADR template (11-field) and 38 module ADR skeletons (docs/adr/)
- ROADMAP with phase plan and dependency graph (docs/ROADMAP.md)
- Feature list extracted from README (docs/FEATURE-LIST.md)
- 38 GitHub issues mapped to ADR-001 through ADR-038

### Changed
- Restructured chip driver from arch/stm32n6/ to arch/arm/stm32n6/
- CI build.yml: use relative symlinks for Docker compatibility
- Fixed stm32_boardinitialize function name (removed extra underscore)
- Fixed up_putc return type to match NuttX declaration (void)
- Removed spurious CONFIG_ARCH_CHIP_STM32=y from nsh defconfig
- Removed duplicate CONFIG_ARM_M_SYSTICK=y from nsh-qemu defconfig
- ADR-001, ADR-002, ADR-003 completed with full implementation details

### Fixed
- Linker error: getreg32/putreg32 undefined (missing arm_internal.h)
- Linker error: stm32_boardinitialize undefined (wrong function name)
- Linker error: up_putc undefined (missing implementation)
- Linker error: up_timer_initialize undefined (missing SysTick driver)
- Warning: up_prioritize_irq implicit declaration (added forward decl)
- Warning: stm32_boardinitialize implicit declaration (added forward decl)
- CI failure: Docker symlinks used absolute host paths, now relative

## [0.3.0] - 2026-07-09

### Added
- Local CI guard scripts (ci-check.sh, check-style.sh, check-memory.sh, qemu-smoke.sh)
- Engineering best practices guide (docs/BEST-PRACTICES.md)
- Pre-commit and commit-msg hooks (.githooks/)
- CI: nxstyle check, QEMU smoke test, binary size guard, commit message validation
- CI: Chinese character check, defconfig consistency check

### Changed
- CI nsh target re-enabled (stm32n6 chip driver now integrated)
- Commit message rules aligned with Apache NuttX community

## [0.2.0] - 2026-07-08

### Added
- .gitignore (CLAUDE.md, .claude)
- GitHub Actions CI workflow (nsh-qemu target) [BUILD: PASS]

### Changed
- README updated for dual-target architecture
- CI: nsh target temporarily disabled (stm32n6 chip driver pending upstream)

## [0.1.0] - 2026-07-07

### Added
- Board directory structure (board/contest_board) [BUILD: PASS]
- Dual-target defconfig (nsh: STM32N6 / nsh-qemu: MPS3-AN547) [BUILD: PASS]
- Board header board.h (STM32N6 clock tree + pin mapping / MPS SysTick)
- Dual linker scripts (flash.ld: STM32N6 SRAM DEV boot / flash-qemu.ld: MPS3)
- Make.defs / CMakeLists.txt conditional build system
- Board initialization (stm32_boardinitialize + board_late_initialize)
- STM32N6 chip driver integration (arch/arm/src/stm32n6)
- QEMU NSH verification: shell boots, help/hello/ps work [QEMU: PASS]
- README with project overview and build instructions
