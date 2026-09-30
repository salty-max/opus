#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace opus::lint {

/// One physical line of a source file, split by lexical role.
struct SourceLine {
    std::string raw;     ///< The line exactly as written.
    std::string code;    ///< Code only: comments removed, string and char literal contents blanked.
    std::string comment; ///< Text of every comment on the line, markers stripped.
};

/// A source file prepared for rule checks.
struct SourceFile {
    std::string path;              ///< Repository-relative path with forward slashes.
    std::vector<SourceLine> lines; ///< Lines in order; index 0 is line 1.
};

/// Splits @p text into lines and separates code from comments, so rules
/// match code patterns without tripping over prose and vice versa.
///
/// Understands line and block comments, string and char literals with
/// escapes, raw string literals, and digit separators (`1'000`).
[[nodiscard]] SourceFile parse_source(std::string path, std::string_view text);

/// Whether the contiguous comment-only lines directly above @p index hold
/// a comment starting with @p marker followed by a non-empty reason.
///
/// @code
/// // safety: the buffer is aligned for T by construction
/// auto* p = reinterpret_cast<T*>(buffer);   // has_marker_above(file, i, "safety:")
/// @endcode
[[nodiscard]] bool has_marker_above(const SourceFile& file, std::size_t index, std::string_view marker);

/// @p text with leading and trailing whitespace removed.
[[nodiscard]] std::string_view trim(std::string_view text);

} // namespace opus::lint
