#!/bin/bash
# scripts/check-memory.sh — 内存使用检查
# 报告固件的 text/data/bss 段大小及 SRAM 使用率
set -e

WORKSPACE="/home/takumi/mi/open-velao-contest/ctrl_future"
ELF="$WORKSPACE/nuttx/nuttx"

# STM32N6 SRAM: 4.2MB (DEV boot, 减去 1KB boot header)
TOTAL_SRAM=4193280

cd "$WORKSPACE"

if [ ! -f "$ELF" ]; then
    echo "ERROR: nuttx ELF not found. Build first."
    exit 1
fi

echo "=== Memory Usage Report ==="
echo ""

# arm-none-eabi-size 输出
if command -v arm-none-eabi-size >/dev/null 2>&1; then
    echo "--- Section sizes ---"
    arm-none-eabi-size "$ELF"
    echo ""

    # 计算使用率
    read TEXT DATA BSS <<< $(arm-none-eabi-size "$ELF" | \
        awk 'NR==2 {print $1, $2, $3}')
    USED=$((TEXT + DATA + BSS))
    PERCENT=$((USED * 100 / TOTAL_SRAM))

    echo "--- Summary ---"
    echo "  .text:  $TEXT bytes (code + rodata)"
    echo "  .data:  $DATA bytes (initialized globals)"
    echo "  .bss:   $BSS bytes (zero-initialized)"
    echo "  TOTAL:  $USED / $TOTAL_SRAM bytes ($PERCENT%)"
    echo ""

    # 警告阈值（70% 以上提醒）
    if [ "$PERCENT" -gt 70 ]; then
        echo "WARNING: SRAM usage > 70% — consider optimization"
    elif [ "$PERCENT" -gt 90 ]; then
        echo "CRITICAL: SRAM usage > 90% — risk of stack overflow"
        exit 1
    else
        echo "OK: SRAM usage within budget"
    fi

    echo ""
    echo "--- Top 20 largest symbols ---"
    arm-none-eabi-nm -S --size-sort "$ELF" | tail -20

else
    echo "WARNING: arm-none-eabi-size not in PATH"
    echo "Install ARM toolchain or add to PATH"

    # 备用：用 stat 检查 .bin 文件大小
    if [ -f "$ELF.bin" ] || [ -f "$WORKSPACE/nuttx/nuttx.bin" ]; then
        BIN_SIZE=$(stat -c%s "$WORKSPACE/nuttx/nuttx.bin")
        echo "Binary size: $BIN_SIZE bytes"
    fi
fi
