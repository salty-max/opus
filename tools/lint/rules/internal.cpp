#include "rules/internal.hpp"
#include "rules.hpp"
#include "source.hpp"

#include <cstddef>
#include <regex>
#include <span>
#include <string>
#include <string_view>

namespace opus::lint::detail {

namespace {

const std::string& field_of(const SourceLine& line, Field field) {
    switch (field) {
    case Field::Raw:
        return line.raw;
    case Field::Code:
        return line.code;
    case Field::Comment:
        return line.comment;
    }
    return line.raw;
}

} // namespace

Pattern pattern(const char* regex, std::string_view message, bool icase) {
    auto flags = std::regex::ECMAScript | std::regex::optimize;
    if (icase) {
        flags |= std::regex::icase;
    }
    return {.regex = std::regex{regex, flags}, .message = message};
}

void report(const SourceFile& file, std::size_t index, std::string_view rule, std::string_view message,
            Diagnostics& out) {
    out.push_back({.path = file.path, .line = index + 1, .rule = rule, .message = std::string{message}});
}

void report_matches(const SourceFile& file, std::string_view rule, Field field,
                    std::span<const Pattern> patterns, Diagnostics& out) {
    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        const std::string& text = field_of(file.lines[i], field);
        if (text.empty()) {
            continue;
        }
        for (const Pattern& candidate : patterns) {
            if (std::regex_search(text, candidate.regex)) {
                report(file, i, rule, candidate.message, out);
                break;
            }
        }
    }
}

} // namespace opus::lint::detail
