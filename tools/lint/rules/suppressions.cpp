#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <array>
#include <cstddef>
#include <regex>
#include <string>
#include <string_view>

namespace opus::lint {

Diagnostics check_suppressions(const SourceFile& file) {
    static const std::regex nolint{R"(\bNOLINT(NEXTLINE|BEGIN)?\b)"};
    static const std::regex nolint_with_reason{R"(\bNOLINT(NEXTLINE|BEGIN)?\([^)]+\)\s*\S)"};
    static const std::regex nolint_next_line{R"(\bNOLINTNEXTLINE\b)"};
    static const std::regex allow_strict{R"(\ballow-strict:)"};
    static const std::regex allow_strict_with_reason{R"(\ballow-strict:\s*\S)"};
    Diagnostics out;
    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        const std::string& comment = file.lines[i].comment;
        if (std::regex_search(comment, nolint) && !std::regex_search(comment, nolint_with_reason)) {
            detail::report(file, i, "suppression",
                           "NOLINT must name its checks and give a reason: `NOLINT(check) reason`", out);
        }
        // A wrapped reason pushes the code a line further down, out of reach.
        const bool next_line_is_code = i + 1 < file.lines.size() && !trim(file.lines[i + 1].code).empty();
        if (std::regex_search(comment, nolint_next_line) && !next_line_is_code) {
            detail::report(file, i, "suppression",
                           "NOLINTNEXTLINE must sit directly above the code it suppresses; keep its reason "
                           "on one line",
                           out);
        }
        if (std::regex_search(comment, allow_strict) &&
            !std::regex_search(comment, allow_strict_with_reason)) {
            detail::report(file, i, "suppression", "allow-strict needs a reason: `// allow-strict: <reason>`",
                           out);
        }
    }
    using detail::pattern;
    static const std::array pragmas{
        pattern(
            R"(^\s*#\s*pragma\s+(clang|GCC)\s+diagnostic\s+ignored|^\s*#\s*pragma\s+warning\s*\(\s*disable)",
            "warning-silencing pragma needs a `// allow-strict: <reason>` comment directly above"),
    };
    detail::report_matches(file, "warning-pragma", detail::Field::Raw, pragmas, out);
    return out;
}

} // namespace opus::lint
