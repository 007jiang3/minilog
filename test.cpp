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
