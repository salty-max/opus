#include "doc_snippets.hpp"

#include "source.hpp"

#include <cstddef>
#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace opus::lint {

namespace {

constexpr std::string_view fence = "```";
constexpr std::string_view cpp_fence = "```cpp";
constexpr std::string_view fragment_marker = "// fragment:";

bool is_fragment(const Snippet& snippet) {
    const std::string_view first_line = std::string_view{snippet.code}.substr(0, snippet.code.find('\n'));
    const std::string_view text = trim(first_line);
    return text.starts_with(fragment_marker) && !trim(text.substr(fragment_marker.size())).empty();
}

} // namespace

std::vector<Snippet> extract_cpp_snippets(std::string_view markdown) {
    std::vector<Snippet> snippets;
    Snippet current{};
    bool in_block = false;
    std::size_t line_number = 0;
    while (!markdown.empty()) {
        const std::size_t newline = markdown.find('\n');
        std::string_view line = markdown.substr(0, newline);
        markdown.remove_prefix(newline == std::string_view::npos ? markdown.size() : newline + 1);
        ++line_number;
        if (line.ends_with('\r')) {
            line.remove_suffix(1);
        }

        const std::string_view marker = trim(line);
        if (!in_block) {
            if (marker == cpp_fence) {
                current = Snippet{.line = line_number + 1, .code = {}};
                in_block = true;
            }
        } else if (marker.starts_with(fence)) {
            in_block = false;
            if (!is_fragment(current)) {
                snippets.push_back(current);
            }
        } else {
            current.code += line;
            current.code += '\n';
        }
    }
    return snippets;
}

std::string snippet_translation_unit(const Snippet& snippet, std::string_view doc_path) {
    return std::format("#line {} \"{}\"\n{}", snippet.line, doc_path, snippet.code);
}

} // namespace opus::lint
