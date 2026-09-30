#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <string>
#include <string_view>
#include <vector>

using opus::lint::check_suppressions;
using opus::lint::test::findings;
using opus::lint::test::none;

namespace {

std::vector<std::string> check(std::string_view text) {
    return findings(check_suppressions, "engine/src/a.cpp", text);
}

} // namespace

// clang-tidy reads suppression directives even inside string literals, so the
// fixtures below split the keyword to stay inert.
TEST_CASE("check_suppressions: a clang-tidy suppression needs checks and a reason") {
    CHECK(check("f(); // NO"
                "LINT") == std::vector<std::string>{"1:suppression"});
    CHECK(check("// NO"
                "LINTNEXTLINE(bugprone-x)") == std::vector<std::string>{"1:suppression"});
    CHECK(check("// NO"
                "LINTNEXTLINE(bugprone-x) SDL owns this pointer") == none);
    CHECK(check("// NO"
                "LINTEND(bugprone-x)") == none);
}

TEST_CASE("check_suppressions: allow-strict needs a reason") {
    CHECK(check("// allow-strict:") == std::vector<std::string>{"1:suppression"});
    CHECK(check("// allow-strict: the only sanctioned console write") == none);
}

TEST_CASE("check_suppressions: flags warning-silencing pragmas") {
    CHECK(check("#pragma clang diagnostic ignored \"-Wshadow\"") ==
          std::vector<std::string>{"1:warning-pragma"});
    CHECK(check("#pragma GCC diagnostic ignored \"-Wshadow\"") ==
          std::vector<std::string>{"1:warning-pragma"});
    CHECK(check("#pragma warning(disable: 4100)") == std::vector<std::string>{"1:warning-pragma"});
    CHECK(check("#pragma clang diagnostic push") == none);
}
