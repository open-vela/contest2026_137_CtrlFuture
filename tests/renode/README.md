# Renode L2/L3 Driver Correctness

Platform: `stm32n647x0.repl`  
Models: `peripherals/*.cs`  
Suites: `tests/*.robot`

## Phase status

### Phase-1 (current wave gate)

Model L1/L2/L3 Robot suites cover Wave 0–3 peripherals used by the
STM32N6 NuttX drivers. Tags:

| Tag | Meaning |
|-----|---------|
| `L1-register` | Base/register readable |
| `L2-state` | Control/status bit behavior |
| `L3-functional` | End-to-end path under firmware |
| `boot-regression` | Boot still reaches `nsh>` |

USART1 L3 is already proven by `012-uart.robot`:

- `USART1 Console Works` — NSH `help` over stock `STM32F7_USART`
- `USART1 Hello Builtin Works` — builtin `hello` output

SPI and I2C L3 models (`013-spi.robot`, `014-i2c.robot`) are ready
for future drivertest binding.

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

## Quick run

```bash
# From $WORKSPACE (ctrl_future)
bash contest2026_137_CtrlFuture/scripts/renode-test.sh
```
