#pragma once

#include "LogEnums.h"

/**
 * When integrating an external log source into the logging framework, you would usually need:
 * 1. To forward external log messages into `Logger`.
 * 2. To control the log level of the external log source.
 *
 * This class is an abstraction for #2.
 */
class LogSource {
 public:
    virtual ~LogSource() = default;

    [[nodiscard]] virtual LogLevel level() const = 0;
    virtual void setLevel(LogLevel level) = 0;
};
