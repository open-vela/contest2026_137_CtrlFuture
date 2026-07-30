# dma2d_test_app（DMA2D 去风险探针样例）

映射到 openvela `packages/demos/contest2026_137_dma2d_test_app`。

ADR-032 探针：判定 DEV boot 下 DMA2D 能否真正写入 SRAM。`dma2d_test`
调用 arch 助手 `stm32n6_dma2d_probe()`——它读取覆盖固件 SRAM 的 RISAF7
region，取其写白名单里已授信的 compartment ID（CID），通过 RIFSC 把该 CID
赋给 DMA2D（master index 8），再跑一次 register-to-memory 填充把已知字写入
一块 cache-line 对齐的 SRAM 缓冲，最后读回校验。

用 sentinel 预填 + D-cache clean/invalidate 严格区分「DMA2D 真写入」与
「被 RISAF 防火墙静默丢弃（RAZ/WI）」——这正是当初发现 GPDMA1 SRAM 阻塞的
手法。PASS 即证明 RISAF 授权对 DMA2D 生效（与 GPDMA1 不同，后者 CID 由 ROM
固定、固件无法授权）。全程片内，无需外部接线或仪器。

依赖 `CONFIG_STM32_DMA2D`。
