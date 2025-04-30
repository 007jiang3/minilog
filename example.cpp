#include "minilog.hpp"

int main() {
    // Initialize the logger with a log file name, log level threshold, and whether to log
    // asynchronously. You must init the logger before any logging.
    MINILOG_INIT("mini.log"); // same as "MINILOG_INIT("mini.log", INFO, true)"

    // Enable logging to console(default is true)
    // and set the level threshold for console(default is INFO).
    // You should call them before any logging in multi-threads context
    // because they are not MT-safe.
    MINILOG_ENABLE_CONSOLE(true);
    MINILOG_SET_LEVEL_THRESHOLD(DEBUG);

    // Log levels:
    // - TRACE
    // - DEBUG
    // - INFO
    // - WARNING
    // - ERROR
    // - FATAL

    // Use MINILOG_LEVEL macros to log formatted messages with different log levels.
    // These callings are MT-safe.
    MINILOG_TRACE("this is a trace message");
    MINILOG_DEBUG("this is a debug message with '{}'", "DEBUG");
    MINILOG_INFO(
        "this is an info message at '{}'",
        std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()));
    MINILOG_WARNING("this is a warning message with two arguments: {:*>6}, {:.2f}", 1, 1.23456);
    MINILOG_ERROR("this is an error message: {}", true);
    MINILOG_FATAL("this is a fatal message: {}", "Stack Overflow");

    // // Shutdown the logger manually. The destructor will also shutdown the logger automatically.
    // // The calling is not MT-safe.
    // MINILOG_SHUTDOWN();

    return 0;
}