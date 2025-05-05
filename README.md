# tt-logger

A Tenstorrent logging library built on top of spdlog and fmt.

## Features

- Built on top of spdlog for robust logging infrastructure

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

- spdlog (automatically managed via CPM)
- fmt (automatically managed via CPM)
- Catch2 (for tests)

## Building

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .

# Run tests
ctest

# Install (optional)
cmake --install .
```

## Usage

```cpp
#include <tt-logger/tt-logger.hpp>

int main() {
    // Log with different levels and categories
    tt::log_info(tt::LogDevice, "Device message");
    tt::log_debug(tt::LogModel, "Model debug message");
    tt::log_warning(tt::LogLLRuntime, "Runtime warning");
    tt::log_error(tt::LogDevice, "Device error");
    tt::log_critical(tt::LogModel, "Model critical error");

    // Log with format strings
    tt::log_info(tt::LogDevice, "Device {} message", 123);
    tt::log_info(tt::LogModel, "Model {} with {} parameters", "test", 42);

    // Log with default category (LogAlways)
    tt::log_info("Default category message");

    return 0;
}
```

## Available Categories
To be refined

- Always
- Test
- Timer
- Device
- Model
- LLRuntime
- Loader
- IO
- CompileTrisc
- BuildKernels
- Verif
- Golden
- Op
- HLK
- HLKC
- Reportify
- GraphCompiler
- Dispatch
- Fabric
- Metal
- MetalTrace
- SiliconDriver
- EmulationDriver

## CMake Integration

Add to your project's CMakeLists.txt:

```cmake
# Add CPM
include(cmake/CPM.cmake)

# Add tt-logger
CPMAddPackage("gh:blozano-tt/tt-logger@0.2.0")

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

## License

Copyright 2024 Tenstorrent Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
