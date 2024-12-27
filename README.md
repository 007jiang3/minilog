# Minilog

Minilog is a minimalistic logging library for C++ applications.

The project requires C++20 or later.

## Features

- Lightweight and easy to integrate
- Supports multiple log levels (INFO, DEBUG, ERROR, etc.)
- Thread-safe logging
- Customizable log format

## Installation

To use Minilog in your project, simply clone the repository and include the header files in your project:

```sh
git clone https://github.com/007jiang3/minilog.git
```

Include the necessary headers in your source files:

```cpp
#include "minilog.hpp"
```

## Usage

Here's a simple example of how to use Minilog:

```cpp
#include "minilog.hpp"

int main(int argc, char const* argv[]) {

    // this is the default log level (info)
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

The log level threshold can be set using the `MINILOG_LEVEL` environment variable. Possible values are `trace`, `debug`, `info`, `warning`, `error`, and `fatal`.

The log file can be specified using the `MINILOG_FILE` environment variable. 
Set `MINILOG_FILE` to the desired log file path.
