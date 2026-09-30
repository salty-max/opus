#include "diagnostic.hpp"
#include "paths.hpp"
#include "rules.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <set>
#include <span>
#include <string>
#include <string_view>

namespace opus::lint {

namespace {

constexpr std::string_view public_root = "engine/include/opus/";
constexpr std::string_view private_root = "engine/src/";
constexpr std::string_view tests_root = "tests/";
constexpr std::string_view spec_suffix = ".test.cpp";

// The barrel re-exports the API and is covered by tests/opus.test.cpp.
constexpr std::string_view barrel = "engine/include/opus/opus.hpp";
constexpr std::string_view barrel_spec = "tests/opus.test.cpp";

bool is_internal(std::string_view path) {
    const std::size_t slash = path.rfind('/');
    const std::string_view name = slash == std::string_view::npos ? path : path.substr(slash + 1);
    return name.contains("internal");
}

std::string_view strip_extension(std::string_view path, std::string_view extension) {
    return path.substr(0, path.size() - extension.size());
}

// The module-relative stem an engine file is mirrored by, e.g.
// "engine/src/core/log.cpp" -> "core/log". Empty when the file needs no spec.
std::string engine_stem(std::string_view path) {
    if (path == barrel || is_internal(path)) {
        return {};
    }
    if (path.starts_with(public_root) && path.ends_with(".hpp")) {
        return std::string{strip_extension(path.substr(public_root.size()), ".hpp")};
    }
    if (path.starts_with(private_root) && path.ends_with(".cpp")) {
        return std::string{strip_extension(path.substr(private_root.size()), ".cpp")};
    }
    return {};
}

} // namespace

Diagnostics check_layout(std::span<const std::string> repo_paths) {
    const std::set<std::string_view> present(repo_paths.begin(), repo_paths.end());
    std::set<std::string> engine_stems;
    Diagnostics out;

    for (const std::string& path : repo_paths) {
        if (path.starts_with(private_root) && is_header(path) && !is_internal(path)) {
            out.push_back({.path = path,
                           .line = 0,
                           .rule = "layout",
                           .message = "private engine header must be named *internal*.hpp; "
                                      "public headers live in engine/include/opus/"});
        }
        const std::string stem = engine_stem(path);
        if (stem.empty()) {
            continue;
        }
        engine_stems.insert(stem);
        const std::string spec = std::format("{}{}{}", tests_root, stem, spec_suffix);
        if (!present.contains(spec)) {
            out.push_back({.path = path,
                           .line = 0,
                           .rule = "mirror",
                           .message = std::format("missing mirrored spec {}", spec)});
        }
    }

    for (const std::string& path : repo_paths) {
        if (!path.starts_with(tests_root) || !path.ends_with(spec_suffix) || path == barrel_spec) {
            continue;
        }
        const std::string stem{strip_extension(path.substr(tests_root.size()), spec_suffix)};
        if (!engine_stems.contains(stem)) {
            out.push_back({.path = path,
                           .line = 0,
                           .rule = "mirror",
                           .message = std::format("spec mirrors no engine file ({}{}.hpp or {}{}.cpp)",
                                                  public_root, stem, private_root, stem)});
        }
    }

    std::ranges::sort(out, {}, &Diagnostic::path);
    return out;
}

} // namespace opus::lint
