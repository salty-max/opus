#!/usr/bin/env bash
# commit-msg hook.
#
# Runs convco for type / scope-enum / length validation, and adds two checks
# convco cannot express: the scope is mandatory (convco's scopeRegex only
# fires when a scope is present), and the message carries no AI attribution.
#
# Usage: bash scripts/commit-msg.sh <commit-msg-file>
set -euo pipefail

msg_file="${1:-}"
if [[ -z "$msg_file" || ! -f "$msg_file" ]]; then
  echo "commit-msg.sh: expected the commit message file as \$1" >&2
  exit 2
fi

first_line="$(head -n 1 "$msg_file")"

case "$first_line" in
  Merge*|Revert*|"fixup! "*|"squash! "*|"amend! "*) exit 0 ;;
esac

if ! [[ "$first_line" =~ ^[a-z]+\([a-zA-Z0-9/_-]+\)\!?:[[:space:]] ]]; then
  cat >&2 <<MSG
❌ Commit subject must include a scope.

   Format:    <type>(<scope>)[!]: <subject>
   Got:       $first_line

   Scopes are listed in .versionrc and docs/development.md.
MSG
  exit 1
fi

# The CLAUDE.md filename is a legitimate thing for a commit to mention.
if grep -v '^#' "$msg_file" | sed 's/CLAUDE\.md//g' | grep -qiE 'claude|anthropic|co-authored-by|generated with|🤖|ai-generated'; then
  echo "❌ Commit message carries AI attribution; see CLAUDE.md \"The contract\"." >&2
  exit 1
fi

convco check --from-stdin < "$msg_file"
