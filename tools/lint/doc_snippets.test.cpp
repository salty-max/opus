#include "doc_snippets.hpp"

#include <doctest/doctest.h>

#include <vector>

using opus::lint::extract_cpp_snippets;
using opus::lint::Snippet;
using opus::lint::snippet_translation_unit;

TEST_CASE("extract_cpp_snippets: returns each cpp block with its first code line") {
    const auto snippets = extract_cpp_snippets("# Title\n"
                                               "```cpp\n"
                                               "int a = 1;\n"
                                               "int b = 2;\n"
                                               "```\n"
                                               "text\n"
                                               "  ```cpp\n"
                                               "int c = 3;\n"
                                               "  ```\n");
    REQUIRE(snippets.size() == 2);
    CHECK(snippets[0].line == 3);
    CHECK(snippets[0].code == "int a = 1;\nint b = 2;\n");
    CHECK(snippets[1].line == 8);
    CHECK(snippets[1].code == "int c = 3;\n");
}

TEST_CASE("extract_cpp_snippets: ignores blocks in other languages and untagged blocks") {
    CHECK(extract_cpp_snippets("```bash\nls\n```\n```\nplain\n```\n```cpp-like\nx\n```\n").empty());
}

TEST_CASE("extract_cpp_snippets: skips blocks marked as fragments") {
    CHECK(extract_cpp_snippets("```cpp\n// fragment: body elided\nvoid f() { ... }\n```\n").empty());
}

TEST_CASE("extract_cpp_snippets: keeps a block whose fragment marker has no reason") {
    CHECK(extract_cpp_snippets("```cpp\n// fragment:\nint x;\n```\n").size() == 1);
}

TEST_CASE("extract_cpp_snippets: drops an unterminated block") {
    CHECK(extract_cpp_snippets("```cpp\nint x;\n").empty());
}

TEST_CASE("extract_cpp_snippets: handles CRLF line endings") {
    const auto snippets = extract_cpp_snippets("```cpp\r\nint x;\r\n```\r\n");
    REQUIRE(snippets.size() == 1);
    CHECK(snippets[0].code == "int x;\n");
}

TEST_CASE("snippet_translation_unit: maps diagnostics back to the document") {
    const Snippet snippet{.line = 12, .code = "int x;\n"};
    CHECK(snippet_translation_unit(snippet, "docs/core.md") == "#line 12 \"docs/core.md\"\nint x;\n");
}
