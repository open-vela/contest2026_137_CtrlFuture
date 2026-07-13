# STM32N647 Firmware Build, Flash & Debug Guide

## Hardware Overview

STM32N647X0 (Cortex-M55, 800MHz, 4.2MB SRAM, no internal flash).
Board: Alientek STM32N647 with 32MB NOR Flash (XSPI1) + 32MB HyperRAM (XSPI2).

### Memory Map (from CMSIS stm32n647xx.h)

| Region | Base (Secure) | Size | Notes |
|--------|--------------|------|-------|
| ITCM | 0x00000000 | 64-256KB | FLEXRAM, configurable split with DTCM |
| DTCM | 0x20000000 | 128-256KB | FLEXRAM |
| BOOTROM | 0x08000000 | 128KB | Factory boot ROM |
| SRAM1_AXI | 0x34000000 | 1MB | Main SRAM bank 1 |
| SRAM2_AXI | 0x34100000 | 1MB | Main SRAM bank 2 |
| SRAM3-6_AXI | 0x34200000 | ~1.8MB | Additional SRAM banks |
| CACHEAXI_RAM | 0x343C0000 | 256KB | NPU cache |
| VENC_RAM | 0x34400000 | 128KB | Video encoder |
| SRAM_AHB | 0x38000000 | 32KB | AHB-accessible SRAM |
| XSPI1 (ext) | 0x90000000 | 32MB | Macronix NOR flash |
| XSPI2 (ext) | 0x70000000 | 32MB | HyperRAM |

FLEXRAM total: 400KB shared between ITCM and DTCM.

### Boot Modes

| BOOT1 | BOOT0 | Mode | Description |
|-------|-------|------|-------------|
| 0 | 0 | Flash boot | Boot from XSPI flash (requires FSBL) |
| 0 | 1 | DEV boot | Code loaded to SRAM via SWD debugger |
| 1 | 0 | Serial boot | UART/USB DFU bootloader |
| 1 | 1 | Reserved | -- |

## Build

### Workspace Layout

```
~/openvela/                     # WORKSPACE root
  build.sh -> nuttx/tools/build.sh
  nuttx/                        # NuttX kernel
  apps/                         # NuttX applications
  vendor/openvela/boards/contest2026_137_board/  # symlink
  contest2026_137_CtrlFuture/   # THIS REPO
  prebuilts/qemu/               # QEMU binaries
```

### Build Commands

```bash
cd ~/openvela

# STM32N6 real hardware (DEV boot, SRAM @ 0x34000400)
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# QEMU (MPS3-AN547, flash @ 0x0, SRAM @ 0x01000000)
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu -j8

# Clean rebuild
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh distclean
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8
```

### Output Files

| File | Description |
|------|-------------|
| `nuttx/nuttx` | ELF (with debug symbols) |
| `nuttx/nuttx.bin` | Raw binary for flashing |
| `nuttx/nuttx.hex` | Intel HEX (if CONFIG_INTELHEX_BINARY) |

## Flash (DEV Boot Mode)

### Prerequisites

1. **ST-Link V3** connected via SWD (SWDIO, SWCLK, GND, 3.3V)
2. **STM32CubeProgrammer CLI** installed (`STM32_Programmer_CLI` in PATH)
3. Board boot pins: **BOOT0=1, BOOT1=0** (DEV boot mode)
4. Serial terminal on USART1 (PE5-TX, PE6-RX) at 115200 8N1

### Flash Command

```bash
# Build first
cd ~/openvela
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# Flash to SRAM at 0x34000400
STM32_Programmer_CLI -c port=SWD freq=8000 \
  -w nuttx/nuttx.bin 0x34000400 -v -rst
```

Alternative (HOTPLUG mode, auto-detect):
```bash
STM32_Programmer_CLI -c port=SWD mode=HOTPLUG \
  -w nuttx/nuttx.bin 0x34000400 -s 0x34000400
```

### Serial Console

```bash
# Linux (ST-Link VCP)
minicom -D /dev/ttyACM0 -b 115200

# Or screen
screen /dev/ttyACM0 115200

# Or picocom
picocom -b 115200 /dev/ttyACM0
```

After reset, the NSH prompt should appear:
```
NuttShell (NSH) NuttX-12.x.x
nsh>
```

## Flash (XSPI Boot Mode -- Persistent)

For firmware that survives power cycles, boot from external XSPI flash.

### Prerequisites

1. **FSBL** (First Stage Boot Loader) -- pre-built at:
   `SoftwarePackage/FSBL/MX25UM25645G_W958D8NBYA5I_Example/Binary/fsbl.hex`
2. **External loader** for STM32CubeProgrammer -- pre-built at:
   `SoftwarePackage/External_Loader/MX25UM25645G_ATK-CNN647B/Binary/MX25UM25645G_ATK-CNN647B_ExtMemLoader.stldr`
3. Copy `.stldr` to `$STM32_PRG_PATH/ExternalLoader/`
4. Board boot pins: **BOOT0=0, BOOT1=0** (Flash boot mode)

### Flash Sequence

```bash
# 1. Flash FSBL to XSPI flash
STM32_Programmer_CLI -c port=SWD freq=8000 \
  -w SoftwarePackage/FSBL/.../fsbl.hex -v

# 2. Flash application (NuttX built with XSPI linker script)
#    Requires XSPI-aware linker script (not yet implemented for NuttX)
STM32_Programmer_CLI -c port=SWD freq=8000 \
  -el MX25UM25645G_ATK-CNN647B_ExtMemLoader \
  -w nuttx/nuttx.bin 0x90100400 -v
```

Boot chain: ROM Bootloader -> FSBL -> XSPI init -> NuttX

## QEMU Verification

```bash
cd ~/openvela

# Build QEMU target
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu -j8

# Run in QEMU (Ctrl+A X to exit)
prebuilts/qemu/linux-x86_64/bin/qemu-system-arm \
  -machine mps3-an547 -nographic -kernel nuttx/nuttx.bin
```

**Important**: `nsh` firmware is for real hardware only. Must use `nsh-qemu` for QEMU.

## Renode Verification

```bash
cd ~/openvela

# Build STM32N6 target (NOT nsh-qemu)
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# Run Renode tests (uses nuttx/nuttx ELF by default)
cd renode
python3 tests/run_tests.py \
  --robot-framework-remote-server-full-directory=output/bin/Release \
  -r ../contest2026_137_CtrlFuture/tests/renode/results \
  ../contest2026_137_CtrlFuture/tests/renode/tests/*.robot
```

**Important**: Renode tests model the actual STM32N647 chip, so they require
the `nsh` build (SRAM at 0x34000400), not `nsh-qemu` (MPS3-AN547).

## Full Local Verification

```bash
cd ~/openvela

# 1. Style check
bash contest2026_137_CtrlFuture/scripts/check-style.sh

# 2. Build both targets
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh distclean
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu distclean
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh-qemu -j8

# 3. QEMU smoke test (needs nsh-qemu)
timeout 15 prebuilts/qemu/linux-x86_64/bin/qemu-system-arm \
  -machine mps3-an547 -nographic -kernel nuttx/nuttx.bin

# 4. Rebuild nsh for Renode (nsh-qemu overwrites nuttx/nuttx)
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh distclean
./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8

# 5. Renode regression
cd renode
python3 tests/run_tests.py \
  --robot-framework-remote-server-full-directory=output/bin/Release \
  -r ../contest2026_137_CtrlFuture/tests/renode/results \
  ../contest2026_137_CtrlFuture/tests/renode/tests/*.robot
```

## Reference Documents

| Document | Location |
|----------|----------|
| STM32N647 Datasheet (DS14791) | `docs/stm32n647x0.pdf` |
| ST HAL/LL Drivers | `SoftwarePackage/Drivers/` |
| ST Example Projects | `SoftwarePackage/Projects/` |
| FSBL Source | `SoftwarePackage/FSBL/` |
| External Loader Source | `SoftwarePackage/External_Loader/` |
| Alientek Wiki | https://wiki.alientek.com/docs/Boards/STM32/DNN647/ |
| Hardware Checklist | `docs/HARDWARE_CHECKLIST.md` |
| ADR-004 (DEV Boot) | `docs/adr/ADR-004.md` |
| ADR-019 (XSPI Boot) | `docs/adr/ADR-019.md` |
