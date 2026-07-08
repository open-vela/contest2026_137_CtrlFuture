#!/bin/bash
# scripts/qemu-smoke.sh — QEMU 冒烟测试
# 编译 nsh-qemu 固件，启动 QEMU，验证 NSH shell 响应
set -e

WORKSPACE="/home/takumi/mi/open-velao-contest/ctrl_future"
BOARD="vendor/openvela/boards/contest2026_137_board"
QEMU="prebuilts/qemu/linux-x86_64/bin/qemu-system-arm"
TIMEOUT=15
OUTPUT_LOG="/tmp/qemu-smoke-output.log"

cd "$WORKSPACE"

# 确保 QEMU 固件已编译
if [ ! -f nuttx/nuttx.bin ]; then
    echo "Building nsh-qemu firmware..."
    ./build.sh "$BOARD/configs/nsh-qemu" -j$(nproc)
fi

echo "Starting QEMU (timeout: ${TIMEOUT}s)..."

# 启动 QEMU，自动发送 help 命令，超时退出
timeout "$TIMEOUT" bash -c "
    echo 'help' | $QEMU \
        -machine mps3-an547 \
        -nographic \
        -kernel nuttx/nuttx.bin \
        2>/dev/null
" > "$OUTPUT_LOG" 2>&1 || true

# 验证 NSH 是否启动
if grep -q "nsh>" "$OUTPUT_LOG"; then
    echo "PASS: NSH shell booted successfully"
else
    echo "FAIL: NSH shell did not start"
    echo "--- Output ---"
    cat "$OUTPUT_LOG"
    exit 1
fi

# 验证 help 命令响应
if grep -q "help" "$OUTPUT_LOG"; then
    echo "PASS: NSH responds to commands"
else
    echo "WARNING: could not verify command response"
fi

rm -f "$OUTPUT_LOG"
echo "=== QEMU SMOKE TEST PASSED ==="
