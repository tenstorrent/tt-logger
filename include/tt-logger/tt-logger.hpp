// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>

#ifdef _WIN32
#    include <io.h>
#    define isatty        _isatty
#    define STDOUT_FILENO 1
#else
#    include <unistd.h>
#endif

#define TT_LOGGER_TYPES \
    X(Always)           \
    X(Test)             \
    X(Timer)            \
    X(Device)           \
    X(Distributed)      \
    X(LLRuntime)        \
    X(Loader)           \
    X(BuildKernels)     \
    X(Verif)            \
    X(Op)               \
    X(Dispatch)         \
    X(Fabric)           \
    X(Metal)            \
    X(TTNN)             \
    X(MetalTrace)       \
    X(Inspector)        \
    X(UMD)              \
    X(EmulationDriver)

namespace tt {

enum LogType {

#define X(name) Log##name,
    TT_LOGGER_TYPES
#undef X
};

constexpr std::array<const char *, std::size_t(LogType::LogEmulationDriver) + 1> log_type_names = {
#define X(name) #name,
    TT_LOGGER_TYPES
#undef X
};

constexpr const char * logtype_to_string(LogType logtype) noexcept {
    return static_cast<std::size_t>(logtype) < log_type_names.size() ?
               log_type_names[static_cast<std::size_t>(logtype)] :
               "UnknownType";
}

class LoggerRegistry {
  private:
    std::array<std::shared_ptr<spdlog::logger>, log_type_names.size()> loggers;

    LoggerRegistry() {
        spdlog::level::level_enum default_level = get_default_log_level();

        // Create sink using the static method
        auto sink = create_sink();

        // Initialize loggers for each LogType
        std::size_t index = 0;
#define X(name)                                                     \
    loggers[index] = std::make_shared<spdlog::logger>(#name, sink); \
    loggers[index].get()->set_level(default_level);                 \
    loggers[index++].get()->flush_on(spdlog::level::critical);
        TT_LOGGER_TYPES
#undef X

        apply_log_type_filtering(default_level);
    }

    LoggerRegistry(const LoggerRegistry &)             = delete;
    LoggerRegistry & operator=(const LoggerRegistry &) = delete;

    static spdlog::level::level_enum get_default_log_level() {
        const char * env_level = std::getenv("TT_LOGGER_LEVEL");
        if (!env_level) {
            env_level = std::getenv("TT_METAL_LOGGER_LEVEL");
        }

        if (env_level) {
            std::string level_str = env_level;
            std::transform(level_str.begin(), level_str.end(), level_str.begin(), ::tolower);

            if (level_str == "trace") {
                return spdlog::level::trace;
            }
            if (level_str == "debug") {
                return spdlog::level::debug;
            }
            if (level_str == "info") {
                return spdlog::level::info;
            }
            if (level_str == "warn") {
                return spdlog::level::warn;
            }
            if (level_str == "error") {
                return spdlog::level::err;
            }
            if (level_str == "critical") {
                return spdlog::level::critical;
            }
            if (level_str == "fatal") {
                return spdlog::level::critical;
            }
            if (level_str == "off") {
                return spdlog::level::off;
            }
        }
        return spdlog::level::info;
    }

    static std::shared_ptr<spdlog::sinks::sink> create_sink() {
        // Define patterns once
        std::string plain_pattern =
            "%Y-%m-%d %H:%M:%S.%e | "  // Timestamp
            "%-8l | "                  // Log level, left-aligned, 8 chars wide
            "%15n | "                  // Logger name, right-aligned, 15 chars wide
            "%v "                      // Message
            "(%s:%#)";                 // Source location

        std::string colored_pattern =
            "\033[90m%Y-%m-%d %H:%M:%S.%e\033[0m | "  // Dark gray timestamp, plain separator
            "%^%-8l%$ | "                             // Auto-colored log level, plain separator
            "\033[35m%15n\033[0m | "                  // Purple logger name, plain separator
            "\033[37m%v\033[0m "                      // White message
            "\033[90m(%s:%#)\033[0m";                 // Dark gray source location

        const char * file_path = std::getenv("TT_LOGGER_FILE");
        if (!file_path) {
            file_path = std::getenv("TT_METAL_LOGGER_FILE");
        }

        if (file_path && strlen(file_path) > 0) {
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path, true);
            if (!sink) {
                std::fprintf(stderr, "tt-logger failed to create log file '%s'\n", file_path);
                std::abort();
            }

            sink->set_pattern(plain_pattern);
            return sink;
        } else {
            auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            // Check if output should be colored:
            // 1. Traditional terminal detection
            // 2. CI environments that support ANSI colors (GitHub Actions, etc.)
            bool is_terminal = isatty(STDOUT_FILENO) != 0;
            bool is_ci_with_colors = std::getenv("GITHUB_ACTIONS") != nullptr || std::getenv("CI") != nullptr ||
                                     std::getenv("CONTINUOUS_INTEGRATION") != nullptr;

            sink->set_pattern((is_terminal || is_ci_with_colors) ? colored_pattern : plain_pattern);

            return sink;
        }
    }

    void apply_log_type_filtering(spdlog::level::level_enum default_level) {
        const char * types_env = std::getenv("TT_LOGGER_TYPES");
        if (!types_env) {
            types_env = std::getenv("TT_METAL_LOGGER_TYPES");
        }

        if (types_env) {
            std::string types_str = types_env;

            if (types_str.find("All") != std::string::npos) {
                // If "All" is specified, keep all loggers enabled
                return;
            } else {
                // Disable all loggers first, then enable only the specified ones
                for (auto & logger : loggers) {
                    logger->set_level(spdlog::level::off);
                }

                // Enable LogAlways by default (always keep this enabled)
                loggers[static_cast<std::size_t>(LogAlways)]->set_level(default_level);

                // Check each log type name and enable if found in the environment variable
                std::size_t type_index = 0;
                for (const char * type_name : log_type_names) {
                    if (types_str.find(type_name) != std::string::npos) {
                        loggers[type_index]->set_level(default_level);
                    }
                    type_index++;
                }
            }
        }
        // If TT_LOGGER_TYPES is not set, keep all loggers enabled (default behavior)
    }

  public:
    static LoggerRegistry & instance() {
        static LoggerRegistry * registry = new LoggerRegistry();
        return *registry;
    }

    std::shared_ptr<spdlog::logger> get(LogType type) { return loggers[static_cast<std::size_t>(type)]; }

    void set_level(spdlog::level::level_enum level) {
        for (auto & logger : loggers) {
            logger->set_level(level);
        }
    }
};

}  // namespace tt

namespace fmt {
template <> struct formatter<tt::LogType> : fmt::formatter<std::string_view> {
    template <typename FormatContext> constexpr auto format(tt::LogType logtype, FormatContext & ctx) const {
        return fmt::formatter<std::string_view>::format(tt::logtype_to_string(logtype), ctx);
    }
};
}  // namespace fmt

// =============================================================================
// Disabled log macro support: consume arguments without evaluation to avoid
// unused variable/parameter warnings when log levels are compile-time disabled.
//
// Uses __VA_OPT__ (C++20) with sizeof() to mark arguments as "used" at compile
// time without any runtime evaluation or side effects.
// =============================================================================

// Detect __VA_OPT__ support:
// - C++20 guarantees __VA_OPT__
// - GCC 8+ and Clang 6+ support it in earlier modes with GNU extensions
#if __cplusplus >= 202002L
#    define TT_LOG_VA_OPT_SUPPORTED 1
#elif defined(__GNUC__) && __GNUC__ >= 8 && !defined(__clang__)
#    define TT_LOG_VA_OPT_SUPPORTED 1
#elif defined(__clang__) && __clang_major__ >= 6
#    define TT_LOG_VA_OPT_SUPPORTED 1
#else
#    define TT_LOG_VA_OPT_SUPPORTED 0
#endif

#if TT_LOG_VA_OPT_SUPPORTED

// Suppress unused warnings without evaluating the expression (compile-time only via sizeof)
#    define TT_LOG_UNUSED(expr) ((void)sizeof(expr))

// FOR_EACH implementation using deferred expansion to work around the preprocessor "blue paint" rule.
// The PARENS trick creates a token sequence that isn't immediately expanded, and EXPAND forces
// multiple rounds of rescanning to eventually expand all iterations.
#    define TT_LOG_PARENS ()

// clang-format off
// Multiple expansion levels to support up to 16 arguments (4^2 = 16 iterations)
#    define TT_LOG_EXPAND(...)  TT_LOG_EXPAND2(TT_LOG_EXPAND2(TT_LOG_EXPAND2(TT_LOG_EXPAND2(__VA_ARGS__))))
#    define TT_LOG_EXPAND2(...) TT_LOG_EXPAND1(TT_LOG_EXPAND1(TT_LOG_EXPAND1(TT_LOG_EXPAND1(__VA_ARGS__))))
#    define TT_LOG_EXPAND1(...) __VA_ARGS__
// clang-format on

// FOR_EACH: Apply TT_LOG_UNUSED to each argument
#    define TT_LOG_FOR_EACH_HELPER(a1, ...) \
        TT_LOG_UNUSED(a1);                  \
        __VA_OPT__(TT_LOG_FOR_EACH_AGAIN TT_LOG_PARENS(__VA_ARGS__))
#    define TT_LOG_FOR_EACH_AGAIN() TT_LOG_FOR_EACH_HELPER

#    define TT_LOG_UNUSED_EACH(...) __VA_OPT__(TT_LOG_EXPAND(TT_LOG_FOR_EACH_HELPER(__VA_ARGS__)))

// Disabled log macro that consumes all arguments without evaluation
#    define TT_LOG_DISABLED(type, ...)       \
        do {                                 \
            TT_LOG_UNUSED(type);             \
            TT_LOG_UNUSED_EACH(__VA_ARGS__); \
        } while (0)

#else

// Fallback for pre-C++20: simple (void)0 - will still produce unused warnings
#    define TT_LOG_DISABLED(type, ...) (void)0

#endif  // TT_LOG_VA_OPT_SUPPORTED

// =============================================================================
// Log macros
// =============================================================================

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
#    define log_trace(type, ...) SPDLOG_LOGGER_TRACE(tt::LoggerRegistry::instance().get(type), __VA_ARGS__)
#else
#    define log_trace(type, ...) TT_LOG_DISABLED(type, __VA_ARGS__)
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
#    define log_debug(type, ...) SPDLOG_LOGGER_DEBUG(tt::LoggerRegistry::instance().get(type), __VA_ARGS__)
#else
#    define log_debug(type, ...) TT_LOG_DISABLED(type, __VA_ARGS__)
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
#    define log_info(type, ...) SPDLOG_LOGGER_INFO(tt::LoggerRegistry::instance().get(type), __VA_ARGS__)
#else
#    define log_info(type, ...) TT_LOG_DISABLED(type, __VA_ARGS__)
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
#    define log_warning(type, ...) SPDLOG_LOGGER_WARN(tt::LoggerRegistry::instance().get(type), __VA_ARGS__)
#else
#    define log_warning(type, ...) TT_LOG_DISABLED(type, __VA_ARGS__)
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
#    define log_error(type, ...) SPDLOG_LOGGER_ERROR(tt::LoggerRegistry::instance().get(type), __VA_ARGS__)
#else
#    define log_error(type, ...) TT_LOG_DISABLED(type, __VA_ARGS__)
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
#    define log_critical(type, ...) SPDLOG_LOGGER_CRITICAL(tt::LoggerRegistry::instance().get(type), __VA_ARGS__)
#else
#    define log_critical(type, ...) TT_LOG_DISABLED(type, __VA_ARGS__)
#endif

// Eventually deprecate log_fatal and use log_critical instead
#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
#    define log_fatal(type, ...) SPDLOG_LOGGER_CRITICAL(tt::LoggerRegistry::instance().get(type), __VA_ARGS__)
#else
#    define log_fatal(type, ...) TT_LOG_DISABLED(type, __VA_ARGS__)
#endif

#undef TT_LOGGER_TYPES
