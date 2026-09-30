#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <string>
#include <vector>

using opus::lint::check_includes;
using opus::lint::test::findings;
using opus::lint::test::none;

namespace {

const std::vector<std::string> flagged{"1:include-path"};

} // namespace

TEST_CASE("check_includes: rejects parent-relative includes everywhere") {
    CHECK(findings(check_includes, "engine/src/core/a.cpp", R"(#include "../render/b.hpp")") == flagged);
    CHECK(findings(check_includes, "tools/lint/a.cpp", R"(#include "../../engine/x.hpp")") == flagged);
}

TEST_CASE("check_includes: public headers use angle includes only") {
    CHECK(findings(check_includes, "engine/include/opus/core/a.hpp", R"(#include "b.hpp")") == flagged);
    CHECK(findings(check_includes, "engine/include/opus/core/a.hpp", "#include <opus/core/b.hpp>") == none);
}

TEST_CASE("check_includes: tests include only util.hpp by quotes") {
    CHECK(findings(check_includes, "tests/core/log.test.cpp", R"(#include "util.hpp")") == none);
    CHECK(findings(check_includes, "tests/core/log.test.cpp", R"(#include "core/internal.hpp")") == flagged);
}

TEST_CASE("check_includes: engine sources may quote private headers") {
    CHECK(findings(check_includes, "engine/src/render/a.cpp", R"(#include "render/internal.hpp")") == none);
}
