#pragma once

#include <string_view>

#include "LogSink.h"

/**
 * Bare-bones log sink that writes to stderr.
 *
 * This is what the fallback `Logger` writes into, so it has to work at any point in the program's life - before the
 * dynamic initialization that builds the serialization tables, and after the static destruction that tears down
 * everything else. That rules out both the singletons that the sink from `LogSink::createDefaultSink` relies on and
 * the terminal coloring it does, so use that one for anything user-facing.
 */
class FallbackLogSink : public LogSink {
 public:
    constexpr FallbackLogSink() = default;

    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) override;
};
