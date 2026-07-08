#!/bin/bash
# scripts/ci-check.sh — CtrlFuture 一键检查脚本
# 每次 commit 前运行，覆盖编译、体积、QEMU、规范、内存
set -e

WORKSPACE="/home/takumi/mi/open-velao-contest/ctrl_future"
BOARD="vendor/openvela/boards/contest2026_137_board"
CONTEST_DIR="contest2026_137_CtrlFuture"

cd "$WORKSPACE"

FIND_DIRS="$CONTEST_DIR/board $CONTEST_DIR/app $CONTEST_DIR/arch"

echo "=== 1. 编译 nxstyle 工具 ==="
make -C nuttx/tools -f Makefile.host nxstyle 2>/dev/null || true

echo ""
echo "=== 2. nxstyle 编码规范检查 ==="
STYLE_FAIL=0
for f in $(find $FIND_DIRS -name "*.c" -o -name "*.h" 2>/dev/null); do
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
echo "=== 3. 执行权限检查 ==="
PERM_FAIL=0
for f in $(find $FIND_DIRS -name "*.c" -o -name "*.h" 2>/dev/null); do
    if [ -x "$f" ]; then
        echo "$f: error: execute permission on source file"
        PERM_FAIL=1
    fi
done
if [ $PERM_FAIL -eq 0 ]; then
    echo "PASS: no execute permission on sources"
else
    echo "FAIL: remove execute permission with: chmod -x <file>"
    exit 1
fi

echo ""
echo "=== 4. Commit message 格式检查 ==="
COMMIT_MSG=$(git -C "$CONTEST_DIR" log -1 --format="%B")
FIRST_LINE=$(echo "$COMMIT_MSG" | head -n1)
MSG_FAIL=0

SCOPE_RE="^(board/contest_board/src|board/contest_board|app|docs|scripts|ci|arch): .+"
if ! echo "$FIRST_LINE" | grep -qE "$SCOPE_RE"; then
    echo "FAIL: commit subject must start with allowed scope"
    echo "  Got: $FIRST_LINE"
    MSG_FAIL=1
fi

LEN=${#FIRST_LINE}
if [ "$LEN" -gt 72 ]; then
    echo "FAIL: commit subject is $LEN chars (max 72)"
    MSG_FAIL=1
fi

if ! echo "$COMMIT_MSG" | grep -qE "^Signed-off-by: .+ <.+>"; then
    echo "FAIL: missing Signed-off-by (use git commit -s)"
    MSG_FAIL=1
fi

if echo "$COMMIT_MSG" | grep -qE "^Change-Id:"; then
    echo "FAIL: Gerrit Change-Id is forbidden"
    MSG_FAIL=1
fi

if echo "$COMMIT_MSG" | grep -qiE "^Co-Authored-By:"; then
    echo "FAIL: AI Co-Authored-By markers are forbidden"
    MSG_FAIL=1
fi

if echo "$COMMIT_MSG" | grep -Pq '[\x{4e00}-\x{9fff}]'; then
    echo "FAIL: Chinese characters in commit message"
    MSG_FAIL=1
fi

if [ $MSG_FAIL -eq 0 ]; then
    echo "PASS: commit message format"
else
    echo "FAIL: commit message format — see errors above"
    exit 1
fi

echo ""
echo "=== 5. 中文字符检查 (源文件) ==="
CN_FAIL=0
for f in $(find $FIND_DIRS -name "*.c" -o -name "*.h" 2>/dev/null); do
    if grep -Pn '[\x{4e00}-\x{9fff}]' "$f"; then
        CN_FAIL=1
    fi
done
if [ $CN_FAIL -eq 0 ]; then
    echo "PASS: no Chinese characters in sources"
else
    echo "FAIL: Chinese characters found in source files"
    exit 1
fi

echo ""
echo "=== 6. STM32N6 真机编译 ==="
./build.sh "$BOARD/configs/nsh" distclean 2>/dev/null || true
./build.sh "$BOARD/configs/nsh" -j$(nproc)
echo "PASS: nsh build"

echo ""
echo "=== 7. QEMU 编译 ==="
./build.sh "$BOARD/configs/nsh-qemu" distclean 2>/dev/null || true
./build.sh "$BOARD/configs/nsh-qemu" -j$(nproc)
echo "PASS: nsh-qemu build"

echo ""
echo "=== 8. 二进制体积守护 ==="
SIZE=$(stat -c%s nuttx/nuttx.bin)
MAX_SIZE=1048576  # 1MB
echo "Binary size: $SIZE bytes (limit: $MAX_SIZE)"
if [ "$SIZE" -gt "$MAX_SIZE" ]; then
    echo "FAIL: binary exceeds size limit"
    exit 1
fi
echo "PASS: size check"

echo ""
echo "=== 9. 内存使用报告 ==="
arm-none-eabi-size nuttx/nuttx 2>/dev/null || echo "(cross-toolchain not in PATH, skip)"

echo ""
echo "=== 10. QEMU 冒烟测试 ==="
bash "$CONTEST_DIR/scripts/qemu-smoke.sh"

echo ""
echo "=== 11. defconfig 一致性 ==="
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
