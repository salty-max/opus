#include "paths.hpp"
#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <cstddef>
#include <regex>

namespace opus::lint {

Diagnostics check_test_names(const SourceFile& file) {
    Diagnostics out;
    if (!is_test_spec(file.path)) {
        return out;
    }
    static const std::regex test_case{R"(\bTEST_CASE\s*\()"};
    static const std::regex well_named{R"(\bTEST_CASE\s*\(\s*"\S+: \S[^"]*")"};
    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        if (std::regex_search(file.lines[i].code, test_case) &&
            !std::regex_search(file.lines[i].raw, well_named)) {
            detail::report(file, i, "test-name",
                           "name tests `TEST_CASE(\"<symbol>: <behavior>\")` on one line", out);
        }
    }
    return out;
}

} // namespace opus::lint
