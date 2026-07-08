#!/bin/bash
# scripts/ci-check.sh — CtrlFuture 一键检查脚本
# 每次 commit 前运行，覆盖编译、体积、QEMU、规范、内存
set -e

WORKSPACE="/home/takumi/mi/open-velao-contest/ctrl_future"
BOARD="vendor/openvela/boards/contest2026_137_board"
CONTEST_DIR="contest2026_137_CtrlFuture"

cd "$WORKSPACE"

echo "=== 1. 编译 nxstyle 工具 ==="
make -C nuttx/tools -f Makefile.host nxstyle 2>/dev/null || true

echo ""
echo "=== 2. nxstyle 编码规范检查 ==="
STYLE_FAIL=0
for f in $(find "$CONTEST_DIR/board" -name "*.c" -o -name "*.h"); do
    OUTPUT=$(nuttx/tools/nxstyle "$f" 2>&1 | grep -v "Path relative" || true)
    if [ -n "$OUTPUT" ]; then
        echo "$OUTPUT"
        STYLE_FAIL=1
    fi
done
if [ $STYLE_FAIL -eq 0 ]; then
    echo "PASS: nxstyle"
else
    echo "FAIL: nxstyle — fix above errors"
    exit 1
fi

echo ""
echo "=== 3. STM32N6 真机编译 ==="
./build.sh "$BOARD/configs/nsh" distclean 2>/dev/null || true
./build.sh "$BOARD/configs/nsh" -j$(nproc)
echo "PASS: nsh build"

echo ""
echo "=== 4. QEMU 编译 ==="
./build.sh "$BOARD/configs/nsh-qemu" distclean 2>/dev/null || true
./build.sh "$BOARD/configs/nsh-qemu" -j$(nproc)
echo "PASS: nsh-qemu build"

echo ""
echo "=== 5. 二进制体积守护 ==="
SIZE=$(stat -c%s nuttx/nuttx.bin)
MAX_SIZE=1048576  # 1MB — 根据实际需求调整
echo "Binary size: $SIZE bytes (limit: $MAX_SIZE)"
if [ "$SIZE" -gt "$MAX_SIZE" ]; then
    echo "FAIL: binary exceeds size limit"
    exit 1
fi
echo "PASS: size check"

echo ""
echo "=== 6. 内存使用报告 ==="
arm-none-eabi-size nuttx/nuttx 2>/dev/null || echo "(cross-toolchain not in PATH, skip)"

echo ""
echo "=== 7. QEMU 冒烟测试 ==="
bash "$CONTEST_DIR/scripts/qemu-smoke.sh"

echo ""
echo "=== 8. defconfig 一致性 ==="
./build.sh "$BOARD/configs/nsh" savedefconfig 2>/dev/null
if ! diff -q nuttx/defconfig "$CONTEST_DIR/board/contest_board/configs/nsh/defconfig" >/dev/null 2>&1; then
    echo "WARNING: defconfig has drifted — review changes"
else
    echo "PASS: defconfig consistent"
fi

echo ""
echo "=========================================="
echo "  ALL CHECKS PASSED"
echo "=========================================="
