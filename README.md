# tt-logger

A flexible and performant C++ logging library for Tenstorrent projects.

## Features

- Built on `spdlog` for robust and performant logging.
- Utilizes the `{fmt}` library for Python-style, type-safe message formatting.
- Compile-time validation of format strings.
- Supports standard log levels: `trace`, `debug`, `info`, `warning`, `error`, `critical` (see table below for details).
- Categorization of logs using `LogType` (e.g., `LogDevice`, `LogOp`).
- Optional category specification, defaults to `LogAlways`.
- **Compile-time elimination**: Log statements can be completely eliminated at compile time using `SPDLOG_ACTIVE_LEVEL`, just like spdlog.
- **Runtime filtering**: Log categories can be filtered at runtime using the `TT_LOGGER_TYPES` environment variable.
- Efficient logging: Formatting cost is avoided for disabled log levels.
- Header-only library for easy integration.
- Macro-based implementation for automatic source location tracking.
- Logging behavior can be customized, just as you would customize the default logger in spdlog.

## Log levels
Log level is explicit in the API call.

| tt-logger API                    | Description                                                                                         | When to Use                                                                          |
|----------------------------------|-----------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------|
| `log_trace(LogType, ...)`        | Very detailed debug info. Useful for tracing program flow or variable changes in critical sections. | In development only, deep-dive into complex logic, loops, or performance paths.      |
| `log_debug(LogType, ...)`        | General debugging messages.                                                                         | For diagnostics during development. Should be compiled out or disabled in release.   |
| `log_info(LogType, ...)`         | General operational messages.                                                                       | To report expected events, like startup/shutdown, config details, or progress.       |
| `log_warning(LogType, ...)`      | Something unexpected but not immediately harmful.                                                   | For soft failures, fallbacks, deprecated usage, or retry scenarios.                  |
| `log_error(LogType, ...)`        | An error occurred, affecting the current operation.                                                 | For recoverable errors like failed file loads, failed RPCs, etc.                     |
| `log_critical(LogType, ...)`     | Severe error. Program may not continue safely.                                                      | For unrecoverable states: corrupt data, system errors, fatal initialization failure. |

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

## Log Categories
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
- Inspector
- SiliconDriver
- EmulationDriver

Each category is prefixed with "Log" when used in code (e.g., `tt::LogDevice`, `tt::LogOp`, `tt::LogInspector`).

## Dependencies

`tt-logger` requires the following libraries:

- **fmt** (tested with `11.1.4`): Used for message formatting.
- **spdlog** (tested with `1.15.2`): Used for the logging backend.

These dependencies must be made available to `tt-logger` during the CMake configuration. You can achieve this by:

1.  **Defining them in your project first:** Add `fmt` and `spdlog` (e.g., using `CPMAddPackage`, `FetchContent`, `find_package`, etc.) in your main `CMakeLists.txt` *before* adding `tt-logger`. This is the recommended approach for better version control across your project.
2.  **Letting `tt-logger` fetch them:** If the `fmt::fmt-header-only` and `spdlog::spdlog_header_only` targets are not found, `tt-logger` will attempt to download them using CPM as specified in its `CMakeLists.txt`.

## Building

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Build and run tests (optional)
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DTT_LOGGER_BUILD_TESTING=ON
cmake --build build
./build/tests/tt-logger-test

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

    // Log with source location automatically included
    log_error(tt::LogDevice, "Error occurred in function: {}", __func__);

    return 0;
}
```

The logging macros automatically include source location information (file, line number, and function name) in the log output. This is handled transparently by the macro implementation.

### Basic Usage

```cpp
#include <tt-logger/tt-logger-initializer.hpp>

int main() {
    // Initialize logger with default environment variables
    tt::LoggerInitializer logger_init;

    // Now you can use the logging functions
    tt::log_info(tt::LogAlways, "Logger initialized");
    return 0;
}
```

### Environment Variables

The logger can be configured using the following environment variables:

- `TT_LOGGER_FILE`: Path to the log file. If not set or empty, logs will be written to stdout.
- `TT_LOGGER_LEVEL`: Log level (trace, debug, info, warning, error, critical). Defaults to "info" if not set.
- `TT_LOGGER_TYPES`: Semicolon-separated list of log categories to enable. Use "All" to enable all categories. If not set, all categories are enabled by default.

Example:
```bash
# Log to a file with debug level
export TT_LOGGER_FILE=/path/to/logfile.log
export TT_LOGGER_LEVEL=debug

# Log to stdout with warning level
export TT_LOGGER_LEVEL=warning

# Only log Device and Op messages
export TT_LOGGER_TYPES=Device,Op

# Enable all log types explicitly
export TT_LOGGER_TYPES=All
```

### Log Category Filtering with TT_LOGGER_TYPES

The `TT_LOGGER_TYPES` environment variable allows you to filter which log categories are active at runtime. This is useful for focusing on specific subsystems during debugging.

```bash
# Only show Metal and TTNN logs
export TT_LOGGER_TYPES="Metal;TTNN"

# Show all device-related logs
export TT_LOGGER_TYPES="Device;SiliconDriver;EmulationDriver"
```

## Compile-time Log Level Control

`tt-logger` supports compile-time elimination of log statements, controlled by the `SPDLOG_ACTIVE_LEVEL` macro, just like spdlog. This allows you to completely remove lower-level log statements from release builds for maximum performance.

// Compile with -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO to eliminate
// trace and debug statements at compile time
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include <tt-logger/tt-logger.hpp>

int main() {
    // These calls will be compiled to (void)0 and completely eliminated
    log_trace(tt::LogOp, "This trace message is eliminated");
    log_debug(tt::LogOp, "This debug message is eliminated");

    // These calls will be compiled normally
    log_info(tt::LogOp, "This info message remains");
    log_error(tt::LogOp, "This error message remains");
    return 0;
}
```

Available compile-time levels:
- `SPDLOG_LEVEL_TRACE` (0) - All messages compiled
- `SPDLOG_LEVEL_DEBUG` (1) - Debug and above compiled
- `SPDLOG_LEVEL_INFO` (2) - Info and above compiled (recommended for release)
- `SPDLOG_LEVEL_WARN` (3) - Warning and above compiled
- `SPDLOG_LEVEL_ERROR` (4) - Error and above compiled
- `SPDLOG_LEVEL_CRITICAL` (5) - Only critical messages compiled
- `SPDLOG_LEVEL_OFF` (6) - All logging disabled

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
    VERSION 1.0.7 # Or your desired version/tag
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
