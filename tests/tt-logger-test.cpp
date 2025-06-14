// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file tt-logger-test.cpp
 * @brief Simple test program for the tt-logger library
 *
 * This file contains a simple test program that demonstrates:
 * - Basic logging functionality (info, debug, warning, error, critical)
 * - Format string functionality with various argument types
 * - Log level filtering
 */

#include <fmt/ranges.h>  // needed for container formatting
#include <fmt/std.h>     // needed for filesystem::path formatting

#include <chrono>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <tt-logger/tt-logger.hpp>
#include <vector>

int main() {
    std::cout << "=== TT-Logger Simple Test Program ===" << std::endl;
    std::cout << std::endl;

    // Test 1: Basic logging functionality
    std::cout << "Test 1: Basic logging functionality" << std::endl;
    std::cout << "Expected: Should see log messages with different levels and types" << std::endl;
    std::cout << "Actual output:" << std::endl;

    log_trace(tt::LogOp, "Op trace message");
    log_debug(tt::LogInspector, "Inspector debug message");
    log_info(tt::LogDevice, "Device info message");
    log_warning(tt::LogLLRuntime, "Runtime warning message");
    log_error(tt::LogDevice, "Device error message");
    log_critical(tt::LogOp, "Op critical error message");

    std::cout << std::endl;

    // Test 2: Format string functionality
    std::cout << "Test 2: Format string functionality" << std::endl;
    std::cout << "Expected: Should see formatted messages with arguments" << std::endl;
    std::cout << "Actual output:" << std::endl;

    log_info(tt::LogDevice, "Device {} message with number {}", "formatted", 123);
    log_info(tt::LogOp, "Op test with {} parameters and value {}", "multiple", 42);

    std::cout << std::endl;

    // Test 3: Container formatting
    std::cout << "Test 3: Container formatting" << std::endl;
    std::cout << "Expected: Should see formatted containers (sets and vectors)" << std::endl;
    std::cout << "Actual output:" << std::endl;

    std::set<int>    chip_ids = { 1, 2, 3 };
    std::vector<int> pci_ids  = { 4096, 8192, 12288 };

    log_info(tt::LogSiliconDriver, "Opening chip ids: {} with pci ids: {}", chip_ids, pci_ids);

    std::cout << std::endl;

    // Test 4: Filesystem path formatting
    std::cout << "Test 4: Filesystem path formatting" << std::endl;
    std::cout << "Expected: Should see formatted filesystem path" << std::endl;
    std::cout << "Actual output:" << std::endl;

    std::filesystem::path test_path = "/usr/bin/test";
    log_info(tt::LogOp, "Using path: {}", test_path);

    std::cout << std::endl;

    // Test 5: Log type to string mapping
    std::cout << "Test 5: Log type to string mapping" << std::endl;
    std::cout << "Expected: Device, Op, LLRuntime, SiliconDriver" << std::endl;
    std::cout << "Actual output: " << tt::logtype_to_string(tt::LogDevice) << ", " << tt::logtype_to_string(tt::LogOp)
              << ", " << tt::logtype_to_string(tt::LogLLRuntime) << ", " << tt::logtype_to_string(tt::LogSiliconDriver)
              << std::endl;

    std::cout << std::endl;

    // Test 6: Debug level filtering
    std::cout << "Test 6: Debug level filtering" << std::endl;
    std::cout << "Expected: Debug message should appear when level is set to debug" << std::endl;
    std::cout << "Setting log level to debug..." << std::endl;

    tt::LoggerRegistry::instance().set_level(spdlog::level::debug);
    std::cout << "Actual output:" << std::endl;
    log_debug(tt::LogOp, "This debug message should be visible");

    std::cout << std::endl;

    // Test 7: Debug level filtering (info level)
    std::cout << "Test 7: Debug level filtering (info level)" << std::endl;
    std::cout << "Expected: Debug message should NOT appear when level is set to info" << std::endl;
    std::cout << "Setting log level to info..." << std::endl;

    tt::LoggerRegistry::instance().set_level(spdlog::level::info);
    std::cout << "Actual output (debug should be filtered out):" << std::endl;
    log_debug(tt::LogOp, "This debug message should NOT be visible");
    log_info(tt::LogOp, "This info message should be visible");

    std::cout << std::endl;

    // Performance Tests
    std::cout << "=== Performance Tests ===" << std::endl;

    // Test 8: log_info performance
    std::cout << "Test 8: log_info performance (1000 iterations)" << std::endl;
    auto start_info = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        log_info(tt::LogDevice, "Performance test iteration {}", i);
    }

    auto end_info      = std::chrono::high_resolution_clock::now();
    auto duration_info = std::chrono::duration_cast<std::chrono::microseconds>(end_info - start_info);
    std::cout << "log_info average time per call: " << duration_info.count() / 1000.0 << " microseconds" << std::endl;

    std::cout << std::endl;

    // Test 9: log_debug performance
    std::cout << "Test 9: log_debug performance (1000 iterations)" << std::endl;
    tt::LoggerRegistry::instance().set_level(spdlog::level::debug);  // Ensure debug messages are processed

    auto start_debug = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        log_debug(tt::LogOp, "Performance test iteration {}", i);
    }

    auto end_debug      = std::chrono::high_resolution_clock::now();
    auto duration_debug = std::chrono::duration_cast<std::chrono::microseconds>(end_debug - start_debug);
    std::cout << "log_debug average time per call: " << duration_debug.count() / 1000.0 << " microseconds" << std::endl;

    std::cout << std::endl;

    // Test 10: log_debug performance when filtered out
    std::cout << "Test 10: log_debug performance when filtered out (1000 iterations)" << std::endl;
    tt::LoggerRegistry::instance().set_level(spdlog::level::info);  // Filter out debug messages

    auto start_debug_filtered = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        log_debug(tt::LogOp, "Performance test iteration {}", i);
    }

    auto end_debug_filtered = std::chrono::high_resolution_clock::now();
    auto duration_debug_filtered =
        std::chrono::duration_cast<std::chrono::microseconds>(end_debug_filtered - start_debug_filtered);
    std::cout << "log_debug average time per call (filtered): " << duration_debug_filtered.count() / 1000.0
              << " microseconds" << std::endl;

    std::cout << std::endl;
    std::cout << "=== Performance tests completed ===" << std::endl;

    std::cout << "=== All tests completed ===" << std::endl;

    return 0;
}
