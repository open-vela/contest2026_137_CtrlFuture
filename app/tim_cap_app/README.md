# tim_cap_app（TIM PWM + 输入捕获环回样例）

映射到 openvela `packages/demos/contest2026_137_tim_cap_app`。

ADR-027 子项：一次性验证 TIM PWM 输出与输入捕获两项功能。
`tim_cap` 在 TIM3 CH1（`/dev/pwm1`）输出 1 kHz / 50% 波形，经 TISEL
片内环回进入 TIM15 TI1（`/dev/cap0`），由 TIM15 PWM-input 捕获读回
频率与占空比。频率落在 900~1100 Hz 且边沿计数递增即打印
`TIM CAP LOOPBACK PASS`。环回全程在片内，不占用任何外部引脚。

依赖 arch 层 `CONFIG_STM32_TIM15_CAP`（其会 select `STM32_TIM3_PWM`）。
