#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
#
# Local Renode Robot Framework test runner.
# Rebuilds Renode if C# models changed, then runs all .robot tests.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
WORKSPACE="$(cd "${REPO_DIR}/.." && pwd)"
RENODE_SRC="${WORKSPACE}/renode"
RESULTS_DIR="${REPO_DIR}/tests/renode/results"
MARKER_FILE="${REPO_DIR}/tests/renode/.renode-built"
PERIPHERALS_DIR="${REPO_DIR}/tests/renode/peripherals"
TESTS_DIR="${REPO_DIR}/tests/renode/tests"

echo "=== Renode Robot Framework Test Runner ==="
echo "Repository: ${REPO_DIR}"
echo "Renode source: ${RENODE_SRC}"
echo "Results: ${RESULTS_DIR}"

# Check renode-test exists
if [ ! -x "${RENODE_SRC}/renode-test" ]; then
    echo "ERROR: renode-test not found at ${RENODE_SRC}/renode-test"
    exit 1
fi

# Check if C# models changed and rebuild if needed
NEED_REBUILD=0
if [ ! -f "${MARKER_FILE}" ]; then
    NEED_REBUILD=1
else
    MARKER_TIME=$(stat -c %Y "${MARKER_FILE}" 2>/dev/null || echo 0)
    for cs_file in "${PERIPHERALS_DIR}"/*.cs; do
        [ -f "${cs_file}" ] || continue
        FILE_TIME=$(stat -c %Y "${cs_file}" 2>/dev/null || echo 0)
        if [ "${FILE_TIME}" -gt "${MARKER_TIME}" ]; then
            NEED_REBUILD=1
            break
        fi
    done
fi

if [ "${NEED_REBUILD}" -eq 1 ]; then
    echo ""
    echo "=== C# models changed, rebuilding Renode ==="
    # Copy .cs files to Renode source tree
    cp "${PERIPHERALS_DIR}"/*.cs \
       "${RENODE_SRC}/src/Infrastructure/src/Emulator/Peripherals/Peripherals/Miscellaneous/" 2>/dev/null || true

    # Build Renode
    cd "${RENODE_SRC}"
    ./build.sh --no-gui
    cd "${REPO_DIR}"

    # Update marker
    touch "${MARKER_FILE}"
    echo "Renode build complete."
else
    echo "C# models unchanged, skipping Renode rebuild."
fi

# Build NuttX if needed
NUTTX_ELF="${WORKSPACE}/nuttx/nuttx"
if [ ! -f "${NUTTX_ELF}" ]; then
    echo ""
    echo "=== Building NuttX nsh firmware ==="
    cd "${WORKSPACE}"
    ./build.sh vendor/openvela/boards/contest2026_137_board/configs/nsh -j8
    cd "${REPO_DIR}"
fi

# Run Robot tests
echo ""
echo "=== Running Robot Framework tests ==="
mkdir -p "${RESULTS_DIR}"

"${RENODE_SRC}/renode-test" "${TESTS_DIR}"/*.robot --results "${RESULTS_DIR}"
RESULT=$?

echo ""
echo "=== Results ==="
echo "Reports: ${RESULTS_DIR}/report.html"
echo "Logs: ${RESULTS_DIR}/log.html"

exit ${RESULT}
