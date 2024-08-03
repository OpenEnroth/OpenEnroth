#pragma once

#include <string_view>
#include <utility>
#include <mutex>

#include "Utility/String/Format.h"

#include "LogCategory.h"
#include "LogEnums.h"

class LogSink;

/**
 * Main logging class.
 *
 * `Logger` is a singleton, but the user is supposed to create a `Logger` instance himself before using it. This would
 * usually be done in the first few lines of `main`.
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
        return level >= (category._level ? *category._level : *_defaultCategory._level);
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
        return level >= *_defaultCategory._level;
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
    void logV(const LogCategory &category, LogLevel level, fmt::string_view fmt, fmt::format_args args);

 private:
    std::mutex _mutex;
    LogCategory _defaultCategory = LogCategory({});
    LogSink *_sink = nullptr;
};

extern Logger *logger; // Singleton logger instance.

