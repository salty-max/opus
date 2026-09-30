#include "source.hpp"

#include <doctest/doctest.h>

using opus::lint::has_marker_above;
using opus::lint::parse_source;
using opus::lint::trim;

TEST_CASE("parse_source: splits code from a line comment") {
    const auto file = parse_source("a.cpp", "int x = 1; // the answer\n");
    REQUIRE(file.lines.size() == 1);
    CHECK(file.lines[0].code == "int x = 1; ");
    CHECK(file.lines[0].comment == "the answer");
    CHECK(file.lines[0].raw == "int x = 1; // the answer");
}

TEST_CASE("parse_source: strips doc-comment markers") {
    const auto file = parse_source("a.hpp", "/// Documented.\nint x; ///< Trailing.\n");
    CHECK(file.lines[0].comment == "Documented.");
    CHECK(file.lines[1].comment == "Trailing.");
}

TEST_CASE("parse_source: tracks block comments across lines") {
    const auto file = parse_source("a.cpp", "a /* one\ntwo\nthree */ b\n");
    REQUIRE(file.lines.size() == 3);
    CHECK(file.lines[0].code == "a ");
    CHECK(file.lines[0].comment == "one");
    CHECK(file.lines[1].code.empty());
    CHECK(file.lines[1].comment == "two");
    CHECK(file.lines[2].code == " b");
    CHECK(file.lines[2].comment == "three");
}

TEST_CASE("parse_source: blanks string literal contents") {
    const auto file = parse_source("a.cpp", R"(auto s = "// not a comment \" still string";)");
    CHECK(file.lines[0].code == R"(auto s = "";)");
    CHECK(file.lines[0].comment.empty());
}

TEST_CASE("parse_source: blanks char literals, including an escaped quote") {
    const auto file = parse_source("a.cpp", R"(char c = '\''; char d = '"'; // tail)");
    CHECK(file.lines[0].code == "char c = ''; char d = ''; ");
    CHECK(file.lines[0].comment == "tail");
}

TEST_CASE("parse_source: keeps digit separators as code") {
    const auto file = parse_source("a.cpp", "int n = 1'000'000; // big\n");
    CHECK(file.lines[0].code == "int n = 1'000'000; ");
    CHECK(file.lines[0].comment == "big");
}

TEST_CASE("parse_source: skips raw strings that span lines") {
    const auto file = parse_source("a.cpp", "auto r = R\"x(line // one\n\" still )x\"; // real\n");
    REQUIRE(file.lines.size() == 2);
    CHECK(file.lines[0].code == "auto r = R\"");
    CHECK(file.lines[0].comment.empty());
    CHECK(file.lines[1].code == "\"; ");
    CHECK(file.lines[1].comment == "real");
}

TEST_CASE("parse_source: handles CRLF and a missing final newline") {
    const auto file = parse_source("a.cpp", "a\r\nb");
    REQUIRE(file.lines.size() == 2);
    CHECK(file.lines[0].raw == "a");
    CHECK(file.lines[1].raw == "b");
}

TEST_CASE("parse_source: an unterminated string ends at the line break") {
    const auto file = parse_source("a.cpp", "\"open\nint x; // c\n");
    CHECK(file.lines[1].code == "int x; ");
    CHECK(file.lines[1].comment == "c");
}

TEST_CASE("has_marker_above: finds a marker directly above") {
    const auto file = parse_source("a.cpp", "// safety: aligned\ncast();\n");
    CHECK(has_marker_above(file, 1, "safety:"));
}

TEST_CASE("has_marker_above: accepts a marker opening a multi-line comment") {
    const auto file = parse_source("a.cpp", "// safety: aligned because\n// the arena rounds up\ncast();\n");
    CHECK(has_marker_above(file, 2, "safety:"));
}

TEST_CASE("has_marker_above: rejects a marker without a reason") {
    const auto file = parse_source("a.cpp", "// safety:\ncast();\n");
    CHECK_FALSE(has_marker_above(file, 1, "safety:"));
}

TEST_CASE("has_marker_above: rejects a marker separated by code or a blank line") {
    const auto separated_by_code = parse_source("a.cpp", "// safety: ok\nint x;\ncast();\n");
    CHECK_FALSE(has_marker_above(separated_by_code, 2, "safety:"));
    const auto separated_by_blank = parse_source("a.cpp", "// safety: ok\n\ncast();\n");
    CHECK_FALSE(has_marker_above(separated_by_blank, 2, "safety:"));
}

TEST_CASE("has_marker_above: rejects a marker on the first line's own position") {
    const auto file = parse_source("a.cpp", "cast(); // safety: ok\n");
    CHECK_FALSE(has_marker_above(file, 0, "safety:"));
}

TEST_CASE("trim: removes surrounding whitespace only") {
    CHECK(trim("  a b \t") == "a b");
    CHECK(trim("   ").empty());
}
