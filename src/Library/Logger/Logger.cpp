#include "Logger.h"

#include <cassert>

Logger::Logger(PlatformLogger *baseLogger): _baseLogger(baseLogger) {
    assert(baseLogger);
}

void Logger::logV(PlatformLogLevel logLevel, fmt::string_view fmt, fmt::format_args args) {
    _baseLogger->log(APPLICATION_LOG, logLevel, fmt::vformat(fmt, args).c_str());
}
