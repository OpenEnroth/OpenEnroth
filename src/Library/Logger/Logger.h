#pragma once

#include <string_view>
#include <utility>
#include <mutex>

#include "Utility/String/Format.h"

#include "LogCategory.h"
#include "LogEnums.h"

class LogSink;
class Logger;

namespace detail {
constexpr Logger *fallbackLogger();
struct DetachedTag {}; // Tells `Logger`'s constructor not to install the newly created logger as the global one.
inline constexpr DetachedTag detached;
} // namespace detail

/**
 * Main logging class.
 *
 * `Logger` is a singleton, but the user is supposed to create a `Logger` instance himself before using it. This would
 * usually be done in the first few lines of `main`. Logging before that point still works and goes to stderr, so
 * early messages, messages from the code that runs after the user-created logger is gone, and messages from the
 * command line tools that never create a logger are not lost.
 *
 * Some notes on design decisions:
 * 1. `Logger` supports hooking into other logging frameworks, translating the log levels appropriately. Thus, it's
 *    possible to create separate `LogCategory` instances for SDL and FFmpeg logs, and manage log levels through the
 *    `Logger` interface. Setting global log level to `LOG_CRITICAL` will then prevent both SDL and FFmpeg from
 *    emitting any non-critical log messages - the important point being that they will be filtered out at SDL/FFmpeg
 *    level, and not in the `Logger` code.
 * 2. Point (1) above led to settling on making `Logger` a singleton. This is what pretty much all other logging
 *    libraries do, and thus supporting both multiple `Logger` instances AND being able to hook into external logging
 *    frameworks made very little sense.
 * 3. This also made it possible to implement log categories efficiently by storing per-category log levels inside the
 *    `LogCategory` objects.
 * 4. Different logging targets are implemented with the `LogSink` interface. `LogSink` also makes it possible to
 *    implement complex logging logic, i.e. writing all logs starting with `LOG_DEBUG` into a file, but printing only
 *    errors to the console. It's up to the user to properly implement the log level handling in this case.
 */
class Logger {
 public:
    explicit Logger(LogLevel level, LogSink *sink);
    ~Logger();

    // LogCategory API.

    [[nodiscard]] bool shouldLog(const LogCategory &category, LogLevel level) const {
        return static_cast<int>(level) >= (category._adjustedLevel ? *category._adjustedLevel : *_defaultCategory._adjustedLevel);
    }

    template<class... Args>
    void log(const LogCategory &category, LogLevel level, fmt::format_string<Args...> fmt, Args &&... args) {
        if (shouldLog(category, level))
            logV(category, level, fmt, fmt::make_format_args(args...));
    }

    template<class... Args>
    void trace(const LogCategory &category, fmt::format_string<Args...> fmt, Args &&... args) {
        log(category, LOG_TRACE, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void debug(const LogCategory &category, fmt::format_string<Args...> fmt, Args &&... args) {
        log(category, LOG_DEBUG, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void info(const LogCategory &category, fmt::format_string<Args...> fmt, Args &&... args) {
        log(category, LOG_INFO, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void warning(const LogCategory &category, fmt::format_string<Args...> fmt, Args &&... args) {
        log(category, LOG_WARNING, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void error(const LogCategory &category, fmt::format_string<Args...> fmt, Args &&... args) {
        log(category, LOG_ERROR, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void critical(const LogCategory &category, fmt::format_string<Args...> fmt, Args &&... args) {
        log(category, LOG_CRITICAL, fmt, std::forward<Args>(args)...);
    }

    // Default category API.

    [[nodiscard]] bool shouldLog(LogLevel level) const {
        return static_cast<int>(level) >= *_defaultCategory._adjustedLevel;
    }

    template<class... Args>
    void log(LogLevel level, fmt::format_string<Args...> fmt, Args &&... args) {
        log(_defaultCategory, level, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void trace(fmt::format_string<Args...> fmt, Args &&... args) {
        trace(_defaultCategory, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void debug(fmt::format_string<Args...> fmt, Args &&... args) {
        debug(_defaultCategory, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void info(fmt::format_string<Args...> fmt, Args &&... args) {
        info(_defaultCategory, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void warning(fmt::format_string<Args...> fmt, Args&&... args) {
        warning(_defaultCategory, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void error(fmt::format_string<Args...> fmt, Args &&... args) {
        error(_defaultCategory, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void critical(fmt::format_string<Args...> fmt, Args &&... args) {
        critical(_defaultCategory, fmt, std::forward<Args>(args)...);
    }

    // Log level handling. NOT thread-safe.

    [[nodiscard]] LogLevel level() const;
    void setLevel(LogLevel level);

    [[nodiscard]] std::optional<LogLevel> level(const LogCategory &category) const;
    void setLevel(LogCategory &category, std::optional<LogLevel> level);

    // Sink handling. NOT thread-safe.

    [[nodiscard]] LogSink *sink() const;
    void setSink(LogSink *sink);

 private:
    friend constexpr Logger *detail::fallbackLogger();

    /**
     * Fallback logger that writes everything to stderr. The global `logger` points here until the user creates a
     * `Logger` of their own, and points back here once that logger is destroyed.
     */
    static Logger fallbackLogger;

    /**
     * Creates a logger that's detached from the global `logger`, and thus doesn't install itself there. Only
     * `fallbackLogger` uses this, and it's `constexpr` so that `fallbackLogger` can be constant-initialized.
     */
    constexpr Logger(detail::DetachedTag, LogLevel level, LogSink *sink) : _sink(sink) {
        _defaultCategory._level = level;
        _defaultCategory._adjustedLevel = LogCategory::adjustLevel(level);
    }

    void logV(const LogCategory &category, LogLevel level, fmt::string_view fmt, fmt::format_args args);

 private:
    std::mutex _mutex;
    LogCategory _defaultCategory;
    LogSink *_sink = nullptr;
};

namespace detail {
constexpr Logger *fallbackLogger() {
    return &Logger::fallbackLogger;
}

extern constinit Logger *logger; // Singleton logger instance, never null - use the macros below.
} // namespace detail

/**
 * Logging macros - this is how you log.
 *
 * Message arguments are not evaluated when the message is dropped by the log level check.
 *
 * The `_IN` variants take a `LogCategory`, the ones without it log into the default category.
 *
 * @param LEVEL                         Level to log at.
 * @param ...                           Format string and format arguments.
 */
#define MM_LOG(LEVEL, ...) \
    do { \
        LogLevel localLevel = (LEVEL); \
        if (::detail::logger->shouldLog(localLevel)) \
            ::detail::logger->log(localLevel, __VA_ARGS__); \
    } while (false)

/**
 * Same as `MM_LOG`, but logs into the provided category.
 *
 * @param CATEGORY                      `LogCategory` to log into.
 * @param LEVEL                         Level to log at.
 * @param ...                           Format string and format arguments.
 */
#define MM_LOG_IN(CATEGORY, LEVEL, ...) \
    do { \
        const LogCategory &localCategory = (CATEGORY); \
        LogLevel localLevel = (LEVEL); \
        if (::detail::logger->shouldLog(localCategory, localLevel)) \
            ::detail::logger->log(localCategory, localLevel, __VA_ARGS__); \
    } while (false)

#define MM_TRACE(...) MM_LOG(LOG_TRACE, __VA_ARGS__)
#define MM_DEBUG(...) MM_LOG(LOG_DEBUG, __VA_ARGS__)
#define MM_INFO(...) MM_LOG(LOG_INFO, __VA_ARGS__)
#define MM_WARNING(...) MM_LOG(LOG_WARNING, __VA_ARGS__)
#define MM_ERROR(...) MM_LOG(LOG_ERROR, __VA_ARGS__)
#define MM_CRITICAL(...) MM_LOG(LOG_CRITICAL, __VA_ARGS__)

#define MM_TRACE_IN(CATEGORY, ...) MM_LOG_IN(CATEGORY, LOG_TRACE, __VA_ARGS__)
#define MM_DEBUG_IN(CATEGORY, ...) MM_LOG_IN(CATEGORY, LOG_DEBUG, __VA_ARGS__)
#define MM_INFO_IN(CATEGORY, ...) MM_LOG_IN(CATEGORY, LOG_INFO, __VA_ARGS__)
#define MM_WARNING_IN(CATEGORY, ...) MM_LOG_IN(CATEGORY, LOG_WARNING, __VA_ARGS__)
#define MM_ERROR_IN(CATEGORY, ...) MM_LOG_IN(CATEGORY, LOG_ERROR, __VA_ARGS__)
#define MM_CRITICAL_IN(CATEGORY, ...) MM_LOG_IN(CATEGORY, LOG_CRITICAL, __VA_ARGS__)

