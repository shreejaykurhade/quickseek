#!/usr/bin/env bash
# Installs the pre-commit hook from scripts/ into .git/hooks/
# Run once after cloning: bash scripts/install-hooks.sh

set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
HOOK_SRC="$ROOT/scripts/pre-commit"
HOOK_DST="$ROOT/.git/hooks/pre-commit"

cp "$HOOK_SRC" "$HOOK_DST"
chmod +x "$HOOK_DST"

echo "pre-commit hook installed at $HOOK_DST"
