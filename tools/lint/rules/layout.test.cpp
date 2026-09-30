#include "rules.hpp"

#include <doctest/doctest.h>

#include <format>
#include <string>
#include <vector>

using opus::lint::check_layout;

namespace {

std::vector<std::string> layout(std::vector<std::string> paths) {
    std::vector<std::string> out;
    for (const auto& diagnostic : check_layout(paths)) {
        out.push_back(std::format("{}:{}", diagnostic.path, diagnostic.rule));
    }
    return out;
}

} // namespace

TEST_CASE("check_layout: accepts a header and source sharing one spec") {
    CHECK(layout({"engine/include/opus/core/log.hpp", "engine/src/core/log.cpp", "tests/core/log.test.cpp"})
              .empty());
}

TEST_CASE("check_layout: flags an engine file without a spec") {
    CHECK(layout({"engine/src/core/log.cpp"}) == std::vector<std::string>{"engine/src/core/log.cpp:mirror"});
    CHECK(layout({"engine/include/opus/sim/tick.hpp"}) ==
          std::vector<std::string>{"engine/include/opus/sim/tick.hpp:mirror"});
}

TEST_CASE("check_layout: flags a spec that mirrors nothing") {
    CHECK(layout({"tests/core/gone.test.cpp"}) ==
          std::vector<std::string>{"tests/core/gone.test.cpp:mirror"});
}

TEST_CASE("check_layout: exempts the barrel, internal files and test helpers") {
    CHECK(layout({"engine/include/opus/opus.hpp", "tests/opus.test.cpp", "engine/src/render/internal.hpp",
                  "engine/src/render/internal.cpp", "tests/util.hpp", "tests/main.cpp"})
              .empty());
}

TEST_CASE("check_layout: private engine headers must be internal") {
    CHECK(layout({"engine/src/render/batch.hpp"}) ==
          std::vector<std::string>{"engine/src/render/batch.hpp:layout"});
    CHECK(layout({"engine/src/render/batch_internal.hpp"}).empty());
}
