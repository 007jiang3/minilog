#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <source_location>
#include <stop_token>
#include <string>
#include <thread>

namespace minilog {

// Log level.
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

// Log message.
struct LogMessage {
    LogLevel level;
    std::string message;
    std::source_location location;
    std::chrono::system_clock::time_point time;

    // Default constructor.
    LogMessage() = default;

    // Constructor.
    LogMessage(LogLevel level, std::string message, std::source_location location)
        : level(level), message(std::move(message)), location(location), time(std::chrono::system_clock::now()) {}

    // Copy constructor.
    LogMessage(const LogMessage& other)
        : level(other.level), message(other.message), location(other.location), time(other.time) {}

    // Copy assignment operator.
    LogMessage& operator=(const LogMessage& other) {
        if (this != &other) {
            level = other.level;
            message = other.message;
            location = other.location;
            time = other.time;
        }
        return *this;
    }

    // Move constructor.
    LogMessage(LogMessage&& other) noexcept
        : level(other.level), message(std::move(other.message)), location(other.location), time(other.time) {}

    // Move assignment operator.
    LogMessage& operator=(LogMessage&& other) noexcept {
        if (this != &other) {
            level = other.level;
            message = std::move(other.message);
            location = other.location;
            time = other.time;
        }
        return *this;
    }
};

// Logger class.
class Logger {
public:
    // Get the instance of the logger.
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    // Initialize the logger.
    void initialize(const std::string& file_name, LogLevel level_threshold = LogLevel::INFO, bool async = false) {
        std::lock_guard lock(mutex_);
        if (initialized_) {
            throw std::runtime_error("Logger already initialized");
        }
        file_name_ = file_name;
        level_threshold_ = level_threshold;
        async_ = async;
#if !defined(NDEBUG)
        std::cout << "The log level threshold for console output: " << __log_level_to_string(level_threshold_) << '\n';
        std::cout << "Output to console: " << (enable_output_to_console_ ? "true" : "false") << '\n';
        std::cout << "Asynchronous logging: " << (async ? "true" : "false") << '\n';
#endif
        __open_log_file();
        if (async_) {
            thread_ = std::jthread(&Logger::__process_messages, this);
        }
        initialized_ = true;
#if !defined(NDEBUG)
        std::cout << "Logger has been initialized" << std::endl;
#endif
    }

    // Log a message with the specified log level and format string.
    template<typename... Args>
    void log(std::source_location location, LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
        std::lock_guard lock(mutex_);
        if (!initialized_) {
            throw std::runtime_error("Logger not initialized");
        }
        if (async_) {
            messages_.emplace(level, std::format(fmt, std::forward<Args>(args)...), location);
            cv_.notify_one();
        } else {
            LogMessage message(level, std::format(fmt, std::forward<Args>(args)...), location);
            __write_log_message(message);
        }
    }

    // Enable or disable output to the console.
    void enable_output_to_console(bool enable = true) {
        std::lock_guard lock(mutex_);
        enable_output_to_console_ = enable;
    }

    // Set the log level threshold for console output.
    void set_level_threshold(LogLevel level) {
        std::lock_guard lock(mutex_);
        level_threshold_ = level;
    }

    // Shutdown the logger.
    void shutdown() {
#if !defined(NDEBUG)
        std::cout << "Request to shutdown the logger" << std::endl;
#endif
        __shutdown();
#if !defined(NDEBUG)
        std::cout << "Logger has been shutdown" << std::endl;
#endif
    }

    // Destructor.
    ~Logger() {
#if !defined(NDEBUG)
        std::cout << "Logger destructor" << std::endl;
#endif
        __shutdown();
    }

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void __open_log_file() {
        file_.open(file_name_, std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
#if !defined(NDEBUG)
        std::cout << "Log file: " << file_name_ << std::endl;
#endif
    }

    void __process_messages(std::stop_token st) {
        while (!st.stop_requested()) {
            std::vector<LogMessage> messages;
            {
                std::unique_lock lock(mutex_);
                cv_.wait(lock, [this, st] { return !messages_.empty() || st.stop_requested(); });
                while (!messages_.empty()) {
                    messages.emplace_back(std::move(messages_.front()));
                    messages_.pop();
                }
            }
            for (const auto& message : messages) {
                __write_log_message(message);
            }
        }
    }

    void __write_log_message(const LogMessage& message) {
        std::string level = __log_level_to_string(message.level);
        auto now = std::chrono::zoned_time(std::chrono::current_zone(), message.time);
        if (enable_output_to_console_ && message.level >= level_threshold_) {
            std::cout << std::format("{:%Y/%m/%d %H:%M:%S} [{}] [{}:{}] {}\n", now, level, message.location.file_name(),
                                     message.location.line(), message.message);
        }
        file_ << std::format("{:%Y/%m/%d %H:%M:%S} [{}] [{}:{}] {}\n", now, level, message.location.file_name(),
                             message.location.line(), message.message);
        file_.flush();
#if !defined(NDEBUG)
        std::cout << "Message has been written to the log file" << std::endl;
#endif
    }

    std::string __log_level_to_string(LogLevel level) {
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

    void __shutdown() {
        if (async_ && thread_.joinable()) {
            thread_.request_stop();
            cv_.notify_one();
            thread_.join();

            std::unique_lock lock(mutex_);
            while (!messages_.empty()) {
                __write_log_message(messages_.front());
                messages_.pop();
            }
        }
        if (file_.is_open()) {
            file_.close();
#if !defined(NDEBUG)
            std::cout << "Log file has been closed" << std::endl;
#endif
        }
        initialized_ = false;
    }

    std::ofstream file_;
    std::string file_name_;
    bool async_ = false;
    bool initialized_ = false;
    std::queue<LogMessage> messages_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::jthread thread_;
    LogLevel level_threshold_ = LogLevel::INFO; // Default log level threshold for console output.
    bool enable_output_to_console_ = true;
};

#define LOG_TRACE(...) Logger::instance().log(std::source_location::current(), LogLevel::TRACE, __VA_ARGS__)
#define LOG_DEBUG(...) Logger::instance().log(std::source_location::current(), LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...) Logger::instance().log(std::source_location::current(), LogLevel::INFO, __VA_ARGS__)
#define LOG_WARNING(...) Logger::instance().log(std::source_location::current(), LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Logger::instance().log(std::source_location::current(), LogLevel::ERROR, __VA_ARGS__)
#define LOG_FATAL(...) Logger::instance().log(std::source_location::current(), LogLevel::FATAL, __VA_ARGS__)

} // namespace minilog