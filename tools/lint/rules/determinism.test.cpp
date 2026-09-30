#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <string>
#include <string_view>
#include <vector>

using opus::lint::check_determinism;
using opus::lint::test::findings;
using opus::lint::test::none;

namespace {

const std::vector<std::string> flagged{"1:determinism"};

std::vector<std::string> sim(std::string_view text) {
    return findings(check_determinism, "engine/src/sim/tick.cpp", text);
}

} // namespace

TEST_CASE("check_determinism: flags floating point and libm") {
    CHECK(sim("float speed = 1;") == flagged);
    CHECK(sim("double d;") == flagged);
    CHECK(sim("auto r = std::sqrt(x);") == flagged);
    CHECK(sim("#include <cmath>") == flagged);
}

TEST_CASE("check_determinism: flags unordered containers") {
    CHECK(sim("std::unordered_map<int, int> m;") == flagged);
    CHECK(sim("#include <unordered_set>") == flagged);
}

TEST_CASE("check_determinism: flags clocks and platform RNGs") {
    CHECK(sim("auto t = std::chrono::steady_clock::now();") == flagged);
    CHECK(sim("auto t = time(nullptr);") == flagged);
    CHECK(sim("int r = rand();") == flagged);
    CHECK(sim("std::mt19937 gen{seed};") == flagged);
    CHECK(sim("std::uniform_int_distribution<int> d{0, 9};") == flagged);
}

TEST_CASE("check_determinism: flags unstable sorts") {
    CHECK(sim("std::sort(v.begin(), v.end());") == flagged);
    CHECK(sim("std::ranges::sort(v);") == flagged);
    CHECK(sim("std::ranges::stable_sort(v);") == none);
}

TEST_CASE("check_determinism: allows lookalike identifiers and comments") {
    CHECK(sim("int floaty = tick.time();") == none);
    CHECK(sim("auto r = rng.rand(); // no float here") == none);
}

TEST_CASE("check_determinism: covers every simulation module and nothing else") {
    CHECK(findings(check_determinism, "engine/include/opus/ecs/registry.hpp", "float f;") == flagged);
    CHECK(findings(check_determinism, "engine/src/world/map.cpp", "float f;") == flagged);
    CHECK(findings(check_determinism, "engine/src/rts/orders.cpp", "float f;") == flagged);
    CHECK(findings(check_determinism, "engine/src/render/camera.cpp", "float f;") == none);
}

TEST_CASE("check_determinism: flags std-qualified C clocks and RNGs") {
    CHECK(sim("auto t = std::time(nullptr);") == flagged);
    CHECK(sim("auto c = std::clock();") == flagged);
    CHECK(sim("int r = std::rand();") == flagged);
    CHECK(sim("std::srand(seed);") == flagged);
}
