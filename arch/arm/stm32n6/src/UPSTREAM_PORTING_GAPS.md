# STM32N6 Upstream Porting Gaps

This document tracks fixes and features from apache/nuttx upstream's
STM32N657 port that were evaluated during the CMSIS/upstream
alignment work but deliberately **not** ported into this repo's
drivers, along with the reason each was deferred. Nothing here is an
oversight -- each item was read, evaluated against this repo's actual
boot configuration, and either found inapplicable or too risky to
land without real-hardware verification.

Re-evaluate every item below once ADR-004 (real hardware validation)
moves from `PENDING` to `MEASURED`.

## 1. Chip-level header organization (stm32.h / chip.h aggregator)

**Upstream:** `stm32.h` is a single aggregator header that
`#include`s `stm32_gpio.h`, `stm32_lowputc.h`, `stm32_pwr.h`,
`stm32_rcc.h`, `stm32_uart.h` so driver `.c` files only need one
`#include "stm32.h"`. `chip.h` (src-level) forwards to
`hardware/stm32n6xxx_memorymap.h` and `<arch/stm32n6/chip.h>`.

**This repo:** each driver `.c` file directly `#include`s the
specific headers it needs; there is no aggregator.

**Why not ported:** purely a code-organization style difference with
no functional impact. Adopting the aggregator pattern would require
touching the `#include` block of every one of the 20+ driver files in
this tree for zero behavior change. Revisit only if a future large
refactor of this driver tree is already in scope for other reasons.

## 2. GPIO: dynamic per-pin bitfield macros

**Upstream:** `stm32_gpio.c`/`stm32_gpio.h` compute per-pin
MODER/OTYPER/OSPEEDR/PUPDR/AFR field positions with macros like
`GPIO_MODER_MASK(pin)` / `GPIO_MODER_SHIFT(pin)`.

**This repo:** keeps its existing compact 32-bit `cfgset` encoding
(`stm32n6_gpio.h`) with fixed shift constants computed inline in
`stm32n6_configgpio()`.

**Why not ported:** the *behavioral* fixes upstream's `stm32_gpio.c`
provides (spinlock, glitch-free output ordering, `-EINVAL`,
`unconfiggpio()`) were ported in isolation on top of this repo's
existing encoding (see `stm32n6_gpio.c`/`.h`, commit `7ce1302`).
Switching to upstream's macro-based encoding itself is a pure
refactor with no behavioral upside and would require re-encoding
every `GPIO_PORTx`/`GPIO_PIN`/`GPIO_AF` constant used across the
tree (including board.h pin assignments). Not worth the churn.

## 3. RCC: `STM32_APBxENSR`-style atomic peripheral clock enables for every peripheral

**Upstream:** `rcc_enableahb4()` in `stm32n6xx_rcc.c` enables *all*
AHB4 peripherals (every GPIO port + PWR) in a single
`STM32_RCC_AHB4ENSR` write, and `rcc_enableapb2()` conditionally
enables USART1 the same way.

**This repo:** `stm32n6_clockconfig()` only enables `GPIOEEN`
(USART1's PE5/PE6 pins) and `USART1EN` -- the minimum needed for the
console. Other peripheral drivers (SPI/I2C/FDCAN/SDMMC/XSPI/EMAC/
etc.) each independently enable their own clocks inside their own
`_initialize()` functions.

**Why not ported:** this repo's per-driver clock-enable pattern is
intentional and predates this porting effort; unconditionally
enabling every AHB4 GPIO port's clock at boot (whether or not any
driver uses that port) is a design choice upstream makes for a
simpler single-purpose board, not one this contest board's
broader driver set needs to copy. No functional bug here, just a
different (also valid) design.

## 4. RCC: `STM32_CPUCLK_FREQUENCY`-based SysTick reload

**Upstream:** `stm32_timerisr.c`'s `SYSTICK_RELOAD` macro uses
`STM32_CPUCLK_FREQUENCY` (a board.h macro) unconditionally.

**This repo:** `stm32n6_timerisr.c` keeps `STM32_HSI_FREQUENCY`
(64 MHz) hardcoded.

**Why not ported:** verified with an explicit calculation (see
commit `0e97e28`) that board.h's `STM32_CPUCLK_FREQUENCY` describes
the frequency the CPU *would* run at once `CONFIG_STM32N6_USE_PLL1`
is enabled (200 MHz or 800 MHz depending on the
`CONFIG_EDGESIGHT_CLOCK_800MHZ` branch) -- it does not track whether
PLL1 is actually enabled in `.config`. The shipped default has
`CONFIG_STM32N6_USE_PLL1=n`, so the CPU is still running from HSI at
64 MHz. Using `STM32_CPUCLK_FREQUENCY` in that configuration would
make every OS tick ~3.1x too slow (computed: 200 MHz reload count
divided by the real 64 MHz clock rate gives a 31.2 ms tick instead of
the intended 10 ms).

**What needs to happen before this can be revisited:** board.h's
`STM32_CPUCLK_FREQUENCY`/`STM32_SYSCLK_FREQUENCY`/etc. macros need to
be made conditional on whether `CONFIG_STM32N6_USE_PLL1` is actually
set, so they always describe the *actual* running frequency rather
than the frequency PLL1 *would* produce if enabled. Once that's true,
switch `stm32n6_timerisr.c` to use `STM32_CPUCLK_FREQUENCY`
unconditionally, matching upstream, and re-verify the tick rate on
real hardware with a logic analyzer or scope on a GPIO toggled once
per `nxsched_process_timer()` call.

## 5. `__start()`: naked dispatcher clearing MSPLIM/PSPLIM

**Upstream:** `__start()` is `naked` + `noinstrument_function` and
its first act is `msr msplim, r0` / `msr psplim, r0` (both zeroed)
before tail-calling `__start_c()`, because "the STM32N6 boot ROM (DEV
mode) leaves MSPLIM and PSPLIM set such that the first stack push
from C code can fault."

**This repo:** `__start()` is an ordinary (non-naked) C function; no
MSPLIM/PSPLIM handling.

**Why not ported:** this repo has never observed the boot-time stack
fault upstream's comment describes, and there is no way to confirm
whether it is present on this board's actual boot ROM revision
without a real device and a debugger session to inspect MSPLIM/PSPLIM
at reset. Porting a `naked` function with inline assembly on
unverified assumptions about register state risks introducing a
*new*, harder-to-diagnose boot failure if the assumptions don't hold
for this specific board/boot-ROM combination.

**What needs to happen before this can be revisited:** with a
debugger attached at reset (before any NuttX code runs), read
MSPLIM and PSPLIM. If they are non-zero and would fault on the first
stack push, port this fix. If they are already zero, this fix is not
needed for this board's boot ROM revision and should stay
undocumented-as-unnecessary rather than blindly copied.

## 6. `__start_c()`: SysTick disable + PENDSTCLR (FSBL chain-load compatibility)

**Upstream:** disables SysTick and clears any pending SysTick
interrupt at the very start of `__start_c()`, because "when
chain-loaded by an FSBL that called `HAL_Init()`, SysTick may be left
running."

**This repo:** no such handling.

**Why not ported:** this repo boots via **DEV mode with a debugger
writing directly to SRAM** -- there is no FSBL (First Stage Boot
Loader) in the boot path today. Confirmed via `docs/adr/ADR-004.md`
(still `PENDING`, not yet run on real hardware) and
`docs/adr/ADR-019.md` (which documents FSBL support as a *future*,
not-yet-integrated flash-boot path, separate from the current DEV
boot mode). The problem this code solves (stale FSBL-configured
SysTick state) has no trigger condition in this repo's current boot
path.

**What needs to happen before this can be revisited:** port this
fix if/when ADR-019's FSBL flash-boot path is actually integrated.
Until then it protects against a scenario that cannot occur.

## 7. CCR.LOB (Low-Overhead Branch) enable

**Upstream:** `stm32_enable_lob()` sets `NVIC_CFGCON.LOB` "before any
loop the compiler may lower with LE (and before MVE code, which is
gated on the same bit)."

**This repo:** no LOB handling; `NVIC_CFGCON_LOB` is not even defined
in this repo's headers.

**Why not ported:** LOB affects whether ARMv8.1-M WLS/DLS/LE
instructions (and MVE/Helium code) execute correctly. This repo does
not currently build any code path that intentionally emits MVE
instructions, so the risk of silently mis-executing a compiler-
generated LE loop is speculative rather than observed. Enabling a
CPU feature bit with no corresponding code that exercises it adds
complexity without a demonstrated need.

**What needs to happen before this can be revisited:** if/when this
repo starts building MVE-using code (e.g. NPU-adjacent DSP paths,
or if the compiler's `-mcpu=cortex-m55` output is confirmed to emit
LE-form loops that need this bit), port `stm32_enable_lob()` and
verify with a disassembly that the relevant loops actually execute
correctly with and without the bit set.

## 8. ES0620 / BSEC / SYSCFG erratum mitigations

**Upstream:** `__start_c()` sets `RCC_APB4HENR_BSECEN` "per ES0620,
BSECEN must remain set or WFI/sleep fails," configures `LPEN` bits
so clocks keep running through WFI, calls
`stm32_pwr_enablevddio(BOARD_PWR_VDDIO)`, and writes
`SYSCFG_CCCR_ES0620_MANUAL` to `STM32_SYSCFG_VDDIO2CCCR`/
`VDDIO3CCCR`/`VDDCCCR` as an "ES0620 I/O-compensation mitigation."

**This repo:** none of this is implemented. `stm32_pwr_enablevddio()`
itself *was* ported as an API in commit `925a6fd` (PWR stage), but
it is not yet called from `__start_c()`/`stm32n6_start.c`, and none
of the SYSCFG/BSEC/LPEN erratum-specific register writes were
ported.

**Why not ported:** ES0620 is a specific silicon errata entry from
ST's errata sheet for this chip family. Whether this repo's actual
silicon revision is affected, and whether the specific mitigation
sequence upstream uses is complete and correct for this board's
power/IO configuration, cannot be determined without: (a) the errata
sheet for the exact silicon revision on hand, and (b) real hardware
to confirm the symptom (WFI/sleep failing, or I/O compensation
issues) is actually present, and that the mitigation resolves it
without side effects. Copying an erratum workaround for a chip
revision that may not need it is not free -- it changes BSEC/SYSCFG/
LPEN register state on every boot for no benefit if the erratum does
not apply here, and a wrong sequence could mask a real problem or
introduce a new one.

**What needs to happen before this can be revisited:**
1. Confirm the exact STM32N647X0 silicon revision/date code on the
   real board once available, and check it against ST's ES0620
   errata sheet to see if it is affected.
2. If affected, port `stm32_pwr_enablevddio(BOARD_PWR_VDDIO)`'s call
   site into `stm32n6_start.c`, along with the `RCC_APB4HENR_BSECEN`/
   `SYSCFGEN` set, the `BUSLPENR`/`MEMLPENR`/`APB2LPENR` LPEN bits,
   and the `SYSCFG_CCCR_ES0620_MANUAL` writes -- verify WFI/sleep
   actually works (or actually fails without the fix) on real
   hardware before and after.
3. If not affected, leave this undone; do not port erratum code for
   a problem that does not exist on this board's silicon.

## 9. `.data` copy guard (partially ported, one difference remains)

**Upstream and this repo now both** skip the `.data` load-to-run
copy when `&_eronly[0] == &_sdata[0]` (ported in commit `0e97e28`).
This item is **not a gap** -- listed here only to note that the
guard is currently a no-op on this repo's SRAM-only DEV-boot linker
script (which always places `.data`'s load address ahead of its run
address), so it has not actually been exercised as a true skip-copy
path. Re-verify it actually skips the copy (not just compiles) if
this board's linker script is ever changed to a flash-boot layout
where the addresses could become equal.

## 10. RTC backup-domain write protection (`stm32n6_pwr_enablebkp`)

**Upstream `stm32_pwr_enablebkp()`** was ported as a standalone API
in commit `925a6fd` (PWR stage), but **`stm32n6_rtc.c` does not call
it** (or any equivalent) before writing RTC registers.

**Why not wired in:** the backup domain defaults to write-protected
on reset (`PWR_DBPCR.DBP` = 0), so if this repo's RTC driver has ever
successfully written its registers on real hardware, either (a) the
domain was already unlocked by something else in the boot path, or
(b) the writes have been silently dropped by hardware and RTC
functionality has never actually worked as intended. This can only
be distinguished with a real board: read back `PWR_DBPCR` after boot
and before any RTC configuration, or probe whether RTC time-keeping
actually persists/advances correctly.

**What needs to happen before this can be revisited:** on real
hardware, verify whether `stm32n6_rtc.c`'s configuration writes are
actually taking effect. If not, call `stm32n6_pwr_enablebkp(true)`
before RTC init in `stm32n6_rtc.c` (or once in `stm32n6_start.c`
before any RTC/backup-SRAM access) and re-verify.

## Summary table

| # | Item | File(s) | Blocker |
|---|------|---------|---------|
| 1 | stm32.h/chip.h aggregator headers | (organizational, all drivers) | None -- deliberately skipped, no bug |
| 2 | GPIO dynamic bitfield macros | stm32n6_gpio.h/.c | None -- deliberately skipped, no bug |
| 3 | Unconditional AHB4/APB2 clock enable-all | stm32n6_rcc.c | None -- different valid design |
| 4 | SysTick reload from STM32_CPUCLK_FREQUENCY | stm32n6_timerisr.c, board.h | board.h CPUCLK_FREQUENCY must track actual clock source |
| 5 | naked __start + MSPLIM/PSPLIM clear | stm32n6_start.c | Need debugger read of MSPLIM/PSPLIM at reset on real board |
| 6 | SysTick disable + PENDSTCLR (FSBL) | stm32n6_start.c | Need FSBL flash-boot path (ADR-019) actually integrated |
| 7 | CCR.LOB enable | stm32n6_start.c | Need MVE-using code path to exist first |
| 8 | ES0620/BSEC/SYSCFG erratum mitigation | stm32n6_start.c | Need silicon revision check + real-hardware WFI/sleep test |
| 10 | RTC backup-domain write unlock | stm32n6_rtc.c | Need real-hardware verification that RTC writes currently work or don't |
