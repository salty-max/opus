#pragma once

#include "diagnostic.hpp"
#include "source.hpp"

#include <span>
#include <string>
#include <vector>

namespace opus::lint {

/// Diagnostics produced by one rule over one file.
using Diagnostics = std::vector<Diagnostic>;

/// Forbidden comment content: issue references, version or milestone
/// markers, change-history narration, and AI attribution.
[[nodiscard]] Diagnostics check_comments(const SourceFile& file);

/// reinterpret_cast, const_cast, std::bit_cast and std::launder each need
/// a `// safety: <reason>` comment directly above.
[[nodiscard]] Diagnostics check_casts(const SourceFile& file);

/// No `../` includes; public headers use angle includes only; tests
/// include only public headers and their shared "util.hpp".
[[nodiscard]] Diagnostics check_includes(const SourceFile& file);

/// The engine never writes to stdout/stderr directly; it logs.
[[nodiscard]] Diagnostics check_engine_io(const SourceFile& file);

/// The engine never uses new/delete/malloc/free directly.
[[nodiscard]] Diagnostics check_raw_memory(const SourceFile& file);

/// Simulation modules use no floating point, clocks, platform RNGs,
/// unordered containers or unstable sorts.
[[nodiscard]] Diagnostics check_determinism(const SourceFile& file);

/// Every TEST_CASE is named `"<symbol>: <behavior>"`.
[[nodiscard]] Diagnostics check_test_names(const SourceFile& file);

/// Every clang-tidy suppression names its checks and gives a reason; every
/// allow-strict gives a reason; warning-silencing pragmas need an allow-strict.
[[nodiscard]] Diagnostics check_suppressions(const SourceFile& file);

/// Every header starts its include guard with `#pragma once`.
[[nodiscard]] Diagnostics check_pragma_once(const SourceFile& file);

/// Tree-level layout: every engine source has a mirrored spec under
/// tests/, every spec has an engine source, and private engine headers
/// are named `*internal*.hpp`.
[[nodiscard]] Diagnostics check_layout(std::span<const std::string> repo_paths);

/// Runs every per-file rule on @p file and drops findings silenced by a
/// `// allow-strict: <reason>` comment directly above their line.
[[nodiscard]] Diagnostics lint_file(const SourceFile& file);

} // namespace opus::lint
