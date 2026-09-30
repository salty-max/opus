#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <string>
#include <string_view>
#include <vector>

using opus::lint::check_test_names;
using opus::lint::test::findings;
using opus::lint::test::none;

namespace {

std::vector<std::string> flagged() {
    return {"1:test-name"};
}

std::vector<std::string> spec(std::string_view text) {
    return findings(check_test_names, "tests/core/log.test.cpp", text);
}

} // namespace

TEST_CASE("check_test_names: accepts symbol-colon-behavior names") {
    CHECK(spec(R"(TEST_CASE("Logger: drops disabled records") {)") == none);
    CHECK(spec(R"(TEST_CASE("Logger::write: forwards verbatim") {)") == none);
}

TEST_CASE("check_test_names: rejects names without a symbol prefix") {
    CHECK(spec(R"(TEST_CASE("logger works") {)") == flagged());
    CHECK(spec(R"(TEST_CASE("Logger:no space") {)") == flagged());
    CHECK(spec(R"(TEST_CASE("Logger: ") {)") == flagged());
}

TEST_CASE("check_test_names: rejects names not written on the TEST_CASE line") {
    CHECK(spec("TEST_CASE(\n") == flagged());
}

TEST_CASE("check_test_names: applies only to spec files") {
    CHECK(findings(check_test_names, "tests/util.hpp", R"(TEST_CASE("bad") {)") == none);
}
