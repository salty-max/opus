#include "paths.hpp"

#include <doctest/doctest.h>

using namespace opus::lint;

TEST_CASE("is_engine: matches only the engine tree") {
    CHECK(is_engine("engine/src/core/log.cpp"));
    CHECK_FALSE(is_engine("tests/core/log.test.cpp"));
    CHECK_FALSE(is_engine("tools/engine/x.cpp"));
}

TEST_CASE("is_public_header: requires engine/include/opus and a header extension") {
    CHECK(is_public_header("engine/include/opus/core/log.hpp"));
    CHECK(is_public_header("engine/include/opus/version.hpp.in"));
    CHECK_FALSE(is_public_header("engine/src/core/internal.hpp"));
    CHECK_FALSE(is_public_header("engine/include/opus/readme.md"));
}

TEST_CASE("is_test_spec: matches the .test.cpp suffix anywhere") {
    CHECK(is_test_spec("tests/core/log.test.cpp"));
    CHECK(is_test_spec("tools/lint/source.test.cpp"));
    CHECK_FALSE(is_test_spec("tests/main.cpp"));
}

TEST_CASE("is_in_tests: matches only the tests tree") {
    CHECK(is_in_tests("tests/util.hpp"));
    CHECK_FALSE(is_in_tests("tools/lint/source.test.cpp"));
}

TEST_CASE("is_header: recognises every header extension") {
    CHECK(is_header("a.hpp"));
    CHECK(is_header("a.h"));
    CHECK(is_header("a.hpp.in"));
    CHECK_FALSE(is_header("a.cpp"));
}

TEST_CASE("is_deterministic_module: covers simulation modules in both engine roots") {
    CHECK(is_deterministic_module("engine/src/sim/tick.cpp"));
    CHECK(is_deterministic_module("engine/include/opus/ecs/registry.hpp"));
    CHECK(is_deterministic_module("engine/src/world/tilemap.cpp"));
    CHECK(is_deterministic_module("engine/src/rts/orders.cpp"));
    CHECK_FALSE(is_deterministic_module("engine/src/render/sprite_batch.cpp"));
    CHECK_FALSE(is_deterministic_module("tests/sim/tick.test.cpp"));
    CHECK_FALSE(is_deterministic_module("engine/src/simulation_ui/x.cpp"));
}

TEST_CASE("is_lintable: accepts C++ sources and headers only") {
    CHECK(is_lintable("engine/src/core/log.cpp"));
    CHECK(is_lintable("engine/include/opus/version.hpp.in"));
    CHECK_FALSE(is_lintable("CMakeLists.txt"));
    CHECK_FALSE(is_lintable("docs/plan.md"));
}
