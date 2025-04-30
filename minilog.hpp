#pragma once

#include <chrono>
#include <format>
#include <queue>
#include <source_location>
#include <string>
#include <string_view>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <fstream>

namespace minilog {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

struct LogMessage {
    LogLevel level;
    std::string message;
    std::source_location location;
    std::chrono::system_clock::time_point time;
};

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    struct Options {
        bool async = true;
        bool console = true;
        LogLevel threshold = LogLevel::INFO;
        std::string file;
    };

    // MT-unsafe
    void init(const Options& opts) {
        if (writer_.is_open()) {
            throw std::runtime_error("logger has already been initialized");
        }

        async_ = opts.async;
        console_ = opts.console;
        threshold_ = opts.threshold;
        file_ = opts.file;

        __open_log_file();

        if (async_) {
            worker_ = std::jthread(&Logger::worker_, this);
        }
    }

    // MT-unsafe
    void init(std::string_view file, bool async = true) {
        Options opts{
            .async{async},
            .console{true},
            .threshold{LogLevel::INFO},
            .file{file},
        };
        init(opts);
    }

    // MT-safe
    template<class... Args>
    void log(std::source_location loc, LogLevel level, std::format_string<Args...> fmt,
             Args&&... args) {
        if (!writer_.is_open()) {
            throw std::runtime_error("logger has not been initialized.");
        }
        if (async_) {
            LogMessage msg{level, std::format(fmt, std::forward<Args>(args)...), loc,
                           std::chrono::system_clock::now()};
            std::lock_guard lock(mtx_);
            messages_.emplace(std::move(msg));
            cv_.notify_one();
        } else {
            std::lock_guard lock(mtx_);
            __write({level, std::format(fmt, std::forward<Args>(args)...), loc,
                     std::chrono::system_clock::now()});
        }
    }

    // MT-unsafe
    void set_level_threshold(LogLevel level) noexcept { threshold_ = level; }

    // MT-unsafe
    void enable_output_to_console(bool enable = true) noexcept { console_ = enable; }

    // MT-unsafe
    void shutdown() { __shutdown(); }

    ~Logger() { __shutdown(); }

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

    void __open_log_file() {
        writer_.open(file_, std::ios::app);
        if (!writer_.is_open()) {
            std::runtime_error("failed to open log file");
        }
    }

    void __worker(std::stop_token st) {
        while (!st.stop_requested()) {
            std::vector<LogMessage> messages;
            {
                std::unique_lock lock(mtx_);
                cv_.wait(lock, [this, st] { return !messages_.empty() || st.stop_requested(); });
                while (!messages_.empty()) {
                    lock.unlock();
                    lock.lock();
                    messages.emplace_back(std::move(messages_.front()));
                    messages_.pop();
                }
            }
            for (auto& msg : messages) {
                __write(msg);
            }
        }
    }

    void __write(const LogMessage& msg) {
        std::string level = __log_level_to_string(msg.level);
        auto now = std::chrono::zoned_time(std::chrono::current_zone(), msg.time);
        std::string formated(std::format("{:%Y-%m-%d %H:%M:%S} [{}] [{}:{}] {}\n", now, level,
                                         msg.location.file_name(), msg.location.line(),
                                         msg.message));
        __write_to_file(formated);
        if (console_ && msg.level >= threshold_) {
            __write_to_console(formated);
        }
    }

    void __write_to_console(std::string_view msg) { std::cout << msg << std::flush; }

    void __write_to_file(std::string_view msg) { writer_ << msg << std::flush; }

    void __shutdown() {
        if (async_ && worker_.joinable()) {
            worker_.request_stop();
            cv_.notify_one();
            worker_.join();
            while (!messages_.empty()) {
                __write(messages_.front());
                messages_.pop();
            }
        }
        if (writer_.is_open()) {
            writer_.close();
        }
    }

    static std::string __log_level_to_string(LogLevel level) {
        switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
        }
    }

    bool async_;
    bool console_;
    LogLevel threshold_;
    std::string file_;

    std::ofstream writer_;

    std::queue<LogMessage> messages_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::jthread worker_;
};

} // namespace minilog

#define MINILOG_INIT(...) minilog::Logger::instance().init(__VA_ARGS__)

#define MINILOG(level, ...)                                                                        \
    minilog::Logger::instance().log(std::source_location::current(), minilog::LogLevel::level,     \
                                    __VA_ARGS__)

#define MINILOG_TRACE(...) MINILOG(TRACE, __VA_ARGS__)
#define MINILOG_DEBUG(...) MINILOG(DEBUG, __VA_ARGS__)
#define MINILOG_INFO(...) MINILOG(INFO, __VA_ARGS__)
#define MINILOG_WARNING(...) MINILOG(WARNING, __VA_ARGS__)
#define MINILOG_ERROR(...) MINILOG(ERROR, __VA_ARGS__)
#define MINILOG_FATAL(...) MINILOG(FATAL, __VA_ARGS__)

#define MINILOG_SET_LEVEL_THRESHOLD(level)                                                         \
    minilog::Logger::instance().set_level_threshold(minilog::LogLevel::level)

#define MINILOG_ENABLE_CONSOLE(enable) minilog::Logger::instance().enable_output_to_console(enable)

#define MINILOG_SHUTDOWN() minilog::Logger::instance().shutdown()
