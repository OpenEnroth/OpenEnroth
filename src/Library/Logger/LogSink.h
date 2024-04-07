#pragma once

#include <string_view>
#include <memory>

#include "LogCategory.h"
#include "LogEnums.h"

/**
 * Log sink interface.
 *
 * Log level handling & log formatting are done in `Logger`. `LogSink` implementations are expected to just dump the
 * log message & do nothing else.
 *
 * @see Logger::setSink
 */
class LogSink {
 public:
    virtual ~LogSink() = default;

    /**
     * Writes out the log message.
     *
     * Calls into `write` from the `Logger` instance are guaranteed to be serialized with a mutex, so you don't need
     * to do any locking in your implementation.
     *
     * @param category                  Log category.
     * @param level                     Log level.
     * @param message                   Log message.
     */
    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) = 0;

    /**
     * @return                          Default sink for the current platform.
     */
    static std::unique_ptr<LogSink> createDefaultSink();
};
