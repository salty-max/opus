#pragma once

#include <opus/core/log.hpp>

#include <string>
#include <utility>
#include <vector>

namespace opus::test {

/// A log record captured by RecordingSink.
struct LogRecord {
    LogLevel level;      ///< Level the record was written at.
    std::string message; ///< Fully formatted message.

    friend bool operator==(const LogRecord&, const LogRecord&) = default;
};

/// Collects every record a Logger forwards, for assertions.
class RecordingSink {
public:
    /// A LogSink that appends into this recorder; the recorder must outlive it.
    [[nodiscard]] LogSink sink() {
        return [this](LogLevel level, std::string_view message) {
            records_.push_back({.level = level, .message = std::string{message}});
        };
    }

    /// Every record received so far, in arrival order.
    [[nodiscard]] const std::vector<LogRecord>& records() const { return records_; }

private:
    std::vector<LogRecord> records_;
};

} // namespace opus::test
