#pragma once

#include <string_view>

#include "LogSink.h"

/**
 * Bare-bones log sink that writes to stderr.
 *
 * This is what the fallback `Logger` writes into, so it has to work before the dynamic initialization that builds the
 * serialization tables.
 */
class FallbackLogSink : public LogSink {
 public:
    constexpr FallbackLogSink() = default;

    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) override;
};
