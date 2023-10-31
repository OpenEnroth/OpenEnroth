#pragma once

#include <string_view>
#include <memory>

#include "LogCategory.h"
#include "LogEnums.h"

class LogSink {
 public:
    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) = 0;

    /**
     * @return                          Default sink for the current platform. Returned sink is thread-safe.
     */
    static std::unique_ptr<LogSink> createDefaultSink();
};
