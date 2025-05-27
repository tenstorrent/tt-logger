/**
 * @file test_logger_initializer.cpp
 * @brief Test file for the TT Logger initialization and basic logging functionality
 *
 * This test file demonstrates the usage of the TT Logger system, including:
 * - Logger initialization using environment variables
 * - Basic logging functionality across all log levels (trace through critical)
 *
 * @note Requires setting the environment variable TT_METAL_LOGGER_LEVEL=debug before running
 */

#include <tt-logger/tt-logger-initializer.hpp>
#include <tt-logger/tt-logger.hpp>

using namespace tt;

// Static logger initialization
constexpr auto env_file_var  = "TT_METAL_LOGGER_FILE";
constexpr auto env_level_var = "TT_METAL_LOGGER_LEVEL";
constexpr auto log_pattern   = "[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v";

static LoggerInitializer _logger(env_file_var, env_level_var, log_pattern);

int main() {
    // Test using tt-logger macros
    log_trace(LogAlways, "This is a trace message");
    log_debug(LogAlways, "This is a debug message");
    log_info(LogAlways, "This is an info message");
    log_warning(LogAlways, "This is a warning message");
    log_error(LogAlways, "This is an error message");
    log_critical(LogAlways, "This is a critical message");

    return 0;
}
