# Development reference

Long-form companion to [`CLAUDE.md`](../CLAUDE.md). CLAUDE.md holds the
rules that govern every PR (specs first, workflow contract, self-review
loop, code rules, gates). This file holds the structural references read
only when needed: toolchain setup, source layout, the full lint rule list,
branch / commit / changeset conventions, and the release flow.

---

## Toolchain

| Tool | Why | macOS | Linux (CI) | Windows (CI) |
|---|---|---|---|---|
| C++23 compiler | build | Xcode Apple Clang 17+ | GCC 14, Clang 23 | MSVC 2022 |
| CMake ≥ 3.28, Ninja | build system, task runner | `brew install cmake ninja` | apt | choco |
| clang-format, clang-tidy 23 | `format`, `verify` | `brew install llvm clang-format` | apt.llvm.org | — |
| Doxygen | `docs` gate | `brew install doxygen` | apt | — |
| lefthook, convco | git hooks | `brew install lefthook convco` | — | — |
| actionlint | workflow edits | `brew install actionlint` | — | — |

clang-format and clang-tidy are **pinned to one LLVM major version**
(`OPUS_LLVM_VERSION` in `cmake/OpusLlvmTools.cmake`, mirrored by
`LLVM_VERSION` in `ci.yml`): formatting and diagnostics change between
majors, so configuration fails if only another version is found. The build
prefers `clang-tidy-23` / `clang-format-23` (Debian and Ubuntu install these
next to an older unversioned default), then Homebrew's keg-only LLVM under
`/opt/homebrew/opt/llvm/bin`, then `PATH`. Bumping LLVM means changing both
constants and reformatting in the same PR.

On Debian/Ubuntu, `scripts/install-linux-deps.sh` installs Ninja and the
development headers SDL3 builds its Linux backends against (CI runs the same
script).

After cloning, once:

```bash
lefthook install
cmake --workflow --preset quick
```

Editors: every preset writes `compile_commands.json` into its build
directory. Point clangd at `build/debug` (or symlink
`ln -sf build/debug/compile_commands.json .`).

**Apple Clang ASan on macOS 26.** Apple Clang 17's AddressSanitizer
runtime hangs at startup on macOS 26 — even for an empty `main`. The
sanitizer lane on macOS therefore builds with Homebrew LLVM (the
`asan-macos` preset); `cmake/ci.cmake` picks it automatically.

---

## Source layout

```
engine/
├── include/opus/          # public API — games include only this
│   ├── opus.hpp           # barrel: the whole public API
│   ├── version.hpp.in     # configured into the build tree
│   └── <module>/<f>.hpp
└── src/<module>/          # implementation; private headers are *internal*.hpp
tests/
├── main.cpp               # doctest runner
├── util.hpp               # shared test helpers
├── opus.test.cpp          # barrel smoke test
└── <module>/<f>.test.cpp  # mirrors engine/{include/opus,src}/<module>/<f>
sandbox/                   # sample game used during development
tools/lint/                # opus-lint + opus-doc-snippets; specs sit beside their sources
cmake/                     # compile policy, deps, gates, ci driver
docs/                      # specs + this file
scripts/                   # hook + changeset helpers
.changeset/                # pending changesets
.github/workflows/         # CI, changeset check, release
```

Engine modules (from [`PLAN.md`](../PLAN.md)): `core`, `platform`,
`render`, `ecs`, `sim`, `world`, `rts`, `script`, `audio`, `net`, `ui`,
`devtools`. `ecs`, `sim`, `world` and `rts` are **simulation modules**
bound by [`determinism.md`](determinism.md).

---

## Gates

Three layered gates, all plain CMake so they run identically on every OS.

| Gate | Command | Runs |
|---|---|---|
| quick | `cmake --workflow --preset quick` | clang-format check, Debug build, all tests |
| verify | `cmake --workflow --preset verify` | quick + opus-lint (whole tree) + clang-tidy on every TU + Doxygen with warnings as errors + doc examples compile |
| ci | `cmake -P cmake/ci.cmake` | verify + RelWithDebInfo + Release + MinSizeRel + ASan/UBSan |

`verify` builds in `build/verify` with clang-tidy wired into compilation,
so re-runs only re-check what changed. Individual targets:

| Target | Does |
|---|---|
| `format` / `format-check` | apply / check clang-format over engine, tests, sandbox, tools |
| `lint` | opus-lint over the whole tree |
| `docs` | Doxygen HTML into `build/<preset>/docs/html` |
| `check-doc-cpp` | compile every ```` ```cpp ```` block in `docs/` and `README.md` |
| `run` | build and launch the sandbox |

GitHub Actions runs `verify` on Linux (Clang 23), the test workflows
across GCC / Clang / Apple Clang / MSVC and every build mode, a
sanitizer lane on Linux and macOS, and a commit-message check on PRs.

---

## Documentation gate

`check-doc-cpp` (part of `verify`) compiles every ```` ```cpp ```` block in
`docs/**/*.md` and `README.md` against the public headers, with warnings as
errors. A spec example is what a reader copies — one that no longer compiles
teaches a wrong API silently. `opus-doc-snippets` extracts the blocks with a
`#line` directive, so a failure points at the Markdown line.

**Only tagged blocks are checked.** An untagged fence is prose (a signature
table, a directory tree) and is skipped; tag a block ```` ```cpp ```` when it
is C++, which also gives it highlighting.

A block that genuinely cannot stand alone — an elided body (`...`), a form
the section documents *as* an error — starts with `// fragment: <why>`. If a
block fails because the API changed under it, fix the block.

Doxygen `@code` examples in headers are fragments by nature and are not
compiled; keep them short and let the specs carry full programs.

---

## Compiler policy

Set in `cmake/OpusCompileOptions.cmake` and applied to every first-party
target:

- `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow
  -Wold-style-cast` and more (GCC adds `-Wduplicated-*`, `-Wlogical-op`,
  `-Wuseless-cast`); MSVC `/W4 /permissive-`. Warnings are errors.
- Engine and tools: `-fno-exceptions -fno-rtti` (`/EHs-c- /GR-`). Test
  binaries keep exceptions because doctest reports through them.
- `OPUS_SANITIZE=address,undefined` instruments the whole build.

clang-tidy (`.clang-tidy`) enables `bugprone`, `cert`, `clang-analyzer`,
`concurrency`, `cppcoreguidelines`, `misc` (including `include-cleaner`),
`modernize`, `performance`, `portability` and `readability`, all as
errors, minus a short documented list. The same profile covers every
first-party file, specs included — doctest is a `SYSTEM` dependency, so its
macro expansions are not checked.

---

## opus-lint

`tools/lint`, built as `opus-lint`. Covers what clang-tidy cannot express.
Whole-tree mode (`opus-lint --root .`, the `lint` target) adds the layout
rules; per-file mode (`opus-lint --root . <files>`, the pre-commit hook)
skips them.

| Rule | Scope | Fails on |
|---|---|---|
| `comment-content` | everywhere | issue numbers, version / milestone / roadmap markers, change-history narration, AI attribution — in comments |
| `cast-safety` | everywhere | `reinterpret_cast`, `const_cast`, `std::bit_cast`, `std::launder` without `// safety: <reason>` directly above |
| `include-path` | everywhere | `#include "../…"`; quoted includes in public headers; SDL includes in public headers; quoted includes other than `"util.hpp"` in `tests/` |
| `engine-io` | `engine/` | `std::cout`/`cerr`/`print`, `printf`-family output, any `stdout`/`stderr`, `<iostream>`/`<print>` |
| `raw-memory` | `engine/` | `new`, `delete`, `malloc`/`calloc`/`realloc`/`free` |
| `determinism` | `ecs`, `sim`, `world`, `rts` | `float`/`double`, libm calls, `std::unordered_*`, clocks, platform RNGs and `<random>` distributions, unstable sorts, and their headers |
| `test-name` | `*.test.cpp` | `TEST_CASE` not named `"<symbol>: <behavior>"` on one line |
| `suppression` | everywhere | clang-tidy suppressions without check names and a reason; a next-line suppression not directly above code (a wrapped reason pushes the code out of reach); `allow-strict:` without a reason |
| `warning-pragma` | everywhere | `#pragma … diagnostic ignored` / `warning(disable…)` |
| `pragma-once` | headers | missing `#pragma once` |
| `mirror` | tree | engine file without its spec; spec without its engine file |
| `layout` | tree | private engine header not named `*internal*.hpp` |

Silence one line with `// allow-strict: <reason>` directly above it (a
multi-line comment block whose first line carries the marker counts).
`suppression` findings cannot be silenced.

Adding a rule: one file in `tools/lint/rules/`, its spec beside it, the
function in `rules.hpp`, the entry in `lint.cpp`, and a row in the table
above — in the same PR.

---

## Third-party dependencies

Declared in `cmake/OpusDependencies.cmake`, in two groups: **engine**
dependencies, fetched for every consumer of the library, and **development**
dependencies, fetched only when Opus is the top-level project.

- **Pinned by content.** Release tarballs carry a `URL_HASH SHA256=…`; git
  sources name an exact tag.
- **Outside the gates.** Third-party targets get neither clang-tidy nor the
  first-party warning policy, and their headers are `SYSTEM`.
- **Private to the engine.** Engine dependencies link `PRIVATE`; no
  third-party header appears under `engine/include/opus/` (opus-lint
  enforces it for SDL).
- **Packaged.** The installed `opusConfig.cmake` calls `find_dependency` for
  each engine dependency, the dependency is installed alongside the engine,
  and its license file ships under `share/licenses/opus/`. The
  `opusConfig` ctest installs the engine and builds a `find_package(opus)`
  consumer against it, so a broken package fails the test suite.
- **Recorded.** Each dependency gets a row in
  [`THIRD_PARTY.md`](../THIRD_PARTY.md) in the PR that adds it.

---

## Roadmap and issues

The roadmap is the [Opus project board](https://github.com/users/salty-max/projects/9).
Its **Phase** field maps each issue to a [`PLAN.md`](../PLAN.md) milestone;
the plain issue list does not show it. Each issue is one PR-sized unit with
acceptance criteria, `kind/*` and `area/*` labels, and a `Depends on` list.

---

## Branches + commits + changesets

**Branches:** `feat/<short>`, `fix/<short>`, `perf/<short>`,
`chore/<short>`, `docs/<short>`, `refactor/<short>` — from `main`.

**Commits:** Conventional Commits, scope mandatory, validated by the
commit-msg hook (`scripts/commit-msg.sh` → convco with `.versionrc`) and
again in CI for every commit of a PR.

- No scope-less commits (`feat: add x` → rejected)
- Multi-concern changes split into multiple commits
- `fixup!` for review feedback, then `--autosquash`
- Tooling-only `perf` → `chore(tooling)`

**Scopes:**

```
core platform render ecs sim world rts script audio net ui devtools
                     → engine/{include/opus,src}/<module>/
<module>/<sub>       → e.g. render/batch, sim/rng
sandbox              → sandbox/
editor               → editor/
shaders              → shaders/
tooling              → CMake, presets, cmake/, scripts/, tools/, hooks
ci                   → .github/workflows/
docs                 → docs/, README, doc comments
meta                 → top-level repo files (CLAUDE.md, PLAN.md, configs)
```

**Changesets** — every PR with a user-visible change adds
`.changeset/<id>.md`; see [`.changeset/README.md`](../.changeset/README.md).

- **Add one** for `feat`, `fix`, library-level `perf`, breaking refactor.
- **Skip** for `chore`, `docs`, `test`, internal `refactor`, `ci`,
  `build`, `style` — the `changeset-check` workflow skips these titles.

```bash
scripts/changeset-new.sh minor "Add opus::Logger with level filtering."
```

---

## Releases (manual)

Merged PRs accumulate changesets on `main`; the maintainer cuts a release
when a batch is coherent. Pushing to `main` never publishes — only a
`vX.Y.Z` tag triggers `release.yml`, which checks the tag against
`CMakeLists.txt`, builds and tests Release on all three OSes, packages
each with CPack, and publishes the GitHub release with the CHANGELOG
section as notes.

```bash
git checkout main && git pull
cmake -P cmake/ci.cmake              # full matrix green
scripts/changeset-version.sh         # consume changesets, bump version, prepend CHANGELOG
git diff                             # review; edit the CHANGELOG narrative if needed
git add . && git commit -m "chore(meta): release vX.Y.Z"
git tag vX.Y.Z
git push origin main --tags
```

Below 1.0 bump levels shift one place right (a `major` changeset moves the
minor); the script never produces `1.0.0` on its own.
