# CHANGELOG

All notable changes to this project will be documented in this file.

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
