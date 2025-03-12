# Minilog

Minilog is a minimalistic logging library for C++ applications.

The project requires C++20 or later.

## Usage

### Minilog v1

The log level threshold can be set using the `MINILOG_LEVEL` environment variable. Possible values are `trace`, `debug`, `info`, `warning`, `error`, and `fatal`.

The log file can be specified using the `MINILOG_FILE` environment variable. Set `MINILOG_FILE` to the desired log file path.

```cpp
#include "minilog.hpp"

int main(int argc, char const* argv[]) {

    // this is the default log level
    // minilog::set_log_level_threshold(minilog::log_level::info);

    // use `log_#level(fmt, args...)` functions
    minilog::log_trace("This is a {} message", "trace");
    minilog::log_debug("This is a {} message", "debug");
    minilog::log_info("This is a {} message", "info");
    minilog::log_warning("This is a {} message", "warning");
    minilog::log_error("This is a {} message", "error");
    minilog::log_fatal("This is a {} message", "fatal");

    // you can also use `log(level, fmt, args...)` directly
    // minilog::log(minilog::log_level::trace, "This is a {} message", "trace");
    // ...

    // set log level threshold
    minilog::set_log_level_threshold(minilog::log_level::debug);

    // set log file, you can see all levels in the file
    minilog::set_log_file("mini.log");

    minilog::log_trace("This is a {} message", "trace");
    minilog::log_debug("This is a {} message", "debug");
    minilog::log_info("This is a {} message", "info");
    minilog::log_warning("This is a {} message", "warning");
    minilog::log_error("This is a {} message", "error");
    minilog::log_fatal("This is a {} message", "fatal");

    return 0;
}
```

### Minilog v2

```cpp
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
```