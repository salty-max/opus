#!/usr/bin/env bash
# Runs clang-format from the LLVM version the build pins
# (OPUS_LLVM_VERSION in cmake/OpusLlvmTools.cmake), searched in the same order
# as the CMake `format` targets, so the hook and the gate never disagree.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
pinned="$(sed -nE 's/^set\(OPUS_LLVM_VERSION ([0-9]+)\)$/\1/p' "$root/cmake/OpusLlvmTools.cmake")"

for candidate in "clang-format-$pinned" /opt/homebrew/opt/llvm/bin/clang-format /usr/local/opt/llvm/bin/clang-format clang-format; do
  if command -v "$candidate" > /dev/null 2>&1; then
    version="$("$candidate" --version | sed -nE 's/.*version ([0-9]+)\..*/\1/p')"
    if [[ "$version" == "$pinned" ]]; then
      exec "$candidate" "$@"
    fi
  fi
done
echo "clang-format $pinned not found; see docs/development.md" >&2
exit 1
