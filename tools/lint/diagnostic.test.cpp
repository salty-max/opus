#include "diagnostic.hpp"

#include <doctest/doctest.h>

TEST_CASE("format: renders path, line, rule and message") {
    const opus::lint::Diagnostic diagnostic{
        .path = "engine/src/a.cpp", .line = 12, .rule = "cast-safety", .message = "needs a reason"};
    CHECK(opus::lint::format(diagnostic) == "engine/src/a.cpp:12: error[cast-safety]: needs a reason");
}
