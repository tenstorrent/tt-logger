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

struct log_impl {
    template <typename... Args>
    log_impl(const spdlog::source_loc & loc, spdlog::level::level_enum level, tt::LogType type,
             fmt::format_string<Args...> fmt, Args &&... args) {
        if (spdlog::should_log(level)) {
            spdlog::log(loc, level, "[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
        }
    }

    template <typename... Args>
    log_impl(const spdlog::source_loc & loc, spdlog::level::level_enum level, fmt::format_string<Args...> fmt,
             Args &&... args) :
        log_impl(loc, level, tt::LogType::LogAlways, fmt, std::forward<Args>(args)...) {}
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

#define log_trace(...) \
    tt::log_impl(spdlog::source_loc{ __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::trace, __VA_ARGS__)
#define log_debug(...) \
    tt::log_impl(spdlog::source_loc{ __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::debug, __VA_ARGS__)
#define log_info(...) \
    tt::log_impl(spdlog::source_loc{ __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::info, __VA_ARGS__)
#define log_warning(...) \
    tt::log_impl(spdlog::source_loc{ __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::warn, __VA_ARGS__)
#define log_error(...) \
    tt::log_impl(spdlog::source_loc{ __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::err, __VA_ARGS__)
#define log_critical(...) \
    tt::log_impl(spdlog::source_loc{ __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::critical, __VA_ARGS__)
#define log_fatal(...) \
    tt::log_impl(spdlog::source_loc{ __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::critical, __VA_ARGS__)
