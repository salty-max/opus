#!/usr/bin/env bash
# Runs the same clang-format the CMake `format` targets find, so the hook and
# the gate never disagree about formatting.
set -euo pipefail

for candidate in clang-format clang-format-23 /opt/homebrew/opt/llvm/bin/clang-format /usr/local/opt/llvm/bin/clang-format; do
  if command -v "$candidate" > /dev/null 2>&1; then
    exec "$candidate" "$@"
  fi
done
echo "clang-format not found; see docs/development.md" >&2
exit 1
