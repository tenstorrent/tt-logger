// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file tt-logger.hpp
 * @brief A logging utility header that provides type-safe logging functionality with different log levels and categories.
 */

#pragma once

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#ifndef TT_LOGGER_TYPES
#define TT_LOGGER_TYPES \
    X(Always)        \
    X(Test)          \
    X(Timer)         \
    X(Device)        \
    X(LLRuntime)     \
    X(Loader)        \
    X(BuildKernels)  \
    X(Verif)         \
    X(Op)            \
    X(Dispatch)      \
    X(Fabric)        \
    X(Metal)         \
    X(TTNN)          \
    X(MetalTrace)    \
    X(Inspector)     \
    X(SiliconDriver) \
    X(EmulationDriver)
#endif

namespace tt {

/**
 * @brief Enumeration of different log categories/types supported by the logger.
 *
 * Each type represents a different component or subsystem that can be logged.
 * The types are defined using the TT_LOGGER_TYPES macro for easy extension.
 */
// clang-format off
enum LogType {
#define X(a) Log ## a,
    TT_LOGGER_TYPES
#undef X
};
// clang-format on

/**
 * @brief Array containing string representations of all log types.
 *
 * The array is indexed by the LogType enum values and contains the corresponding
 * string names for each log type.
 */
constexpr std::array<const char *, std::size_t(LogType::LogEmulationDriver) + 1> log_type_names = {
#define X(name) #name,
    TT_LOGGER_TYPES
#undef X
};

static_assert(log_type_names.size() <= 64, "LogType bitmask only supports up to 64 log types");

/**
 * @brief Converts a LogType enum value to its string representation.
 *
 * @param logtype The LogType enum value to convert
 * @return const char* The string representation of the log type
 */
constexpr const char * logtype_to_string(LogType logtype) noexcept {
    return static_cast<std::size_t>(logtype) < log_type_names.size() ?
               log_type_names[static_cast<std::size_t>(logtype)] :
               "UnknownType";
}

/**
 * @brief LogType filtering manager
 *
 * This class handles the filtering of log messages based on LogType using environment variables.
 * It uses a bitset approach for efficient and safe runtime filtering.
 */
class LogTypeFilter {
  private:
    std::bitset<64> enabled_mask;  // Support up to 64 log types

    LogTypeFilter() {
        // Always enable LogAlways by default
        enabled_mask.set(LogAlways);

        // Parse TT_LOGGER_TYPES environment variable
        const char * types_env = std::getenv("TT_LOGGER_TYPES");
        if (types_env) {
            if (std::strstr(types_env, "All")) {
                // Enable all log types
                enabled_mask.set();  // Set all bits
            } else {
                // Parse specific log types
                enabled_mask.reset();  // Clear all bits first
                std::uint32_t type_index = 0;
                for (const char * type_name : log_type_names) {
                    if (std::strstr(types_env, type_name) != nullptr) {
                        enabled_mask.set(type_index);
                    }
                    type_index++;
                }
                // Always ensure LogAlways is enabled
                enabled_mask.set(LogAlways);
            }
        }
        // If no environment variable is set, default to all types enabled
        else {
            enabled_mask.set();  // Set all bits
        }
    }

  public:
    /**
     * @brief Get the singleton instance of LogTypeFilter
     * @return Reference to the LogTypeFilter instance
     */
    static LogTypeFilter & get() {
        static LogTypeFilter instance;
        return instance;
    }

    /**
     * @brief Check if a specific LogType is enabled for logging
     * @param logtype The LogType to check
     * @return true if the LogType is enabled, false otherwise
     */
    bool log(LogType logtype) const noexcept { return enabled_mask.test(static_cast<std::size_t>(logtype)); }
};

}  // namespace tt

/**
 * @brief Custom formatter for tt::LogType to enable formatting in fmt library.
 */
namespace fmt {
template <> struct formatter<tt::LogType> : fmt::formatter<std::string_view> {
    template <typename FormatContext> constexpr auto format(tt::LogType logtype, FormatContext & ctx) const {
        return fmt::formatter<std::string_view>::format(tt::logtype_to_string(logtype), ctx);
    }
};
}  // namespace fmt
#undef TT_LOGGER_TYPES

/**
 * @brief Logging macros for different severity levels with LogType filtering
 *
 * These macros provide type-safe logging functionality with different severity levels.
 * They include runtime filtering based on LogType using environment variables.
 * Each macro takes the following parameters:
 * @param type The log type/category for the message (e.g., LogDevice, LogOp)
 * @param fmt Format string for the log message
 * @param ... Variadic arguments to be formatted into the message
 *
 * Environment Variables:
 * - TT_LOGGER_TYPES: Comma-separated list of log types to enable (e.g., "Device,Op") or "All"
 * - TT_LOGGER_LEVEL: Log level (trace, debug, info, warning, error, critical)
 * - TT_LOGGER_FILE: Path to log file (if not set, logs to stdout)
 *
 * Available macros:
 * - log_trace: Very detailed debug info, useful for tracing program flow
 * - log_debug: General debugging messages
 * - log_info: General operational messages
 * - log_warning: Something unexpected but not immediately harmful
 * - log_error: An error occurred, affecting the current operation
 * - log_critical: Severe error, program may not continue safely
 * - log_fatal: Alias for log_critical
 */
#ifdef NDEBUG

#    define log_trace(type, fmt, ...)    SPDLOG_TRACE(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__)
#    define log_debug(type, fmt, ...)    SPDLOG_DEBUG(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__)
#    define log_info(type, fmt, ...)     SPDLOG_INFO(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__)
#    define log_warning(type, fmt, ...)  SPDLOG_WARN(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__)
#    define log_error(type, fmt, ...)    SPDLOG_ERROR(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__)
#    define log_critical(type, fmt, ...) SPDLOG_CRITICAL(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__)
#    define log_fatal(type, fmt, ...)    SPDLOG_CRITICAL(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__)

#else

#    define log_trace(type, fmt, ...)                                       \
        do {                                                                \
            if (tt::LogTypeFilter::get().log(type)) {                       \
                SPDLOG_TRACE(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__); \
            }                                                               \
        } while (0)
#    define log_debug(type, fmt, ...)                                       \
        do {                                                                \
            if (tt::LogTypeFilter::get().log(type)) {                       \
                SPDLOG_DEBUG(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__); \
            }                                                               \
        } while (0)
#    define log_info(type, fmt, ...)                                       \
        do {                                                               \
            if (tt::LogTypeFilter::get().log(type)) {                      \
                SPDLOG_INFO(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__); \
            }                                                              \
        } while (0)
#    define log_warning(type, fmt, ...)                                    \
        do {                                                               \
            if (tt::LogTypeFilter::get().log(type)) {                      \
                SPDLOG_WARN(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__); \
            }                                                              \
        } while (0)
#    define log_error(type, fmt, ...)                                       \
        do {                                                                \
            if (tt::LogTypeFilter::get().log(type)) {                       \
                SPDLOG_ERROR(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__); \
            }                                                               \
        } while (0)
#    define log_critical(type, fmt, ...)                                       \
        do {                                                                   \
            if (tt::LogTypeFilter::get().log(type)) {                          \
                SPDLOG_CRITICAL(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__); \
            }                                                                  \
        } while (0)
#    define log_fatal(type, fmt, ...)                                          \
        do {                                                                   \
            if (tt::LogTypeFilter::get().log(type)) {                          \
                SPDLOG_CRITICAL(FMT_STRING("[{}] " fmt), type, ##__VA_ARGS__); \
            }                                                                  \
        } while (0)

#endif
