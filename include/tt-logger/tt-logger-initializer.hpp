// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file tt-logger-initializer.hpp
 * @brief Header file for the TT Logger initialization system
 *
 * This file contains the LoggerInitializer class which handles the setup and configuration
 * of the logging system using spdlog. It provides functionality for both file-based and
 * console-based logging with environment variable configuration support.
 */

#pragma once

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <memory>
#include <string>

namespace tt {

/**
 * @brief Initializes and configures the logging system
 *
 * This class handles the setup of spdlog with either file-based or console-based logging.
 * It supports configuration through environment variables.
 */
class LoggerInitializer {
  private:
    std::shared_ptr<spdlog::sinks::sink> create_sink(const std::string & file_path) {
        if (file_path.empty()) {
            return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        }

        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path, true);
        if (!sink) {
            std::fprintf(stderr, "tt-logger failed to create log file '%s'\n", file_path.c_str());
            std::abort();
        }
        return sink;
    }

    void configure_logger(std::shared_ptr<spdlog::sinks::sink> sink, const std::string & pattern) {
        auto logger = std::make_shared<spdlog::logger>("", sink);
        spdlog::set_default_logger(logger);
        if (!pattern.empty()) {
            spdlog::set_pattern(pattern);
        }
        spdlog::flush_on(spdlog::level::err);
    }

  public:
    /**
     * @brief Constructs a LoggerInitializer with environment variable configuration
     *
     * @param file_env Environment variable name for log file path
     * @param level_env Environment variable name for log level
     * @param pattern The pattern string to use for log formatting
     */
    LoggerInitializer(std::string file_env = "TT_LOGGER_FILE", std::string level_env = "TT_LOGGER_LEVEL",
                      std::string pattern = "") noexcept {
        const char * file_path = std::getenv(file_env.c_str());
        auto         sink      = create_sink(file_path ? file_path : "");
        configure_logger(sink, pattern);
        spdlog::cfg::load_env_levels(level_env.c_str());  // Defaults to "info" if no ENV var set
    }
};

}  // namespace tt
