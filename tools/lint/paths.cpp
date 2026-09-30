#include "paths.hpp"

#include <array>
#include <string_view>

namespace opus::lint {

namespace {

constexpr std::array deterministic_modules{
    std::string_view{"ecs/"},
    std::string_view{"sim/"},
    std::string_view{"world/"},
    std::string_view{"rts/"},
};

constexpr std::array engine_module_roots{
    std::string_view{"engine/include/opus/"},
    std::string_view{"engine/src/"},
};

} // namespace

bool is_engine(std::string_view path) {
    return path.starts_with("engine/");
}

bool is_public_header(std::string_view path) {
    return path.starts_with("engine/include/opus/") && is_header(path);
}

bool is_test_spec(std::string_view path) {
    return path.ends_with(".test.cpp");
}

bool is_in_tests(std::string_view path) {
    return path.starts_with("tests/");
}

bool is_header(std::string_view path) {
    return path.ends_with(".hpp") || path.ends_with(".h") || path.ends_with(".hpp.in");
}

bool is_deterministic_module(std::string_view path) {
    for (const std::string_view root : engine_module_roots) {
        if (!path.starts_with(root)) {
            continue;
        }
        const std::string_view module_path = path.substr(root.size());
        for (const std::string_view module : deterministic_modules) {
            if (module_path.starts_with(module)) {
                return true;
            }
        }
    }
    return false;
}

bool is_lintable(std::string_view path) {
    return path.ends_with(".cpp") || is_header(path);
}

} // namespace opus::lint
