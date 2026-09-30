#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <string>
#include <string_view>
#include <vector>

using opus::lint::check_comments;
using opus::lint::test::findings;
using opus::lint::test::none;

namespace {

std::vector<std::string> comment(std::string_view text) {
    return findings(check_comments, "engine/src/a.cpp", text);
}

std::vector<std::string> flagged() {
    return {"1:comment-content"};
}

} // namespace

TEST_CASE("check_comments: flags issue references") {
    CHECK(comment("// see #42") == flagged());
    CHECK(comment("/* closes #7 */") == flagged());
}

TEST_CASE("check_comments: ignores hash signs that are not issue references") {
    CHECK(comment("// the #include below pulls in <span>") == none);
    CHECK(comment("// HTML entity &#38; is not an issue") == none);
    CHECK(comment("#define X 1") == none);
}

TEST_CASE("check_comments: flags version, milestone and roadmap markers") {
    CHECK(comment("// added in v0.3") == flagged());
    CHECK(comment("// lands with M5") == flagged());
    CHECK(comment("// Phase 2 work") == flagged());
    CHECK(comment("// Roadmap: networking") == flagged());
}

TEST_CASE("check_comments: flags change-history narration") {
    CHECK(comment("// previously this leaked") == flagged());
    CHECK(comment("// this used to return null") == flagged());
    CHECK(comment("// Regression: order mattered") == flagged());
    CHECK(comment("// now fixed for empty input") == flagged());
}

TEST_CASE("check_comments: allows 'used to' meaning 'employed to'") {
    CHECK(comment("// scratch buffer used to hold vertices") == none);
}

TEST_CASE("check_comments: flags AI attribution in any case") {
    CHECK(comment("// Written by Claude") == flagged());
    CHECK(comment("// Generated with some tool") == flagged());
    CHECK(comment("// Co-Authored-By: someone") == flagged());
    CHECK(comment("// AI-generated table") == flagged());
}

TEST_CASE("check_comments: ignores forbidden words outside comments") {
    CHECK(comment(R"(auto s = "see #42 previously v1.0";)") == none);
}
