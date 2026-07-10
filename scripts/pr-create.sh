#!/bin/bash
# scripts/pr-create.sh — Create or update PR on upstream
# Usage: bash scripts/pr-create.sh [title] [body-file]
#
# If a PR already exists for the current branch, updates its title/body.
# Otherwise creates a new PR with auto-generated body from commit log.
set -e

REPO="open-vela/contest2026_137_CtrlFuture"
BASE="dev-ai-contest-2026"
BRANCH=$(git rev-parse --abbrev-ref HEAD)

if [ -n "$1" ]; then
    TITLE="$1"
else
    TITLE=$(git log -1 --format="%s")
fi

if [ -n "$2" ] && [ -f "$2" ]; then
    BODY=$(cat "$2")
else
    # Auto-generate body from commits since base branch
    COMMITS=$(git log --format="- **%s**%n%b" "origin/${BASE}...${BRANCH}" 2>/dev/null \
        | grep -v "^$" \
        | grep -v "^Signed-off-by:" \
        | grep -v "^Co-Authored-By:" \
        | sed '/^$/d')
    COMMIT_COUNT=$(git rev-list --count "origin/${BASE}...${BRANCH}" 2>/dev/null || echo "?")

    BODY="## Summary

${COMMITS}

## Changes

${COMMIT_COUNT} commit(s) on branch \`${BRANCH}\`.

## Test plan

- [ ] \`ci-check.sh\` passes (nxstyle, build, QEMU smoke)
- [ ] License headers present (ASF required, SPDX recommended)
- [ ] Binary size < 1MB"
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
