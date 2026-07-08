# CHANGELOG

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- 本地守护脚本：ci-check.sh / check-style.sh / check-memory.sh / qemu-smoke.sh
- 工程最佳实践文档（docs/BEST-PRACTICES.md）
- Pre-commit hook（nxstyle 自动检查）
- CI 守护：nxstyle 编码规范检查、QEMU 冒烟测试、二进制体积守护

### Changed
- README 进度更新（CI 守护完善项）

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
