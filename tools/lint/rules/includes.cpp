#include "paths.hpp"
#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <cstddef>
#include <regex>
#include <string>

namespace opus::lint {

Diagnostics check_includes(const SourceFile& file) {
    static const std::regex quoted_include{R"re(^\s*#\s*include\s*"([^"]*)")re"};
    static const std::regex sdl_include{R"re(^\s*#\s*include\s*[<"]SDL)re"};
    Diagnostics out;
    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        // SDL is a private dependency of the engine; games never see it.
        if (is_public_header(file.path) && std::regex_search(file.lines[i].raw, sdl_include)) {
            detail::report(file, i, "include-path",
                           "public headers never include SDL; keep platform types behind engine/src", out);
            continue;
        }
        std::smatch match;
        if (!std::regex_search(file.lines[i].raw, match, quoted_include)) {
            continue;
        }
        const std::string target = match[1].str();
        if (target.contains("../")) {
            detail::report(file, i, "include-path",
                           "relative `../` include; include through the target's include directory", out);
        } else if (is_public_header(file.path)) {
            detail::report(file, i, "include-path",
                           "public headers include with angle brackets: `#include <opus/...>`", out);
        } else if (is_in_tests(file.path) && target != "util.hpp") {
            detail::report(file, i, "include-path",
                           "tests include public headers `<opus/...>` and \"util.hpp\" only", out);
        }
    }
    return out;
}

} // namespace opus::lint
