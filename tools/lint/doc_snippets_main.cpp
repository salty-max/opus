// opus-doc-snippets: writes every ```cpp block of the given Markdown files
// as a standalone translation unit, for the check-doc-cpp gate to compile.
//
//   opus-doc-snippets --root <repo> --out <dir> <doc.md>...
//
// Exit codes: 0 written, 2 usage or I/O error.

#include "doc_snippets.hpp"

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
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

constexpr int exit_written = 0;
constexpr int exit_usage = 2;

struct Options {
    fs::path root;
    fs::path out;
    std::vector<std::string> docs;
};

std::optional<Options> parse_options(std::span<char*> args) {
    Options options;
    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        const bool has_value = i + 1 < args.size();
        if (arg == "--root" && has_value) {
            options.root = args[++i];
        } else if (arg == "--out" && has_value) {
            options.out = args[++i];
        } else if (arg.starts_with('-')) {
            return std::nullopt;
        } else {
            options.docs.emplace_back(arg);
        }
    }
    if (options.root.empty() || options.out.empty()) {
        return std::nullopt;
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

// "docs/core.md" -> "docs_core_md", so snippets of every document share one directory.
std::string flatten(std::string_view doc_path) {
    std::string name{doc_path};
    for (char& c : name) {
        if (c == '/' || c == '\\' || c == '.') {
            c = '_';
        }
    }
    return name;
}

} // namespace

int main(int argc, char** argv) {
    const std::optional<Options> options = parse_options(std::span{argv, static_cast<std::size_t>(argc)});
    if (!options) {
        std::println(stderr, "usage: opus-doc-snippets --root <repo> --out <dir> <doc.md>...");
        return exit_usage;
    }

    std::error_code error;
    fs::create_directories(options->out, error);
    if (error) {
        std::println(stderr, "opus-doc-snippets: cannot create {}: {}", options->out.string(),
                     error.message());
        return exit_usage;
    }

    for (const std::string& doc : options->docs) {
        const std::optional<std::string> text = read_file(options->root / doc);
        if (!text) {
            std::println(stderr, "opus-doc-snippets: cannot read {}", doc);
            return exit_usage;
        }
        for (const opus::lint::Snippet& snippet : opus::lint::extract_cpp_snippets(*text)) {
            const fs::path target = options->out / std::format("{}_L{}.cpp", flatten(doc), snippet.line);
            std::ofstream stream{target, std::ios::binary};
            stream << opus::lint::snippet_translation_unit(snippet, doc);
            if (!stream) {
                std::println(stderr, "opus-doc-snippets: cannot write {}", target.string());
                return exit_usage;
            }
        }
    }
    return exit_written;
}
