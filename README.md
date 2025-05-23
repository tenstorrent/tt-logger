# tt-logger

A flexible and performant C++ logging library for Tenstorrent projects.

## Features

- Built on `spdlog` for robust and performant logging.
- Utilizes the `{fmt}` library for Python-style, type-safe message formatting.
- Compile-time validation of format strings.
- Supports standard log levels: `trace`, `debug`, `info`, `warning`, `error`, `critical` (see table below for details).
- Categorization of logs using `LogType` (e.g., `LogDevice`, `LogModel`).
- Optional category specification, defaults to `LogAlways`.
- Efficient logging: Formatting cost is avoided for `trace` and `debug` messages if the level is disabled.
- Header-only library for easy integration.
- Macro-based implementation for automatic source location tracking.
- Logging behavior can be customized, just as you would customize the default logger in spdlog.

## Log levels
Log level is explicit in the API call.

| tt-logger API        | spdlog Function       | Level      | Description                                                                                         | When to Use                                                                          |
|----------------------|------------------------|------------|-----------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------|
| `log_trace(...)`     | `spdlog::trace()`      | `trace`    | Very detailed debug info. Useful for tracing program flow or variable changes in critical sections. | In development only, deep-dive into complex logic, loops, or performance paths.      |
| `log_debug(...)`     | `spdlog::debug()`      | `debug`    | General debugging messages.                                                                         | For diagnostics during development. Should be compiled out or disabled in release.   |
| `log_info(...)`      | `spdlog::info()`       | `info`     | General operational messages.                                                                       | To report expected events, like startup/shutdown, config details, or progress.       |
| `log_warning(...)`   | `spdlog::warn()`       | `warn`     | Something unexpected but not immediately harmful.                                                   | For soft failures, fallbacks, deprecated usage, or retry scenarios.                  |
| `log_error(...)`     | `spdlog::error()`      | `error`    | An error occurred, affecting the current operation.                                                 | For recoverable errors like failed file loads, failed RPCs, etc.                     |
| `log_critical(...)`  | `spdlog::critical()`   | `critical` | Severe error. Program may not continue safely.                                                      | For unrecoverable states: corrupt data, system errors, fatal initialization failure. |

### Error vs Critical

`error`: Something has gone wrong, but the system may be able to continue. It might or might not recover.

Examples:

- A file failed to open

- A network call timed out

- A configuration was invalid but defaulted

`critical`: A serious failure that usually makes the application unstable or unable to continue running safely.

Examples:

- Out-of-memory

- Corrupted internal state

- Failed assertion or invariant violation

## Dependencies

`tt-logger` requires the following libraries:

- **fmt** (tested with `11.1.4`): Used for message formatting.
- **spdlog** (tested with `1.15.2`): Used for the logging backend.

These dependencies must be made available to `tt-logger` during the CMake configuration. You can achieve this by:

1.  **Defining them in your project first:** Add `fmt` and `spdlog` (e.g., using `CPMAddPackage`, `FetchContent`, `find_package`, etc.) in your main `CMakeLists.txt` *before* adding `tt-logger`. This is the recommended approach for better version control across your project.
2.  **Letting `tt-logger` fetch them:** If the `fmt::fmt-header-only` and `spdlog::spdlog_header_only` targets are not found, `tt-logger` will attempt to download them using CPM as specified in its `CMakeLists.txt`.

- **Catch2**: Required only for building and running tests (`TT_LOGGER_BUILD_TESTING=ON`). This is managed within the `tests/` directory.

## Building

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run tests
ctest --test-dir build

# Install (optional)
cmake --install build
```

## Usage

```cpp
#include <tt-logger/tt-logger.hpp>

int main() {
    // Log with different levels and categories
    log_info(tt::LogDevice, "Device message");
    log_debug(tt::LogOp, "Op debug message");
    log_warning(tt::LogLLRuntime, "Runtime warning");
    log_error(tt::LogDevice, "Device error");
    log_critical(tt::LogOp, "Op critical error");

    // Log with format strings
    log_info(tt::LogDevice, "Device {} message", 123);
    log_info(tt::LogOp, "Op {} with {} parameters", "test", 42);

    // Log with default category (LogAlways)
    log_info("Default category message");

    // Log with source location automatically included
    log_error(tt::LogDevice, "Error occurred in function: {}", __func__);

    return 0;
}
```

The logging macros automatically include source location information (file, line number, and function name) in the log output. This is handled transparently by the macro implementation.

## Logger Initialization

The `LoggerInitializer` class provides a convenient way to set up the logging system with either file-based or console-based logging. It supports configuration through environment variables.

### Basic Usage

```cpp
#include <tt-logger/tt-logger-initializer.hpp>

int main() {
    // Initialize logger with default environment variables
    tt::LoggerInitializer logger_init;

    // Now you can use the logging functions
    tt::log_info("Logger initialized");
    return 0;
}
```

### Environment Variables

The `LoggerInitializer` can be configured using the following environment variables:

- `TT_LOGGER_FILE`: Path to the log file. If not set or empty, logs will be written to stdout.
- `TT_LOGGER_LEVEL`: Log level (trace, debug, info, warning, error, critical). Defaults to "info" if not set.

Example:
```bash
# Log to a file with debug level
export TT_LOGGER_FILE=/path/to/logfile.log
export TT_LOGGER_LEVEL=debug

# Log to stdout with warning level
export TT_LOGGER_LEVEL=warning
```

### Custom Environment Variable Names

You can also specify custom environment variable names when creating the initializer:

```cpp
// Use custom environment variable names
tt::LoggerInitializer logger_init("CUSTOM_LOG_FILE", "CUSTOM_LOG_LEVEL");
```

## Available Categories
The following log categories are available:

- Always
- Test
- Timer
- Device
- LLRuntime
- Loader
- BuildKernels
- Verif
- Op
- Dispatch
- Fabric
- Metal
- TTNN
- MetalTrace
- SiliconDriver
- EmulationDriver

Each category is prefixed with "Log" when used in code (e.g., `tt::LogDevice`, `tt::LogOp`).

## CMake Integration

Choose what suits your needs:

  - `add_subdirectory`
  - `submodule`
  - `ExternalProject_Add`
  - `FetchContent`
  - `CPM`

CPM Example:
```cmake
# Add CPM
include(cmake/CPM.cmake)

# Add tt-logger
CPMAddPackage(
    NAME tt-logger
    GITHUB_REPOSITORY tenstorrent/tt-logger
    VERSION 1.0.0 # Or your desired version/tag
)

# Link to your target
target_link_libraries(your_target PRIVATE tt-logger::tt-logger)
```

## Project Structure

```
tt-logger/
├── include/
│   └── tt-logger/
│       └── tt-logger.hpp
├── tests/
│   ├── tt-logger-test.cpp
│   └── CMakeLists.txt
├── cmake/
│   └── CPM.cmake
├── CMakeLists.txt
└── README.md
```
