#include "minilog.hpp"

int main(int argc, char const* argv[]) {
    minilog::log_trace("Hello, {}!", "world");
    minilog::log_debug("Hello, {}!", "world");
    minilog::log_info("Hello, {}!", "world");
    minilog::log_warning("Hello, {}!", "world");
    minilog::log_error("Hello, {}!", "world");
    minilog::log_fatal("Hello, {}!", "world");

    return 0;
}
