#include "FallbackLogSink.h"

#include <cstdio>
#include <iterator>

#include "Utility/String/Format.h"

#include "LogCategory.h"

void FallbackLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    // `memory_buffer` formats into inline storage, so a log line of a sane length doesn't allocate. Level names go
    // through `logLevelName` and not through `toString` because this sink also runs before the serialization tables
    // are initialized.
    fmt::memory_buffer line;
    if (category.name().empty()) {
        fmt::format_to(std::back_inserter(line), "[{}] {}\n", logLevelName(level), message);
    } else {
        fmt::format_to(std::back_inserter(line), "[{}] [{}] {}\n", category.name(), logLevelName(level), message);
    }

    // A single `fwrite` keeps lines from interleaving.
    std::fwrite(line.data(), 1, line.size(), stderr);
}
