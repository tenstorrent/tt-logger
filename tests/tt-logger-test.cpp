// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file tt-logger-test.cpp
 * @brief Test suite for the tt-logger library
 *
 * This file contains comprehensive unit tests for the tt-logger library, including:
 * - Basic logging functionality (info, debug, warning, error, critical)
 * - Format string functionality with various argument types
 * - Log level filtering and type mapping
 * - File logging capabilities
 * - Performance testing for logging operations
 *
 * The tests use Catch2 framework and include custom sink implementations for testing.
 */

#include <fmt/std.h>  // needed for filesystem::path formatting
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tt-logger/tt-logger.hpp>

#define TT_LOGGER_TESTING

// Custom sink to capture log output when TT_LOGGER_TESTING is defined
#ifdef TT_LOGGER_TESTING
class TestSink : public spdlog::sinks::base_sink<std::mutex> {
  public:
    std::stringstream buffer;

  protected:
    void sink_it_(const spdlog::details::log_msg & msg) override {
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        buffer << fmt::to_string(formatted);
    }

    void flush_() override { buffer.flush(); }
};
#endif

// Setup logger: capture or normal depending on TT_LOGGER_TESTING
static std::shared_ptr<void> setup_logger() {
#ifdef TT_LOGGER_TESTING
    auto sink   = std::make_shared<TestSink>();
    auto logger = std::make_shared<spdlog::logger>("tt-logger-test", sink);
    spdlog::set_default_logger(logger);
    return sink;
#else
    return nullptr;
#endif
}

// Get captured output (only in testing)
static std::string get_output(void * sink_ptr) {
#ifdef TT_LOGGER_TESTING
    auto sink = static_cast<TestSink *>(sink_ptr);
    return sink->buffer.str();
#else
    return {};
#endif
}

// Clear captured output (only in testing)
static void clear_output(void * sink_ptr) {
#ifdef TT_LOGGER_TESTING
    auto sink = static_cast<TestSink *>(sink_ptr);
    sink->buffer.str("");
    sink->buffer.clear();
#endif
}

// Smart checker: validate when testing, force success otherwise
static void soft_check_log_contains(void * sink_ptr, const std::string & expected) {
#ifdef TT_LOGGER_TESTING
    auto output = get_output(sink_ptr);
    INFO("Expected log to contain: '" << expected << "'\nActual log output:\n" << output);
    REQUIRE(output.find(expected) != std::string::npos);
    clear_output(sink_ptr);
#else
    (void) sink_ptr;
    (void) expected;
    SUCCEED();
#endif
}

TEST_CASE("Basic logging functionality", "[logger]") {
    auto sink = setup_logger();

    SECTION("String literal") {
        tt::log_info("This is a string literal");
        soft_check_log_contains(sink.get(), "[Always] This is a string literal");
    }

    SECTION("Info") {
        tt::log_info(tt::LogDevice, "Device message");
        soft_check_log_contains(sink.get(), "[Device] Device message");
    }

    SECTION("Debug") {
        spdlog::default_logger()->set_level(spdlog::level::debug);
        tt::log_debug(tt::LogOp, "Model debug message");
        soft_check_log_contains(sink.get(), "[Op] Model debug message");
    }

    SECTION("Warning") {
        tt::log_warning(tt::LogLLRuntime, "Runtime warning");
        soft_check_log_contains(sink.get(), "[LLRuntime] Runtime warning");
    }

    SECTION("Error") {
        tt::log_error(tt::LogDevice, "Device error");
        soft_check_log_contains(sink.get(), "[Device] Device error");
    }

    SECTION("Critical") {
        tt::log_critical(tt::LogOp, "Model critical error");
        soft_check_log_contains(sink.get(), "[Op] Model critical error");
    }
}

TEST_CASE("Format string functionality", "[logger]") {
    auto sink = setup_logger();

    SECTION("Single argument") {
        tt::log_info(tt::LogDevice, "Device {} message", 123);
        soft_check_log_contains(sink.get(), "[Device] Device 123 message");
    }

    SECTION("Multiple arguments") {
        tt::log_info(tt::LogOp, "Model {} with {} parameters", "test", 42);
        soft_check_log_contains(sink.get(), "[Op] Model test with 42 parameters");
    }

    SECTION("Filesystem path formatting") {
        std::filesystem::path p = "/usr/bin/hello";
        tt::log_info(tt::LogOp, "Path: {}", p);
        soft_check_log_contains(sink.get(), "[Op] Path: /usr/bin/hello");
    }
}

TEST_CASE("Log level filtering", "[logger]") {
    auto sink   = setup_logger();
    auto logger = spdlog::default_logger();

    SECTION("Debug filtering") {
        logger->set_level(spdlog::level::debug);

        tt::log_trace(tt::LogDevice, "Should not appear");
        tt::log_debug(tt::LogDevice, "Should appear");

#ifdef TT_LOGGER_TESTING
        auto output = get_output(sink.get());
        INFO("Captured output:\n" << output);
        REQUIRE(output.find("Should not appear") == std::string::npos);
        REQUIRE(output.find("Should appear") != std::string::npos);
        clear_output(sink.get());
#else
        SUCCEED();
#endif
    }

    SECTION("Info filtering") {
        logger->set_level(spdlog::level::info);

        tt::log_debug(tt::LogDevice, "Should not appear");
        tt::log_info(tt::LogDevice, "Should appear");

#ifdef TT_LOGGER_TESTING
        auto output = get_output(sink.get());
        INFO("Captured output:\n" << output);
        REQUIRE(output.find("Should not appear") == std::string::npos);
        REQUIRE(output.find("Should appear") != std::string::npos);
        clear_output(sink.get());
#else
        SUCCEED();
#endif
    }
}

TEST_CASE("Log type to string mapping", "[logger]") {
    REQUIRE(std::string(tt::logtype_to_string(tt::LogDevice)) == "Device");
    REQUIRE(std::string(tt::logtype_to_string(tt::LogOp)) == "Op");
    REQUIRE(std::string(tt::logtype_to_string(tt::LogLLRuntime)) == "LLRuntime");
}

TEST_CASE("Default log type behavior", "[logger]") {
    auto sink = setup_logger();

    SECTION("Defaults to LogAlways") {
        tt::log_info("Default type message");
        soft_check_log_contains(sink.get(), "[Always] Default type message");
    }
}

TEST_CASE("File logging functionality", "[logger]") {
    // Create a temporary file for testing
    std::filesystem::path temp_log_file = std::filesystem::temp_directory_path() / "tt-logger-test.log";

    SECTION("Basic file logging") {
        // Create a file sink
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(temp_log_file.string());
        auto logger    = std::make_shared<spdlog::logger>("tt-logger-file-test", file_sink);
        spdlog::set_default_logger(logger);

        // Write some test logs
        tt::log_info(tt::LogDevice, "Device file message");
        tt::log_warning(tt::LogOp, "Model file warning");
        tt::log_error(tt::LogLLRuntime, "Runtime file error");

        // Flush and close the logger
        logger->flush();
        spdlog::drop_all();

        // Read and verify the file contents
        std::ifstream     log_file(temp_log_file);
        std::stringstream buffer;
        buffer << log_file.rdbuf();
        std::string file_contents = buffer.str();

        REQUIRE(file_contents.find("[Device] Device file message") != std::string::npos);
        REQUIRE(file_contents.find("[Op] Model file warning") != std::string::npos);
        REQUIRE(file_contents.find("[LLRuntime] Runtime file error") != std::string::npos);

        // Clean up
        std::filesystem::remove(temp_log_file);
    }
}

#ifdef TT_LOGGER_TESTING
TEST_CASE("Performance testing", "[logger]") {
    constexpr int num_iterations = 10000;  // Reduced iterations for testing

    SECTION("log_info performance") {
        try {
            auto sink = setup_logger();
            REQUIRE(sink != nullptr);

            auto logger = spdlog::default_logger();
            REQUIRE(logger != nullptr);

            logger->set_level(spdlog::level::info);

            // Test a single log first to verify setup
            tt::log_info(tt::LogDevice, "Test setup message");

            auto start = std::chrono::high_resolution_clock::now();

            for (int i = 0; i < num_iterations; ++i) {
                tt::log_info(tt::LogDevice, "Performance test message {}", i);
            }

            auto end      = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            double avg_time_per_log = static_cast<double>(duration.count()) / num_iterations;
            std::cout << "Average time per log_info call: " << avg_time_per_log << " microseconds\n";

            REQUIRE(duration.count() > 0);

            // Cleanup
            spdlog::drop_all();
        } catch (const std::exception & e) {
            FAIL("Exception in log_info performance test: " << e.what());
        }
    }

    SECTION("log_debug performance when level is info") {
        try {
            auto sink = setup_logger();
            REQUIRE(sink != nullptr);

            auto logger = spdlog::default_logger();
            REQUIRE(logger != nullptr);

            logger->set_level(spdlog::level::info);

            // Test a single log first to verify setup
            tt::log_debug(tt::LogOp, "Test setup message");

            auto start = std::chrono::high_resolution_clock::now();

            for (int i = 0; i < num_iterations; ++i) {
                tt::log_debug(tt::LogOp, "Debug performance test message {}", i);
            }

            auto end      = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            double avg_time_per_log = static_cast<double>(duration.count()) / num_iterations;
            std::cout << "Average time per log_debug call: " << avg_time_per_log << " microseconds\n";

            REQUIRE(duration.count() > 0);

            // Cleanup
            spdlog::drop_all();
        } catch (const std::exception & e) {
            FAIL("Exception in log_debug performance test: " << e.what());
        }
    }
}
#endif
