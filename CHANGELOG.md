# CHANGELOG

All notable changes to this project will be documented in this file.

## [Unreleased]

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
- CI build.yml updated for arch/arm/stm32n6 symlink paths
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

## [0.2.0] - 2026-07-08

### Added
- .gitignore（CLAUDE.md, .claude）
- GitHub Actions CI 构建工作流（nsh-qemu 目标）[BUILD: PASS]
- README 更新为双目标架构描述

### Changed
- CI 暂时禁用 nsh 目标（stm32n6 芯片驱动待合入上游 NuttX）

## [0.1.0] - 2026-07-07

### Added
- 板级目录结构创建（board/contest_board）[BUILD: PASS]
- 双目标 defconfig（nsh: STM32N6 真机 / nsh-qemu: MPS3-AN547 QEMU）[BUILD: PASS]
- 板级头文件 board.h（STM32N6 时钟树 + 引脚映射 / MPS SysTick 双分支）
- 双链接脚本（flash.ld: STM32N6 SRAM DEV boot / flash-qemu.ld: MPS3 flash+sram）
- Make.defs / CMakeLists.txt 条件构建系统
- 三阶段板级初始化 + stm32_board_initialize() 支持
- STM32N6 芯片驱动集成（arch/arm/src/stm32n6）
- QEMU NSH 验证：命令行启动，help/hello/ps 正常 [QEMU: PASS]
- README 项目说明及构建指南
