# ap3216c_test — AP3216C I2C4 self-test (ADR-014)

Jumper-free real-hardware check that the STM32N6 I2C4 driver transacts data
against a real onboard slave.

- **Bus**: I2C4 (`/dev/i2c4`), PE13=SCL / PE14=SDA, AF4, 400 kHz.
- **Slave**: onboard AP3216C ambient-light / proximity / IR sensor @ `0x1e`
  (ALIENTEK STM32N647 board).
- **Method**: software-reset the sensor, write `0x03` (ALS+PS+IR enable) to
  the system-config register `0x00`, then read it back with a repeated-start
  transaction. PASS iff every transfer ACKs and the register reads back
  `0x03`. A live IR/ALS/PS sample is printed for visibility.

Because the sensor is a real external I2C slave, a pass proves the bus moves
data in both directions — no jumper wire or instrument required.

## Run

```
nsh> ap3216c_test
AP3216C PASS (mode=0x03 ir=... als=... ps=...)
```
