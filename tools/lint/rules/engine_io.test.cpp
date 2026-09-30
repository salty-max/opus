#include "rules.hpp"
#include "test_util.hpp"

#include <doctest/doctest.h>

#include <string>
#include <vector>

using opus::lint::check_engine_io;
using opus::lint::test::findings;
using opus::lint::test::none;

namespace {

std::vector<std::string> flagged() {
    return {"1:engine-io"};
}

} // namespace

TEST_CASE("check_engine_io: flags console output in the engine") {
    CHECK(findings(check_engine_io, "engine/src/a.cpp", R"(std::cout << "x";)") == flagged());
    CHECK(findings(check_engine_io, "engine/src/a.cpp", R"(std::println("x");)") == flagged());
    CHECK(findings(check_engine_io, "engine/src/a.cpp", R"(printf("x");)") == flagged());
    CHECK(findings(check_engine_io, "engine/src/a.cpp", R"(std::fprintf(stderr, "x");)") == flagged());
    CHECK(findings(check_engine_io, "engine/src/a.cpp", "#include <iostream>") == flagged());
}

TEST_CASE("check_engine_io: allows formatting into buffers and member functions named print") {
    CHECK(findings(check_engine_io, "engine/src/a.cpp", R"(std::snprintf(buf, n, "x");)") == none);
    CHECK(findings(check_engine_io, "engine/src/a.cpp", "widget.printf(x);") == none);
}

TEST_CASE("check_engine_io: does not apply outside the engine") {
    CHECK(findings(check_engine_io, "sandbox/main.cpp", R"(std::println("x");)") == none);
    CHECK(findings(check_engine_io, "tools/lint/main.cpp", R"(std::fprintf(stderr, "x");)") == none);
}

TEST_CASE("check_engine_io: flags std-qualified C output functions") {
    CHECK(findings(check_engine_io, "engine/src/a.cpp", R"(std::puts("x");)") == flagged());
    CHECK(findings(check_engine_io, "engine/src/a.cpp", R"(std::printf("x");)") == flagged());
}

TEST_CASE("check_engine_io: flags any use of the console streams") {
    CHECK(findings(check_engine_io, "engine/src/a.cpp", "std::fwrite(p, 1, n, stderr);") == flagged());
    CHECK(findings(check_engine_io, "engine/src/a.cpp", "std::fflush(stdout);") == flagged());
    CHECK(findings(check_engine_io, "engine/src/a.cpp", "std::fwrite(p, 1, n, file);") == none);
}
