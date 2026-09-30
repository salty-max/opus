#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace opus::lint {

/// One ```cpp block of a Markdown document.
struct Snippet {
    std::size_t line; ///< 1-based line of the block's first code line.
    std::string code; ///< Block contents, newline-terminated lines.
};

/// Every ```cpp block of @p markdown, in order, except blocks whose first
/// line is `// fragment: <why>` — those document code that cannot stand
/// alone (an elided body, a form shown as an error) and are skipped.
[[nodiscard]] std::vector<Snippet> extract_cpp_snippets(std::string_view markdown);

/// @p snippet as a standalone translation unit whose `#line` directive maps
/// compiler diagnostics back to @p doc_path.
[[nodiscard]] std::string snippet_translation_unit(const Snippet& snippet, std::string_view doc_path);

} // namespace opus::lint
