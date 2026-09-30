#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <string>
#include <string_view>
#include <vector>

using opus::lint::check_raw_memory;
using opus::lint::test::findings;
using opus::lint::test::none;

namespace {

const std::vector<std::string> flagged{"1:raw-memory"};

std::vector<std::string> engine(std::string_view text) {
    return findings(check_raw_memory, "engine/src/a.cpp", text);
}

} // namespace

TEST_CASE("check_raw_memory: flags new, delete and the C allocators") {
    CHECK(engine("auto* p = new Unit{};") == flagged);
    CHECK(engine("auto* p = new (storage) Unit;") == flagged);
    CHECK(engine("delete p;") == flagged);
    CHECK(engine("delete[] items;") == flagged);
    CHECK(engine("void* p = malloc(64);") == flagged);
    CHECK(engine("free(p);") == flagged);
}

TEST_CASE("check_raw_memory: allows deleted functions, identifiers and member calls") {
    CHECK(engine("Unit(const Unit&) = delete;") == none);
    CHECK(engine("int renew = new_count;") == none);
    CHECK(engine("pool.free(handle);") == none);
    CHECK(engine("arena->free(block);") == none);
}

TEST_CASE("check_raw_memory: does not apply outside the engine") {
    CHECK(findings(check_raw_memory, "tests/a.test.cpp", "auto* p = new int;") == none);
}

TEST_CASE("check_raw_memory: flags new followed by a parenthesised type") {
    CHECK(engine("auto* p = new(Unit);") == flagged);
}
