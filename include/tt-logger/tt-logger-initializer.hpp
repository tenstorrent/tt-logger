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
#include <tt-logger/remote_log_sink.hpp>
#include <tt-logger/consts.hpp>

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <strings.h>

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

    void configure_logger(std::shared_ptr<spdlog::sinks::sink> sink, const std::string & pattern, 
        bool enable_remote, const std::string& server_socket) {
        auto logger = std::make_shared<spdlog::logger>("", sink);
        if (enable_remote) {
            if (server_socket.empty()) {
                logger->sinks().push_back(std::make_shared<internal::RemoteLogSink_t>());
            } else {
                logger->sinks().push_back(std::make_shared<internal::RemoteLogSink_t>(server_socket));
            }
        }

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
    LoggerInitializer(std::string file_env = internal::tt_logger_file_env, std::string level_env = internal::tt_log_level_env,
                      std::string pattern = "", std::string enable_remote_env = internal::tt_remote_logger_env, std::string server_socket_env = internal::tt_remote_logger_socket_env) noexcept {
        const char * file_path = std::getenv(file_env.c_str());
        auto         sink      = create_sink(file_path ? file_path : "");
        const char * remote_logger = std::getenv(enable_remote_env.c_str());
        bool enable_remote = false;
        std::string server_socket;
        if (remote_logger != nullptr) {
            if (::strcasecmp(remote_logger, "true") == 0 ||
                ::strcasecmp(remote_logger, "on") == 0) {
                enable_remote = true;
                const char* server_socket_file_env = std::getenv(server_socket_env.c_str());
                if (server_socket_file_env != nullptr) {
                    server_socket = server_socket_file_env;
                }
            } else {
                std::cerr << "Only use true or on to enable: " << internal::tt_remote_logger_env << "\n";
            }
        }

        configure_logger(sink, pattern, enable_remote, server_socket);

        spdlog::cfg::load_env_levels(level_env.c_str());  // Defaults to "info" if no ENV var set
    }
};

}  // namespace tt
