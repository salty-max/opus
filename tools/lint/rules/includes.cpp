#include "paths.hpp"
#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <cstddef>
#include <regex>
#include <string>
#include <string_view>

namespace opus::lint {

namespace {

// SDL is the engine's private dependency: only the engine's sources use it,
// and the platform specs, which play the operating system through it.
bool may_include_sdl(std::string_view path) {
    return path.starts_with("engine/src/") || path.starts_with("tests/platform/");
}

// Specs include the shared "util.hpp" and their own module's
// "<module>/util.hpp" (tests/platform/x.test.cpp may include "platform/util.hpp").
bool is_test_helper_for(std::string_view target, std::string_view path) {
    if (target == "util.hpp") {
        return true;
    }
    const std::string_view module_dir = path.substr(0, path.rfind('/') + 1);
    return module_dir.starts_with("tests/") && module_dir != "tests/" &&
           target == std::string{module_dir.substr(std::string_view{"tests/"}.size())} + "util.hpp";
}

} // namespace

Diagnostics check_includes(const SourceFile& file) {
    static const std::regex quoted_include{R"re(^\s*#\s*include\s*"([^"]*)")re"};
    static const std::regex sdl_include{R"re(^\s*#\s*include\s*[<"]SDL)re"};
    Diagnostics out;
    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        if (std::regex_search(file.lines[i].raw, sdl_include) && !may_include_sdl(file.path)) {
            detail::report(file, i, "include-path",
                           "SDL is private to the engine: only engine/src and tests/platform include it",
                           out);
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
        } else if (is_in_tests(file.path) && !is_test_helper_for(target, file.path)) {
            detail::report(file, i, "include-path",
                           "tests include public headers `<opus/...>`, \"util.hpp\" and their module's "
                           "\"<module>/util.hpp\" only",
                           out);
        }
    }
    return out;
}

} // namespace opus::lint
