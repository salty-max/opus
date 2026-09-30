#!/usr/bin/env bash
# Scaffold a changeset.
#
#   scripts/changeset-new.sh <major|minor|patch> "<CHANGELOG bullet>"
#   scripts/changeset-new.sh                        (interactive)
#
# See .changeset/README.md for what each bump level means.
set -euo pipefail
cd "$(dirname "$0")/.."

bump="${1:-}"
summary="${2:-}"

if [[ -z "$bump" ]]; then
  read -r -p "Bump (major / minor / patch): " bump
fi
case "$bump" in
  major | minor | patch) ;;
  *) echo "Invalid bump '$bump' (expected major, minor or patch)" >&2; exit 1 ;;
esac

if [[ -z "$summary" ]]; then
  read -r -p "CHANGELOG bullet: " summary
fi
if [[ -z "$summary" ]]; then
  echo "A changeset needs a summary." >&2
  exit 1
fi

name=".changeset/$(od -An -N4 -tx1 /dev/urandom | tr -d ' \n').md"
printf -- '---\nbump: %s\n---\n\n%s\n' "$bump" "$summary" > "$name"
echo "Created $name"
