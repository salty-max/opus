#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <vector>

using opus::lint::lint_file;
using opus::lint::test::findings;
using opus::lint::test::none;

TEST_CASE("lint_file: aggregates findings from every rule in line order") {
    const auto found = findings(lint_file, "engine/src/sim/tick.cpp",
                                "int* p = new int; // was a bug\n"
                                "double d = 0;\n");
    CHECK(found == std::vector<std::string>{"1:comment-content", "1:raw-memory", "2:determinism"});
}

TEST_CASE("lint_file: allow-strict silences the line directly below") {
    const auto found = findings(lint_file, "engine/src/core/x.cpp",
                                "// allow-strict: sink is the sanctioned output path\n"
                                "std::fprintf(stderr, \"x\");\n");
    CHECK(found == none);
}

TEST_CASE("lint_file: allow-strict without a reason silences nothing") {
    const auto found = findings(lint_file, "engine/src/core/x.cpp",
                                "// allow-strict:\n"
                                "std::fprintf(stderr, \"x\");\n");
    CHECK(found == std::vector<std::string>{"1:suppression", "2:engine-io"});
}

TEST_CASE("lint_file: allow-strict does not reach past the next statement") {
    const auto found = findings(lint_file, "engine/src/core/x.cpp",
                                "// allow-strict: first only\n"
                                "std::fprintf(stderr, \"x\");\n"
                                "std::fprintf(stderr, \"y\");\n");
    CHECK(found == std::vector<std::string>{"3:engine-io"});
}
