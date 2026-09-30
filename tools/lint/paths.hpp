#pragma once

#include <string_view>

namespace opus::lint {

/// Whether @p path (repository-relative) is part of the engine library.
[[nodiscard]] bool is_engine(std::string_view path);

/// Whether @p path is a public engine header under engine/include/opus/.
[[nodiscard]] bool is_public_header(std::string_view path);

/// Whether @p path is a test spec (`*.test.cpp`) anywhere in the tree.
[[nodiscard]] bool is_test_spec(std::string_view path);

/// Whether @p path lives in the tests/ tree.
[[nodiscard]] bool is_in_tests(std::string_view path);

/// Whether @p path is a C++ header (`.hpp`, `.h`, or a configured `.hpp.in`).
[[nodiscard]] bool is_header(std::string_view path);

/// Whether @p path belongs to an engine module whose code runs inside the
/// lockstep simulation (docs/determinism.md) and must be bit-reproducible.
[[nodiscard]] bool is_deterministic_module(std::string_view path);

/// Whether opus-lint checks files with @p path's extension at all.
[[nodiscard]] bool is_lintable(std::string_view path);

} // namespace opus::lint
