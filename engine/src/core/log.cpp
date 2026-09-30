#include <opus/core/log.hpp>

#include <array>
#include <cstdio>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace opus {

namespace {

constexpr std::array level_names{
    std::pair{LogLevel::Trace, std::string_view{"trace"}},
    std::pair{LogLevel::Debug, std::string_view{"debug"}},
    std::pair{LogLevel::Info, std::string_view{"info"}},
    std::pair{LogLevel::Warn, std::string_view{"warn"}},
    std::pair{LogLevel::Error, std::string_view{"error"}},
};

} // namespace

std::string_view to_string(LogLevel level) {
    for (const auto& [candidate, name] : level_names) {
        if (candidate == level) {
            return name;
        }
    }
    return "unknown";
}

std::expected<LogLevel, ParseLogLevelError> parse_log_level(std::string_view name) {
    if (name.empty()) {
        return std::unexpected{ParseLogLevelError::Empty};
    }
    for (const auto& [level, candidate] : level_names) {
        if (candidate == name) {
            return level;
        }
    }
    return std::unexpected{ParseLogLevelError::Unknown};
}

LogSink stderr_sink() {
    return [](LogLevel level, std::string_view message) {
        const std::string line = std::format("[{}] {}\n", to_string(level), message);
        // allow-strict: this sink is the engine's one sanctioned path to the terminal; a
        // failed write has nowhere left to be reported, so its result is discarded
        static_cast<void>(std::fwrite(line.data(), 1, line.size(), stderr));
    };
}

Logger::Logger(LogSink sink, LogLevel min_level) : sink_{std::move(sink)}, min_level_{min_level} {
}

bool Logger::enabled(LogLevel level) const {
    return sink_ && level >= min_level_;
}

LogLevel Logger::min_level() const {
    return min_level_;
}

void Logger::set_min_level(LogLevel level) {
    min_level_ = level;
}

void Logger::write(LogLevel level, std::string_view message) const {
    if (enabled(level)) {
        sink_(level, message);
    }
}

} // namespace opus
