#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace opus::lint {

/// One rule violation at a specific line of a repository file.
struct Diagnostic {
    std::string path;      ///< Repository-relative path with forward slashes.
    std::size_t line;      ///< 1-based line number; 0 for whole-file findings.
    std::string_view rule; ///< Stable rule identifier, e.g. "cast-safety".
    std::string message;   ///< Human-readable explanation of the violation.
};

/// Renders @p diagnostic as `path:line: error[rule]: message`, the shape
/// editors and CI annotations parse.
[[nodiscard]] std::string format(const Diagnostic& diagnostic);

} // namespace opus::lint
