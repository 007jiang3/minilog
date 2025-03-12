#include <minilog_v2.hpp>

using namespace minilog;

int main() {
    // Get the instance of the logger first.
    auto& logger = Logger::instance();

    // Initialize the logger with a log file name, log level threshold, and whether to log asynchronously.
    logger.initialize("test2.log", LogLevel::INFO, true);

    // // Enable output to the console. Default is enabled.
    // logger.enable_output_to_console(true);

    // // Set the log level threshold for console output. Default is INFO.
    // logger.set_level_threshold(LogLevel::INFO);

    // Log formatted messages with different log levels.
    LOG_TRACE("This is a trace message");
    LOG_DEBUG("This is a debug message with an integer: {}", 42);
    LOG_INFO("This is an info message with a floating-point number: {}", 3.14);
    LOG_WARNING("This is a warning message with a string: {}", "hello");
    LOG_ERROR("This is an error message with a character: {}", 'X');
    LOG_FATAL("This is a fatal message with a boolean: {}", true);

    // // Shutdown the logger manually. The destructor will also shutdown the logger automatically.
    // logger.shutdown();

    return 0;
}