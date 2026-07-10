#!/bin/bash
# scripts/pr-create.sh — Create or update PR on upstream
# Usage: bash scripts/pr-create.sh [title] [body-file]
#
# If a PR already exists for the current branch, updates its title/body.
# Otherwise creates a new PR.
# Body: use body-file if provided, otherwise a minimal placeholder.
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
    BODY="Auto-generated PR from branch \`${BRANCH}\`."
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
