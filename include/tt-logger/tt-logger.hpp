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
#include <cstddef>
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
 * @brief Logs a trace level message with a specific log type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param type The log type/category
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_trace(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
    }
}

/**
 * @brief Logs a trace level message with default LogAlways type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_trace(fmt::format_string<Args...> fmt, Args &&... args) {
    log_trace(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

/**
 * @brief Logs a debug level message with a specific log type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param type The log type/category
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_debug(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    if (spdlog::should_log(spdlog::level::debug)) {
        spdlog::debug("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
    }
}

/**
 * @brief Logs a debug level message with default LogAlways type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_debug(fmt::format_string<Args...> fmt, Args &&... args) {
    log_debug(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

/**
 * @brief Logs an info level message with a specific log type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param type The log type/category
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_info(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::info("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief Logs an info level message with default LogAlways type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_info(fmt::format_string<Args...> fmt, Args &&... args) {
    log_info(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

/**
 * @brief Logs a warning level message with a specific log type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param type The log type/category
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_warning(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::warn("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief Logs a warning level message with default LogAlways type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_warning(fmt::format_string<Args...> fmt, Args &&... args) {
    log_warning(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

/**
 * @brief Logs a critical level message with a specific log type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param type The log type/category
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_critical(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::critical("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief Logs a critical level message with default LogAlways type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_critical(fmt::format_string<Args...> fmt, Args &&... args) {
    log_critical(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

/**
 * @brief Logs a fatal level message with a specific log type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param type The log type/category
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_fatal(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::critical("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief Logs a fatal level message with default LogAlways type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_fatal(fmt::format_string<Args...> fmt, Args &&... args) {
    log_fatal(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

/**
 * @brief Logs an error level message with a specific log type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param type The log type/category
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_error(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::error("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief Logs an error level message with default LogAlways type.
 *
 * @tparam Args Variadic template parameter for format arguments
 * @param fmt The format string
 * @param args The format arguments
 */
template <typename... Args> inline void log_error(fmt::format_string<Args...> fmt, Args &&... args) {
    log_error(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

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
