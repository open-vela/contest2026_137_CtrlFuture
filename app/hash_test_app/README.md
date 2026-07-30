# hash_test_app（HASH SHA-256 自测样例）

映射到 openvela `packages/demos/contest2026_137_hash_test_app`。

ADR-037 子项：验证 STM32N6 HASH 加速器。`hash_test` 调用 arch 层
`stm32n6_sha256()` 计算 `SHA-256("abc")`，与 NIST FIPS 180-4 已知答案向量
`ba7816bf...f20015ad` 逐字节比对，匹配则打印 `HASH SHA256 PASS`。
HASH 引擎全程片内，比对通过已知向量即证明真实硬件计算，无需外部接线或仪器。

依赖 arch 层 `CONFIG_STM32_HASH`。
