#include <opus/opus.hpp>

#include <doctest/doctest.h>

#include <format>

TEST_CASE("version_string: joins the numeric components") {
    CHECK(opus::version_string ==
          std::format("{}.{}.{}", opus::version_major, opus::version_minor, opus::version_patch));
}

TEST_CASE("opus.hpp: exposes the logger") {
    const opus::Logger log{opus::LogSink{}};
    CHECK_FALSE(log.enabled(opus::LogLevel::Error));
}
