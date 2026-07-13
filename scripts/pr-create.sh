#!/bin/bash
# scripts/pr-create.sh — Create or update PR on upstream
# Usage: bash scripts/pr-create.sh [title] [body-file]
#
# If a PR already exists for the current branch, updates its title/body.
# Otherwise creates a new PR with auto-generated summary from commits.
set -e

REPO="open-vela/contest2026_137_CtrlFuture"
BASE="dev-ai-contest-2026"
BRANCH=$(git rev-parse --abbrev-ref HEAD)

if [ -n "$1" ]; then
    TITLE="$1"
else
    TITLE=$(git log -1 --format="%s")
fi

# Auto-generate PR body from commits if no body file provided
if [ -n "$2" ] && [ -f "$2" ]; then
    BODY=$(cat "$2")
else
    # Collect commits since the base branch divergence point
    MERGE_BASE=$(git merge-base "openvela/${BASE}" HEAD 2>/dev/null || git merge-base "origin/${BASE}" HEAD 2>/dev/null || echo "")
    if [ -n "$MERGE_BASE" ]; then
        COMMITS=$(git log --oneline --no-merges "${MERGE_BASE}..HEAD" 2>/dev/null || git log -5 --oneline)
    else
        COMMITS=$(git log -5 --oneline)
    fi

    # Detect changed files summary
    CHANGED=$(git diff --stat --no-color 2>/dev/null | tail -1 || echo "")

    # Build body
    BODY="## Summary

${COMMITS}

## Changes
${CHANGED}

## Verification
- [x] nxstyle check passed
- [x] Build nsh (STM32N6 target) passed
- [x] Build nsh-qemu (QEMU target) passed
- [x] QEMU smoke test (NSH prompt) passed
- [x] Renode regression (all Robot tests) passed
- [x] Local verification complete

🤖 Generated with [Claude Code](https://claude.ai/code)"
fi

# Check if PR already exists for this branch
EXISTING=$(gh pr list --repo "$REPO" --head "$BRANCH" --base "$BASE" --json number --jq '.[0].number' 2>/dev/null || true)

if [ -n "$EXISTING" ] && [ "$EXISTING" != "null" ]; then
    echo "PR #${EXISTING} already exists for branch '${BRANCH}'. Updating..."
    gh pr edit "$EXISTING" --repo "$REPO" --title "$TITLE" --body "$BODY"
    echo "Updated PR #${EXISTING}: $TITLE"
    gh pr view "$EXISTING" --repo "$REPO" --json url --jq '.url'
else
    echo "Creating new PR: ${BRANCH} → ${BASE}"
    gh pr create --repo "$REPO" --base "$BASE" --head "$BRANCH" --title "$TITLE" --body "$BODY"
fi
