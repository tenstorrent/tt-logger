// SPDX-FileCopyrightText: © 2025 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#define LOGGER_TYPES \
    X(Always)        \
    X(Test)          \
    X(Timer)         \
    X(Device)        \
    X(Model)         \
    X(LLRuntime)     \
    X(Loader)        \
    X(IO)            \
    X(CompileTrisc)  \
    X(BuildKernels)  \
    X(Verif)         \
    X(Golden)        \
    X(Op)            \
    X(HLK)           \
    X(HLKC)          \
    X(Reportify)     \
    X(GraphCompiler) \
    X(Dispatch)      \
    X(Fabric)        \
    X(Metal)         \
    X(MetalTrace)    \
    X(SiliconDriver) \
    X(EmulationDriver)

namespace tt {

// clang-format off
enum LogType {
#define X(a) Log ## a,
    LOGGER_TYPES
#undef X
};
// clang-format on

constexpr std::array<const char *, std::size_t(LogType::LogEmulationDriver) + 1> log_type_names = {
#define X(name) #name,
    LOGGER_TYPES
#undef X
};

constexpr const char * logtype_to_string(LogType logtype) noexcept {
    return static_cast<std::size_t>(logtype) < log_type_names.size() ?
               log_type_names[static_cast<std::size_t>(logtype)] :
               "UnknownType";
}

// log_trace
template <typename... Args> inline void log_trace(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args> inline void log_trace(fmt::format_string<Args...> fmt, Args &&... args) {
    log_trace(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

// log_debug
template <typename... Args> inline void log_debug(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    if (spdlog::should_log(spdlog::level::debug)) {
        spdlog::debug("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args> inline void log_debug(fmt::format_string<Args...> fmt, Args &&... args) {
    log_debug(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

// log_info
template <typename... Args> inline void log_info(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::info("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args> inline void log_info(fmt::format_string<Args...> fmt, Args &&... args) {
    log_info(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

// log_warning
template <typename... Args> inline void log_warning(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::warn("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args> inline void log_warning(fmt::format_string<Args...> fmt, Args &&... args) {
    log_warning(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

// log_critical
template <typename... Args> inline void log_critical(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::critical("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args> inline void log_critical(fmt::format_string<Args...> fmt, Args &&... args) {
    log_critical(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

// log_fatal
template <typename... Args> inline void log_fatal(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::critical("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

// Without LogType (defaults to LogAlways)
template <typename... Args> inline void log_fatal(fmt::format_string<Args...> fmt, Args &&... args) {
    log_fatal(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

// log_error
template <typename... Args> inline void log_error(LogType type, fmt::format_string<Args...> fmt, Args &&... args) {
    spdlog::error("[{}] {}", type, fmt::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args> inline void log_error(fmt::format_string<Args...> fmt, Args &&... args) {
    log_error(LogType::LogAlways, fmt, std::forward<Args>(args)...);
}

}  // namespace tt

// Custom formatter for LogType
namespace fmt {
template <> struct formatter<tt::LogType> : fmt::formatter<std::string_view> {
    template <typename FormatContext> constexpr auto format(tt::LogType logtype, FormatContext & ctx) const {
        return fmt::formatter<std::string_view>::format(tt::logtype_to_string(logtype), ctx);
    }
};
}  // namespace fmt
#undef LOGGER_TYPES
