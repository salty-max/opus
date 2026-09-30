#pragma once

#include "rules.hpp"
#include "source.hpp"

#include <regex>
#include <span>
#include <string_view>

namespace opus::lint::detail {

/// Which part of a SourceLine a pattern is matched against.
enum class Field : unsigned char { Raw, Code, Comment };

/// A forbidden pattern and the explanation reported when it matches.
struct Pattern {
    std::regex regex;         ///< Searched anywhere in the chosen field.
    std::string_view message; ///< Reported verbatim on a match.
};

/// Builds a Pattern; @p icase makes the match case-insensitive.
[[nodiscard]] Pattern pattern(const char* regex, std::string_view message, bool icase = false);

/// Reports the first matching pattern of @p patterns on every line of @p file.
void report_matches(const SourceFile& file, std::string_view rule, Field field,
                    std::span<const Pattern> patterns, Diagnostics& out);

/// Appends a diagnostic for 0-based line @p index of @p file.
void report(const SourceFile& file, std::size_t index, std::string_view rule, std::string_view message,
            Diagnostics& out);

} // namespace opus::lint::detail
