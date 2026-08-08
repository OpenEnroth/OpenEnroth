#pragma once

#include <string_view>

#include "LogSink.h"

/**
 * Log sink that writes to stderr.
 *
 * Unlike the sink returned by `LogSink::createDefaultSink`, this one is constant-initializable, which is what makes
 * the fallback `Logger` possible.
 */
class StderrLogSink : public LogSink {
 public:
    constexpr StderrLogSink() = default;

    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) override;
};
