# rng_test_app（RNG 熵健全性自测样例）

映射到 openvela `packages/demos/contest2026_137_rng_test_app`。

ADR-037 子项：验证 STM32N6 硬件 TRNG。`rng_test` 从 `/dev/random`
读两个 32 字节块，断言每块非退化（非全零、非单字节重复）且两块互不相同，
证明真实片上熵源。驱动内部已强制 FIPS 连续检验。全程片内，无需外部接线或仪器。

依赖 `CONFIG_DEV_RANDOM`（由 `STM32_RNG` 驱动支撑）。
