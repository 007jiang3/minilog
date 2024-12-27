#pragma once

#include <iostream>
#include <fstream>
#include <format>
#include <source_location>
#include <chrono>

#define NS_MINILOG_BEGIN namespace minilog {
#define NS_MINILOG_END }

NS_MINILOG_BEGIN

#define MINILOG_FOREACH_LOG_LEVEL(f) \
    f(trace) \
    f(debug) \
    f(info) \
    f(warning) \
    f(error) \
    f(fatal)

enum class log_level : uint8_t {
#define _ENUMERATE_LOG_LEVEL(level) level,
    MINILOG_FOREACH_LOG_LEVEL(_ENUMERATE_LOG_LEVEL)
#undef _ENUMERATE_LOG_LEVEL
};

namespace details {
    inline std::string to_string(log_level level) {
        switch (level) {
#define _CASE_LOG_LEVEL(level) case log_level::level: return #level;
            MINILOG_FOREACH_LOG_LEVEL(_CASE_LOG_LEVEL)
#undef _CASE_LOG_LEVEL
        }
        return "unknown";
    }

    inline log_level g_log_level_threshold = []() -> log_level {
        if (auto level = std::getenv("MINILOG_LEVEL"); level != nullptr) {
            if (std::string_view(level) == "trace") return log_level::trace;
            if (std::string_view(level) == "debug") return log_level::debug;
            if (std::string_view(level) == "info") return log_level::info;
            if (std::string_view(level) == "warning") return log_level::warning;
            if (std::string_view(level) == "error") return log_level::error;
            if (std::string_view(level) == "fatal") return log_level::fatal;
        }
        return log_level::info;
        }();

    inline std::ofstream g_log_file = []() -> std::ofstream {
        if (auto filename = std::getenv("MINILOG_FILE"); filename != nullptr) {
            return std::ofstream(filename, std::ios::app);
        }
        return std::ofstream();
        }();

    inline void output_log(log_level level, std::string msg, std::source_location location) {
        std::chrono::zoned_time now(std::chrono::current_zone(), std::chrono::high_resolution_clock::now());
        msg = std::format("{} {}:{} [{}] {}", now, location.file_name(), location.line(),
            to_string(level), msg);
        if (level >= g_log_level_threshold) {
            std::cout << msg + '\n';
        }
        if (g_log_file.is_open()) {
            g_log_file << msg + '\n';
        }
    }

    template <typename... Args>
    inline void log_with_source_location(log_level level, std::source_location location, std::format_string<Args...> fmt, Args&&... args) {
        output_log(level, std::format(fmt, std::forward<Args>(args)...), location);
    }
}

inline void set_log_level_threshold(log_level level) {
    details::g_log_level_threshold = level;
}

inline void set_log_file(const std::string& filename) {
    details::g_log_file.open(filename, std::ios::app);
}

template <typename... Args>
void log(log_level level, std::format_string<Args...> fmt, Args&&... args) {
    details::log_with_source_location(level, std::source_location::current(), fmt, std::forward<Args>(args)...);
}

#define _LOG_LEVEL(level) \
    template <typename... Args> \
    void log_##level(std::format_string<Args...> fmt, Args&&... args) { \
        log(log_level::level, fmt, std::forward<Args>(args)...); \
    }
MINILOG_FOREACH_LOG_LEVEL(_LOG_LEVEL)
#undef _LOG_LEVEL

#define MINILOG_P(x) log_debug(#x " = {}", x)

NS_MINILOG_END