#pragma once

#include <cstdint>
#include <expected>
#include <format>
#include <functional>
#include <string_view>
#include <utility>

namespace opus {

/// Severity of a log record, ordered from most to least verbose.
enum class LogLevel : std::uint8_t {
    Trace, ///< Step-by-step detail, off unless chasing a specific bug.
    Debug, ///< Diagnostic detail useful during development.
    Info,  ///< Normal lifecycle events.
    Warn,  ///< Something unexpected that the engine recovered from.
    Error, ///< An operation failed.
};

/// Canonical lowercase name of @p level, e.g. `"warn"`; `"unknown"` for a
/// value outside the enumeration.
[[nodiscard]] std::string_view to_string(LogLevel level);

/// Why a string could not be parsed as a LogLevel.
enum class ParseLogLevelError : std::uint8_t {
    Empty,   ///< The input was empty.
    Unknown, ///< The input names no level.
};

/// Parses a level from its canonical name, as returned by to_string.
///
/// @code
/// auto level = opus::parse_log_level("warn"); // LogLevel::Warn
/// @endcode
[[nodiscard]] std::expected<LogLevel, ParseLogLevelError> parse_log_level(std::string_view name);

/// Destination for records that passed the logger's level filter.
using LogSink = std::function<void(LogLevel level, std::string_view message)>;

/// Sink that writes `[level] message` lines to the process's stderr.
[[nodiscard]] LogSink stderr_sink();

/// Filters records by level and forwards the survivors to a sink.
///
/// A Logger is an ordinary value owned by whoever needs one; the engine
/// has no global logger.
///
/// @code
/// opus::Logger log{opus::stderr_sink(), opus::LogLevel::Debug};
/// log.info("loaded {} units", count);
/// @endcode
class Logger {
public:
    /// Creates a logger that forwards records at @p min_level or above to
    /// @p sink. An empty @p sink discards every record.
    explicit Logger(LogSink sink, LogLevel min_level = LogLevel::Info);

    /// Whether a record at @p level would reach a sink.
    [[nodiscard]] bool enabled(LogLevel level) const;

    /// Least severe level that still reaches the sink.
    [[nodiscard]] LogLevel min_level() const;

    /// Changes the least severe level that reaches the sink.
    void set_min_level(LogLevel level);

    /// Forwards an already-formatted @p message if @p level is enabled.
    void write(LogLevel level, std::string_view message) const;

    /// Formats and forwards a record; formatting is skipped when @p level is disabled.
    template <typename... Args>
    void log(LogLevel level, std::format_string<Args...> fmt, Args&&... args) const {
        if (enabled(level)) {
            sink_(level, std::format(fmt, std::forward<Args>(args)...));
        }
    }

    /// Logs at LogLevel::Trace.
    template <typename... Args> void trace(std::format_string<Args...> fmt, Args&&... args) const {
        log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
    }

    /// Logs at LogLevel::Debug.
    template <typename... Args> void debug(std::format_string<Args...> fmt, Args&&... args) const {
        log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
    }

    /// Logs at LogLevel::Info.
    template <typename... Args> void info(std::format_string<Args...> fmt, Args&&... args) const {
        log(LogLevel::Info, fmt, std::forward<Args>(args)...);
    }

    /// Logs at LogLevel::Warn.
    template <typename... Args> void warn(std::format_string<Args...> fmt, Args&&... args) const {
        log(LogLevel::Warn, fmt, std::forward<Args>(args)...);
    }

    /// Logs at LogLevel::Error.
    template <typename... Args> void error(std::format_string<Args...> fmt, Args&&... args) const {
        log(LogLevel::Error, fmt, std::forward<Args>(args)...);
    }

private:
    LogSink sink_;
    LogLevel min_level_;
};

} // namespace opus
