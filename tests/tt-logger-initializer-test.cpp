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
static LoggerInitializer _logger("TT_METAL_LOGGER_FILE", "TT_METAL_LOGGER_LEVEL");

int main() {
    // Test using tt-logger macros
    log_trace("This is a trace message");
    log_debug("This is a debug message");
    log_info("This is an info message");
    log_warning("This is a warning message");
    log_error("This is an error message");
    log_critical("This is a critical message");

    return 0;
}
