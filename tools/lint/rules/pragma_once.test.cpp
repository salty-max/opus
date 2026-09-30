#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <vector>

using opus::lint::check_pragma_once;
using opus::lint::test::findings;
using opus::lint::test::none;

TEST_CASE("check_pragma_once: accepts a header with pragma once") {
    CHECK(findings(check_pragma_once, "engine/include/opus/a.hpp", "// doc\n#pragma once\n") == none);
}

TEST_CASE("check_pragma_once: flags a header without it") {
    CHECK(findings(check_pragma_once, "engine/include/opus/a.hpp", "#ifndef A\n#define A\n#endif\n") ==
          std::vector<std::string>{"0:pragma-once"});
}

TEST_CASE("check_pragma_once: ignores pragma once inside a comment") {
    CHECK(findings(check_pragma_once, "tests/util.hpp", "// #pragma once\n") ==
          std::vector<std::string>{"0:pragma-once"});
}

TEST_CASE("check_pragma_once: ignores source files") {
    CHECK(findings(check_pragma_once, "engine/src/a.cpp", "int x;\n") == none);
}
