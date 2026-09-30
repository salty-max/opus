// opus-lint: project rules that clang-tidy cannot express.
//
//   opus-lint [--root <dir>]              whole tree, including layout rules
//   opus-lint [--root <dir>] <file>...    only these files (pre-commit)
//
// Exit codes: 0 clean, 1 violations found, 2 usage or I/O error.

#include "diagnostic.hpp"
#include "paths.hpp"
#include "rules.hpp"
#include "source.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <optional>
#include <print>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr int exit_clean = 0;
constexpr int exit_violations = 1;
constexpr int exit_usage = 2;

constexpr std::array scanned_roots{
    std::string_view{"engine"},
    std::string_view{"tests"},
    std::string_view{"sandbox"},
    std::string_view{"tools"},
};

struct Options {
    fs::path root = ".";
    std::vector<std::string> files;
};

std::optional<Options> parse_options(std::span<char*> args) {
    Options options;
    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        if (arg == "--root") {
            if (i + 1 == args.size()) {
                return std::nullopt;
            }
            options.root = args[++i];
        } else if (arg.starts_with('-')) {
            return std::nullopt;
        } else {
            options.files.emplace_back(arg);
        }
    }
    return options;
}

std::optional<std::string> read_file(const fs::path& path) {
    const std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        return std::nullopt;
    }
    std::ostringstream contents;
    contents << stream.rdbuf();
    return std::move(contents).str();
}

// Repository-relative, forward-slash form of @p path; nullopt when the path
// cannot be resolved or lies outside the root.
std::optional<std::string> repo_relative(const fs::path& root, const fs::path& path) {
    std::error_code path_error;
    std::error_code root_error;
    std::error_code relative_error;
    const fs::path absolute_path = fs::absolute(path, path_error);
    const fs::path absolute_root = fs::absolute(root, root_error);
    const fs::path relative = fs::relative(absolute_path, absolute_root, relative_error);
    if (path_error || root_error || relative_error || relative.empty() || *relative.begin() == "..") {
        return std::nullopt;
    }
    return relative.generic_string();
}

// Every file under the scanned roots. A root that does not exist is skipped;
// any other filesystem error aborts, because a partial scan would pass as clean.
std::optional<std::vector<std::string>> collect_tree(const fs::path& root) {
    std::vector<std::string> paths;
    for (const std::string_view top : scanned_roots) {
        std::error_code error;
        if (!fs::exists(root / top, error)) {
            if (error) {
                std::println(stderr, "opus-lint: cannot access {}: {}", (root / top).string(),
                             error.message());
                return std::nullopt;
            }
            continue;
        }
        for (fs::recursive_directory_iterator it{root / top, error}; it != fs::recursive_directory_iterator{};
             it.increment(error)) {
            if (error) {
                break;
            }
            const bool regular = it->is_regular_file(error);
            if (error) {
                break;
            }
            if (!regular) {
                continue;
            }
            std::optional<std::string> path = repo_relative(root, it->path());
            if (!path) {
                std::println(stderr, "opus-lint: cannot resolve {}", it->path().string());
                return std::nullopt;
            }
            paths.push_back(*std::move(path));
        }
        if (error) {
            std::println(stderr, "opus-lint: cannot scan {}: {}", (root / top).string(), error.message());
            return std::nullopt;
        }
    }
    std::ranges::sort(paths);
    return paths;
}

} // namespace

int main(int argc, char** argv) {
    const std::optional<Options> options = parse_options(std::span{argv, static_cast<std::size_t>(argc)});
    if (!options) {
        std::println(stderr, "usage: opus-lint [--root <dir>] [<file>...]");
        return exit_usage;
    }

    const bool whole_tree = options->files.empty();
    std::vector<std::string> paths;
    if (whole_tree) {
        std::optional<std::vector<std::string>> tree = collect_tree(options->root);
        if (!tree) {
            return exit_usage;
        }
        paths = *std::move(tree);
    } else {
        for (const std::string& file : options->files) {
            std::optional<std::string> path = repo_relative(options->root, file);
            if (!path) {
                std::println(stderr, "opus-lint: {} is not inside {}", file, options->root.string());
                return exit_usage;
            }
            paths.push_back(*std::move(path));
        }
    }

    opus::lint::Diagnostics diagnostics;
    for (const std::string& path : paths) {
        if (!opus::lint::is_lintable(path)) {
            continue;
        }
        const std::optional<std::string> text = read_file(options->root / path);
        if (!text) {
            std::println(stderr, "opus-lint: cannot read {}", path);
            return exit_usage;
        }
        std::ranges::move(opus::lint::lint_file(opus::lint::parse_source(path, *text)),
                          std::back_inserter(diagnostics));
    }
    if (whole_tree) {
        std::ranges::move(opus::lint::check_layout(paths), std::back_inserter(diagnostics));
    }

    for (const opus::lint::Diagnostic& diagnostic : diagnostics) {
        std::println("{}", opus::lint::format(diagnostic));
    }
    if (!diagnostics.empty()) {
        std::println(stderr, "opus-lint: {} violation(s)", diagnostics.size());
        return exit_violations;
    }
    return exit_clean;
}
