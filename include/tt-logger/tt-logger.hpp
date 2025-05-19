// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file tt-logger.hpp
 * @brief A logging utility header that provides type-safe logging functionality with different log levels and categories.
 * @details This header provides a comprehensive logging system based on spdlog, with support for:
 *          - Multiple log levels (trace, debug, info, warning, error, critical, fatal)
 *          - Custom log categories/types for different components
 *          - Type-safe formatting using fmt library
 *          - Source location tracking
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
 * @brief Gets the current source location information.
 * @param file The source file name (automatically populated)
 * @param line The source line number (automatically populated)
 * @param function The function name (automatically populated)
 * @return spdlog::source_loc The source location information
 * @note Uses compiler-specific builtins when available for better accuracy
 */
constexpr spdlog::source_loc current_source_location(
#if defined(__clang__) || defined(__GNUC__)
    const char * file = __builtin_FILE(), int line = __builtin_LINE(), const char * function = __builtin_FUNCTION()
#else
    const char * file = __FILE__, int line = __LINE__, const char * function = __func__
#endif
        ) noexcept {
    return spdlog::source_loc{ file, line, function };
}

/**
 * @brief Enumeration of different log categories/types supported by the logger.
 * @details Each type represents a different component or subsystem that can be logged.
 *          The types are defined using the TT_LOGGER_TYPES macro for easy extension.
 *          Available types include: Always, Test, Timer, Device, LLRuntime, etc.
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
 * @details The array is indexed by the LogType enum values and contains the corresponding
 *          string names for each log type. Used internally for converting LogType to string.
 */
constexpr std::array<const char *, std::size_t(LogType::LogEmulationDriver) + 1> log_type_names = {
#define X(name) #name,
    TT_LOGGER_TYPES
#undef X
};

/**
 * @brief Converts a LogType enum value to its string representation.
 * @param logtype The LogType enum value to convert
 * @return const char* The string representation of the log type, or "UnknownType" if invalid
 */
constexpr const char * logtype_to_string(LogType logtype) noexcept {
    return static_cast<std::size_t>(logtype) < log_type_names.size() ?
               log_type_names[static_cast<std::size_t>(logtype)] :
               "UnknownType";
}

/**
 * @brief Internal implementation of logging functionality.
 * @tparam T Type of the first argument
 * @tparam Rest Types of the remaining arguments
 * @param loc Source location information
 * @param level Log level to use
 * @param first First argument (either LogType or format string)
 * @param rest Remaining format arguments
 * @details Handles the actual logging logic, supporting both direct logging and category-based logging
 */
template <typename T, typename... Rest>
static constexpr void log_impl(spdlog::source_loc loc, spdlog::level::level_enum level, T && first, Rest &&... rest) {
    if constexpr (std::is_same_v<std::decay_t<T>, LogType>) {
        if (spdlog::should_log(level)) {
            if constexpr (sizeof...(Rest) > 0) {
                spdlog::log(loc, level, "[{}] {}", first, fmt::format(std::forward<Rest>(rest)...));
            } else {
                spdlog::log(loc, level, "[{}]", first);
            }
        }
    } else {
        if (spdlog::should_log(level)) {
            spdlog::log(loc, level, "[{}] {}", LogType::LogAlways,
                        fmt::format(std::forward<T>(first), std::forward<Rest>(rest)...));
        }
    }
}

/**
 * @brief Logs a trace level message.
 * @tparam Args Variadic template parameter for format arguments
 * @param args Format string and arguments to log
 * @param loc Source location information (automatically populated)
 * @details Creates a temporary object that logs the message when constructed.
 *          Supports both direct logging and category-based logging.
 */
template <typename... Args> struct log_trace {
    constexpr log_trace(Args &&... args, spdlog::source_loc loc = current_source_location()) noexcept {
        if constexpr (sizeof...(Args) > 0) {
            log_impl(loc, spdlog::level::trace, std::forward<Args>(args)...);
        }
    }
};

// Deduction guide
template <typename... Args> log_trace(Args &&...) -> log_trace<Args...>;

/**
 * @brief Logs a debug level message.
 * @tparam Args Variadic template parameter for format arguments
 * @param args Format string and arguments to log
 * @param loc Source location information (automatically populated)
 * @details Creates a temporary object that logs the message when constructed.
 *          Supports both direct logging and category-based logging.
 */
template <typename... Args> struct log_debug {
    constexpr log_debug(Args &&... args, spdlog::source_loc loc = current_source_location()) noexcept {
        if constexpr (sizeof...(Args) > 0) {
            log_impl(loc, spdlog::level::debug, std::forward<Args>(args)...);
        }
    }
};

// Deduction guide
template <typename... Args> log_debug(Args &&...) -> log_debug<Args...>;

/**
 * @brief Logs an info level message.
 * @tparam Args Variadic template parameter for format arguments
 * @param args Format string and arguments to log
 * @param loc Source location information (automatically populated)
 * @details Creates a temporary object that logs the message when constructed.
 *          Supports both direct logging and category-based logging.
 */
template <typename... Args> struct log_info {
    constexpr log_info(Args &&... args, spdlog::source_loc loc = current_source_location()) noexcept {
        if constexpr (sizeof...(Args) > 0) {
            log_impl(loc, spdlog::level::info, std::forward<Args>(args)...);
        }
    }
};

// Deduction guide
template <typename... Args> log_info(Args &&...) -> log_info<Args...>;

/**
 * @brief Logs a warning level message.
 * @tparam Args Variadic template parameter for format arguments
 * @param args Format string and arguments to log
 * @param loc Source location information (automatically populated)
 * @details Creates a temporary object that logs the message when constructed.
 *          Supports both direct logging and category-based logging.
 */
template <typename... Args> struct log_warning {
    constexpr log_warning(Args &&... args, spdlog::source_loc loc = current_source_location()) noexcept {
        if constexpr (sizeof...(Args) > 0) {
            log_impl(loc, spdlog::level::warn, std::forward<Args>(args)...);
        }
    }
};

// Deduction guide
template <typename... Args> log_warning(Args &&...) -> log_warning<Args...>;

/**
 * @brief Logs an error level message.
 * @tparam Args Variadic template parameter for format arguments
 * @param args Format string and arguments to log
 * @param loc Source location information (automatically populated)
 * @details Creates a temporary object that logs the message when constructed.
 *          Supports both direct logging and category-based logging.
 */
template <typename... Args> struct log_error {
    constexpr log_error(Args &&... args, spdlog::source_loc loc = current_source_location()) noexcept {
        if constexpr (sizeof...(Args) > 0) {
            log_impl(loc, spdlog::level::err, std::forward<Args>(args)...);
        }
    }
};

// Deduction guide
template <typename... Args> log_error(Args &&...) -> log_error<Args...>;

/**
 * @brief Logs a critical level message.
 * @tparam Args Variadic template parameter for format arguments
 * @param args Format string and arguments to log
 * @param loc Source location information (automatically populated)
 * @details Creates a temporary object that logs the message when constructed.
 *          Supports both direct logging and category-based logging.
 */
template <typename... Args> struct log_critical {
    constexpr log_critical(Args &&... args, spdlog::source_loc loc = current_source_location()) noexcept {
        if constexpr (sizeof...(Args) > 0) {
            log_impl(loc, spdlog::level::critical, std::forward<Args>(args)...);
        }
    }
};

// Deduction guide
template <typename... Args> log_critical(Args &&...) -> log_critical<Args...>;

/**
 * @brief Logs a fatal level message.
 * @tparam Args Variadic template parameter for format arguments
 * @param args Format string and arguments to log
 * @param loc Source location information (automatically populated)
 * @details Creates a temporary object that logs the message when constructed.
 *          Supports both direct logging and category-based logging.
 *          Note: Currently uses critical level internally.
 */
template <typename... Args> struct log_fatal {
    constexpr log_fatal(Args &&... args, spdlog::source_loc loc = current_source_location()) noexcept {
        if constexpr (sizeof...(Args) > 0) {
            log_impl(loc, spdlog::level::critical, std::forward<Args>(args)...);
        }
    }
};

// Deduction guide
template <typename... Args> log_fatal(Args &&...) -> log_fatal<Args...>;

}  // namespace tt

/**
 * @brief Custom formatter for tt::LogType to enable formatting in fmt library.
 * @details Specializes fmt::formatter to allow LogType values to be formatted directly
 *          in format strings using the string representation of the log type.
 */
namespace fmt {
template <> struct formatter<tt::LogType> : fmt::formatter<std::string_view> {
    template <typename FormatContext> constexpr auto format(tt::LogType logtype, FormatContext & ctx) const {
        return fmt::formatter<std::string_view>::format(tt::logtype_to_string(logtype), ctx);
    }
};
}  // namespace fmt
#undef TT_LOGGER_TYPES
