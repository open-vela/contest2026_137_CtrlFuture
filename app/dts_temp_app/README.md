# dts_temp_app（DTS 芯片温度自测样例）

映射到 openvela `packages/demos/contest2026_137_dts_temp_app`。

ADR-030：验证片内数字温度传感器（DTS）。`dts_temp` 打开 `/dev/temp0`，
读回一个 b16_t（Q16.16）摄氏温度样本并打印。读数落在运行芯片的合理
区间（0~110°C）即打印 `DTS TEMP PASS (temp=NN.NNC)`，否则打印 `FAIL`。
传感器为片内结温传感器，回读全程不出芯片，无需外部接线或仪器。

依赖 arch 层 `CONFIG_STM32_DTS`。
