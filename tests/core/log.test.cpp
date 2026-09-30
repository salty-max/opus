#include <opus/core/log.hpp>

#include "util.hpp"

#include <doctest/doctest.h>

#include <array>
#include <vector>

using opus::Logger;
using opus::LogLevel;
using opus::ParseLogLevelError;
using opus::test::LogRecord;
using opus::test::RecordingSink;

namespace {

constexpr std::array all_levels{LogLevel::Trace, LogLevel::Debug, LogLevel::Info, LogLevel::Warn,
                                LogLevel::Error};

} // namespace

TEST_CASE("to_string: names every level") {
    CHECK(opus::to_string(LogLevel::Trace) == "trace");
    CHECK(opus::to_string(LogLevel::Debug) == "debug");
    CHECK(opus::to_string(LogLevel::Info) == "info");
    CHECK(opus::to_string(LogLevel::Warn) == "warn");
    CHECK(opus::to_string(LogLevel::Error) == "error");
}

TEST_CASE("parse_log_level: round-trips every canonical name") {
    for (const LogLevel level : all_levels) {
        CHECK(opus::parse_log_level(opus::to_string(level)) == level);
    }
}

TEST_CASE("parse_log_level: rejects empty input") {
    CHECK(opus::parse_log_level("").error() == ParseLogLevelError::Empty);
}

TEST_CASE("parse_log_level: rejects unknown and non-canonical names") {
    CHECK(opus::parse_log_level("verbose").error() == ParseLogLevelError::Unknown);
    CHECK(opus::parse_log_level("WARN").error() == ParseLogLevelError::Unknown);
    CHECK(opus::parse_log_level(" warn").error() == ParseLogLevelError::Unknown);
}

TEST_CASE("Logger: forwards records at or above the minimum level") {
    RecordingSink recorder;
    const Logger log{recorder.sink(), LogLevel::Warn};

    log.info("dropped");
    log.warn("kept {}", 1);
    log.error("kept {}", 2);

    CHECK(recorder.records() ==
          std::vector<LogRecord>{{LogLevel::Warn, "kept 1"}, {LogLevel::Error, "kept 2"}});
}

TEST_CASE("Logger: defaults the minimum level to info") {
    const Logger log{opus::LogSink{}};
    CHECK(log.min_level() == LogLevel::Info);
}

TEST_CASE("Logger: set_min_level changes what reaches the sink") {
    RecordingSink recorder;
    Logger log{recorder.sink(), LogLevel::Error};

    log.debug("dropped");
    log.set_min_level(LogLevel::Trace);
    log.trace("kept");

    CHECK(recorder.records() == std::vector<LogRecord>{{LogLevel::Trace, "kept"}});
}

TEST_CASE("Logger: write forwards pre-formatted text verbatim") {
    RecordingSink recorder;
    const Logger log{recorder.sink(), LogLevel::Trace};

    log.write(LogLevel::Info, "{not a format}");

    CHECK(recorder.records() == std::vector<LogRecord>{{LogLevel::Info, "{not a format}"}});
}

TEST_CASE("Logger: an empty sink discards every record") {
    const Logger log{opus::LogSink{}, LogLevel::Trace};
    for (const LogLevel level : all_levels) {
        CHECK_FALSE(log.enabled(level));
    }
    log.error("must not crash");
}
