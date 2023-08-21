#pragma once

#include <utility>

#include "Platform/PlatformLogger.h"
#include "Utility/Format.h"

class Logger {
 public:
    // TODO(captainurist): this should go to ctor, but that's not doable right now because of a shitload of static
    // variables that call EngineIoc::ResolveLogger.
    PlatformLogger *baseLogger() const;
    void setBaseLogger(PlatformLogger *baseLogger);

    bool shouldLog(PlatformLogLevel logLevel) const {
        return _baseLogger == nullptr || _baseLogger->logLevel(APPLICATION_LOG) <= logLevel;
    }

    template<class... Args>
    void log(PlatformLogLevel logLevel, fmt::format_string<Args...> fmt, Args&&... args) {
        if (shouldLog(logLevel))
            logV(logLevel, fmt, fmt::make_format_args(std::forward<Args>(args)...));
    }

    template<class... Args>
    void verbose(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LOG_VERBOSE, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LOG_DEBUG, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LOG_INFO, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void warning(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LOG_WARNING, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LOG_ERROR, fmt, std::forward<Args>(args)...);
    }

    template<class... Args>
    void critical(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LOG_CRITICAL, fmt, std::forward<Args>(args)...);
    }

 private:
    void logV(PlatformLogLevel logLevel, fmt::string_view fmt, fmt::format_args args);

 private:
    PlatformLogger *_baseLogger = nullptr;
};
