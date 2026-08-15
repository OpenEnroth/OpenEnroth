#include "FallbackLogSink.h"

#include <cstdio>
#include <string>

#include "Utility/String/Format.h"

void FallbackLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    std::string levelName;
    serialize(level, &levelName);

    // A single `fwrite` keeps lines from interleaving.
    std::string line = category.name().empty()
        ? fmt::format("[{}] {}\n", levelName, message)
        : fmt::format("[{}] [{}] {}\n", category.name(), levelName, message);
    std::fwrite(line.data(), 1, line.size(), stderr);
}
