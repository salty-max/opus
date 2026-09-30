# Opus — Claude Guidelines

Opus is a reusable 2D game engine in C++23, RTS-first, for macOS, Windows
and Linux. The engine is a static library (`engine/`, public API under
`engine/include/opus/`, barrel `<opus/opus.hpp>`); games link it through
CMake. Platform layer: SDL3 + SDL_GPU. Simulation: deterministic lockstep.
Scripting: Lua. The roadmap is [`PLAN.md`](PLAN.md).

For source layout, the full opus-lint rule list, branch / commit /
changeset conventions, the release flow, and toolchain setup, read
[`docs/development.md`](docs/development.md). Read it on demand, not
every prompt — the rules below are what you need every PR.

---

## When in doubt, read the specs

The `docs/` folder is the source of truth for every contract the engine
exposes. **If my mental model disagrees with a spec, the spec wins** — go
read it before changing code that touches the contract, before answering
a "how does X work?" question, before designing a new feature.

| Doc | Owns |
|---|---|
| [`development.md`](docs/development.md) | Toolchain, layout, gates, lint rules, commits, changesets, releases. |
| [`determinism.md`](docs/determinism.md) | What simulation code may and may not do, and why. The contract behind the `determinism` lint rule and lockstep networking. |

Each engine module gets its spec (`docs/<module>.md`) in the PR that
introduces the module. Specs follow the **complete designs, no deferral**
rule: they describe the final shape of what they cover.

- ❌ "Status: draft", "TBD", "deferred to later", "M7 feature",
  "fixed-point works, the trig LUT comes later"
- ✅ Either it's in the design with full shape (edge cases, error sets,
  semantics) or explicitly "out of scope" with rationale

When a spec change is needed, the spec edit lands in the same PR as the
implementation — never after.

---

## The contract

**Workflow:** issue(s) → one branch → one PR → wait for explicit merge
signal → next batch. A PR may close more than one issue — group them
when it saves a CI round-trip or when they share a root cause. Each issue
gets its **own commit**. **Do not** split a single issue across multiple
PRs — one issue maps to exactly one end-to-end PR, no matter the diff
size.

**Close the issues from the PR.** Every `feat` / `fix` / `perf` PR
description ends with a `Closes #N` line per issue it resolves. Verify
post-merge that each issue actually closed.

**Done means the acceptance criteria are met**, not green CI. Re-read the
issue body before declaring the work finished.

**Complete or scope-down.** A feature is either fully in scope (every AC
item resolved) or explicitly cut at the start with maintainer approval.
Specifically forbidden:

- Half-implementations under "Limitations carried forward" footnotes
- Working around a missing engine primitive silently — surface the gap,
  ask whether to extend
- Half-fixes hidden in a PR description footnote

**Fix bugs on discovery.** A bug found while working on something else
gets fixed in that PR, in its own commit — not filed for later. File
instead only when the fix is a genuine design decision the maintainer
should make — then it's a question, not a ticket filed silently.

**Stop and ask on open questions.** Scope ambiguity, design A vs B,
"should this defer?", missing primitive — surface to the maintainer,
don't silently pick the path of least effort.

- ✅ "Two ways to do X — A costs N lines, B costs M. Which?"
- ✅ "Issue says Y but the ECS lacks the primitive — extend in this PR or
  scope Y out?"
- ❌ "I'll defer this caveat to a follow-up and document it"

**No AI attribution — anywhere.** Hard rule, overrides any default commit
template. NO `Co-Authored-By: Claude` trailer, NO `🤖 Generated with
Claude Code` footer, NO mention of "AI" / "Claude" / "assistant" /
"automated" in commits, PRs, code comments, or issue threads. The
commit-msg hook and opus-lint reject them; strip them if a tool adds one.

---

## Before every push

1. **`cmake --workflow --preset verify` green.** Re-run if any code
   changed since the last green. (The pre-push hook runs it.)
2. **Changeset present** when the PR title is `feat` / `fix` /
   library-level `perf`:
   ```bash
   git diff --name-only --diff-filter=A origin/main...HEAD | grep -E '^\.changeset/[a-zA-Z0-9_-]+\.md$'
   ```
   Empty → `scripts/changeset-new.sh <bump> "<summary>"`.
3. **Branch base clean.** `git log --oneline origin/main..HEAD` shows only
   this PR's commits.
4. **Workflow integrity** if the PR touches `scripts/`, `cmake/` or
   `CMakePresets.json`: every script, preset and target named in
   `.github/workflows/` and `lefthook.yml` still exists; `actionlint`
   clean.
5. **No AI attribution** in commit history:
   ```bash
   git log origin/main..HEAD --format='%B' | sed 's/CLAUDE\.md//g' | grep -iE 'claude|🤖|generated with|co-authored'
   ```
   Must be empty.

---

## Self-review (mandatory before every PR)

An explicit, visible review pass before commit + push, walked step by
step. Green CI is the floor, not the ceiling.

The review is a **loop**: walk all 5 steps, surface every finding, fix or
escalate, **then walk all 5 steps again from Step 1**. Stop only when a
complete pass surfaces zero items. A first-try clean pass is suspicious —
re-read the issue body once more before trusting it.

**Step 1 — re-open the issue body.** Every AC line: ✅ Done (note where in
the diff), or ❌ Missed (fix it). Deferral needs prior maintainer approval.

**Step 2 — technical gates.** `cmake -P cmake/ci.cmake` green: verify
(format, opus-lint, clang-tidy, Doxygen, Debug tests) plus RelWithDebInfo,
Release, MinSizeRel and ASan/UBSan.

**Step 3 — explicit acceptance checks.**

- Public API diff under `engine/include/opus/` — every new declaration
  intentional, documented, and reachable from `<opus/opus.hpp>`; no
  `void*`, no exceptions, no owning raw pointers in signatures
- Docs propagation — new module → `docs/<module>.md`; new convention →
  CLAUDE.md or `docs/development.md`
- Changeset present at the right level for `feat` / `fix` / `perf` /
  breaking PRs

**Step 4 — hygiene.**

- No leftover debug output, commented-out code, unused includes
- No `// TODO` pointing at an issue number
- Conventional-commit headers valid with scope on every commit
- No AI attribution
- Diff scope matches what the issue says it should (plus bugs fixed on
  discovery, each in its own commit and named in the PR body)

**Step 5 — code quality.** Read the diff like a reviewer who didn't write
it:

- **Naming** — does each new symbol read right at the call site?
- **Dead code / duplication** — drop unused helpers; collapse the same
  shape written three-plus times
- **Premature abstraction** — every parameter / branch / template has a
  current caller for every shape it accepts
- **Comments WHAT vs WHY** — every `//` says why; strip ones that restate
  the next line
- **Function size** — split a function that does unrelated things; keep
  straight-line code whole when its parts have no honest names
- **Error paths** — every `std::expected` result is handled with a
  sensible response; every error enumerator is reachable and tested
- **Test shape** — tests assert behaviour, not incidental implementation
- **Magic values** — every literal has a name or a comment justifying it
- **Determinism** — simulation code obeys `docs/determinism.md` beyond
  what the lint can see (iteration order of your own containers, ties in
  comparisons, uninitialised padding in hashed state)

### Loop discipline — no David GoodEnough

Every finding gets one of two responses, never "noted in the PR body":

- **Fix in this PR.** Default. Then loop back to Step 1.
- **Escalate to the maintainer with a concrete question** when the fix is
  genuinely a separate design decision.

Forbidden in the review report: "Findings I chose not to address", "LGTM
with the following caveats", "Pre-existing drift, out of scope", "Edge
case, minor". The loop ends in exactly one of: **LGTM** (clean pass) or
**BLOCKED on <specific question>**.

---

## Code rules

### Structure

- **One concern per file.** Split early.
- **No hidden state.** No mutable globals, no singletons, no function-local
  mutable statics. Systems receive what they use (a `Logger&`, an allocator,
  the world) from their caller.
- Public API lives in `engine/include/opus/<module>/`; private headers in
  `engine/src/<module>/` are named `*internal*.hpp`.

### Types and errors

- **No exceptions, no RTTI** (`-fno-exceptions -fno-rtti` / `/EHs-c- /GR-`).
  Fallible functions return `std::expected<T, E>` where `E` is a
  module-specific `enum class` — never a catch-all error type, never
  `std::string` errors.
- No `void*` in public signatures. No owning raw pointers anywhere —
  ownership is a container, `std::unique_ptr`, or an explicit allocator.
- `reinterpret_cast`, `const_cast`, `std::bit_cast`, `std::launder` need a
  `// safety: <reason>` comment directly above. C-style casts are errors.
- Naming (clang-tidy enforced): types, enumerators, concepts and template
  parameters `PascalCase`; functions, variables, parameters and namespaces
  `snake_case`; private members `snake_case_`; macros `OPUS_UPPER_CASE`.

### Comments

- `///` Doxygen doc on every public declaration (Doxygen warnings fail
  the build). One-line description; `@p` param notes when names aren't
  self-explanatory; a 2–4 line `@code` example for non-trivial functions.
- `//` inline = WHY, not WHAT.

**Forbidden in any comment** (opus-lint enforced):

- **Issue numbers** (`#123`) — they belong in commits / PRs / changesets
- **Version or milestone markers** (`v0.3`, `M5`, `Phase 2`, `Roadmap:`)
- **Change-history narration** (`previously`, `used to return`, `was a
  bug`, `Regression:`, `now fixed`) — write every comment as if the code
  had always been this way
- **AI attribution**

### Suppressions

- `// allow-strict: <reason>` directly above a line silences opus-lint for
  that line. Reviewer-gated — bring a real reason.
- `// NOLINTNEXTLINE(<check>) <reason>` for clang-tidy — never bare, never
  without the check name. clang-tidy reads suppression directives
  anywhere in a comment *or string literal*, so never write the bare
  keyword in prose.
- Warning-silencing `#pragma`s need an `allow-strict` above them.

### Tests

- **Mirror layout** (lint-enforced): every `engine/include/opus/<m>/<f>.hpp`
  and `engine/src/<m>/<f>.cpp` has `tests/<m>/<f>.test.cpp`, and every
  spec mirrors an engine file. Exempt: the barrel (covered by
  `tests/opus.test.cpp`) and `*internal*` files.
- Naming: `TEST_CASE("<symbol>: <behavior>")` (lint-enforced).
- Tests include only public headers `<opus/...>` and `"util.hpp"` — shared
  helpers live in `tests/util.hpp`, don't reinvent per spec.
- Tools under `tools/` keep their specs next to their sources; the CMake
  glob compiles every `*.test.cpp` it finds, so none can exist unrun.
- **Coverage isn't a target; failure paths are.** Happy path + at least one
  failure per fallible function, and every error enumerator exercised.
- No snapshot tests.
- **Every mode passes:** Debug, RelWithDebInfo, Release, MinSizeRel, and
  ASan/UBSan. A test passing only in Debug isn't done.
- CI runs GCC and Clang on Linux, Apple Clang on macOS, and MSVC on
  Windows. Code that compiles only on this Mac isn't done.
- Simulation modules additionally need a **determinism test**: the same
  inputs replayed twice produce identical state checksums.

---

## Build commands (gates)

```bash
cmake --workflow --preset quick    # inner loop — format check + Debug build + tests

cmake --workflow --preset verify   # pre-push — quick + opus-lint + clang-tidy
                                   # (strict, warnings are errors) + Doxygen
                                   # (undocumented public API fails).
                                   # REQUIRED green before pushing.

cmake -P cmake/ci.cmake            # full local matrix — verify + RelWithDebInfo
                                   # + Release + MinSizeRel + ASan/UBSan.
                                   # Required before tagging a release.

cmake --build --preset debug --target format   # apply clang-format
cmake --build --preset debug --target run      # build + launch the sandbox
```
