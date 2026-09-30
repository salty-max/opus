#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <vector>

using opus::lint::check_casts;
using opus::lint::test::findings;
using opus::lint::test::none;

TEST_CASE("check_casts: flags every type-punning cast without a safety comment") {
    CHECK(findings(check_casts, "engine/src/a.cpp", "auto* p = reinterpret_cast<T*>(q);") ==
          std::vector<std::string>{"1:cast-safety"});
    CHECK(findings(check_casts, "tools/x.cpp", "auto& r = const_cast<T&>(c);") ==
          std::vector<std::string>{"1:cast-safety"});
    CHECK(findings(check_casts, "engine/src/a.cpp", "auto u = std::bit_cast<std::uint32_t>(f);") ==
          std::vector<std::string>{"1:cast-safety"});
    CHECK(findings(check_casts, "engine/src/a.cpp", "auto* p = std::launder(q);") ==
          std::vector<std::string>{"1:cast-safety"});
}

TEST_CASE("check_casts: accepts a cast justified directly above") {
    CHECK(findings(check_casts, "engine/src/a.cpp",
                   "// safety: storage is aligned for T by the arena\n"
                   "auto* p = reinterpret_cast<T*>(q);\n") == none);
}

TEST_CASE("check_casts: ignores static_cast and casts named in comments") {
    CHECK(findings(check_casts, "engine/src/a.cpp",
                   "auto n = static_cast<int>(x); // not reinterpret_cast") == none);
}
