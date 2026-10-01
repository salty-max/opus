#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <string>
#include <vector>

using opus::lint::check_includes;
using opus::lint::test::findings;
using opus::lint::test::none;

namespace {

std::vector<std::string> flagged() {
    return {"1:include-path"};
}

} // namespace

TEST_CASE("check_includes: rejects parent-relative includes everywhere") {
    CHECK(findings(check_includes, "engine/src/core/a.cpp", R"(#include "../render/b.hpp")") == flagged());
    CHECK(findings(check_includes, "tools/lint/a.cpp", R"(#include "../../engine/x.hpp")") == flagged());
}

TEST_CASE("check_includes: public headers use angle includes only") {
    CHECK(findings(check_includes, "engine/include/opus/core/a.hpp", R"(#include "b.hpp")") == flagged());
    CHECK(findings(check_includes, "engine/include/opus/core/a.hpp", "#include <opus/core/b.hpp>") == none);
}

TEST_CASE("check_includes: tests include only util.hpp by quotes") {
    CHECK(findings(check_includes, "tests/core/log.test.cpp", R"(#include "util.hpp")") == none);
    CHECK(findings(check_includes, "tests/core/log.test.cpp", R"(#include "core/internal.hpp")") ==
          flagged());
}

TEST_CASE("check_includes: engine sources may quote private headers") {
    CHECK(findings(check_includes, "engine/src/render/a.cpp", R"(#include "render/internal.hpp")") == none);
}

TEST_CASE("check_includes: only engine sources and platform specs include SDL") {
    CHECK(findings(check_includes, "engine/include/opus/platform/window.hpp", "#include <SDL3/SDL.h>") ==
          flagged());
    CHECK(findings(check_includes, "engine/include/opus/platform/window.hpp", R"(#include "SDL3/SDL.h")") ==
          flagged());
    CHECK(findings(check_includes, "sandbox/main.cpp", "#include <SDL3/SDL.h>") == flagged());
    CHECK(findings(check_includes, "tests/core/log.test.cpp", "#include <SDL3/SDL.h>") == flagged());
    CHECK(findings(check_includes, "engine/src/platform/window.cpp", "#include <SDL3/SDL.h>") == none);
    CHECK(findings(check_includes, "tests/platform/window.test.cpp", "#include <SDL3/SDL_events.h>") == none);
}

TEST_CASE("check_includes: specs may include their own module's util.hpp") {
    CHECK(findings(check_includes, "tests/platform/window.test.cpp", R"(#include "platform/util.hpp")") ==
          none);
    CHECK(findings(check_includes, "tests/platform/util.hpp", R"(#include "util.hpp")") == none);
    CHECK(findings(check_includes, "tests/core/log.test.cpp", R"(#include "platform/util.hpp")") ==
          flagged());
    CHECK(findings(check_includes, "tests/opus.test.cpp", R"(#include "/util.hpp")") == flagged());
}
