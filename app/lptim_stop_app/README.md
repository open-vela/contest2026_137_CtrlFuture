# lptim_stop_app（LPTIM Stop 模式唤醒样例）

映射到 openvela `packages/demos/contest2026_137_lptim_stop_app`。

ADR-028 子项：验证 LPTIM 在 CPU Stop 模式下仍由 LSI 计数并能唤醒内核。
`lptim_stop [ms]` 以 LPTIM3 单次定时（默认 2000ms）进入 Stop，唤醒后
读回 `PWR_CPUCR.STOPF` 确认确实进入过 Stop，打印
`LPTIM STOP WAKE PASS`。对 `/dev/timer3` 破坏性，勿与 timer3 测试并发。

依赖 arch 层 `CONFIG_STM32_LPTIM_STOPWAKE`。
