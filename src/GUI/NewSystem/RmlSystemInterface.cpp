#include "RmlSystemInterface.h"

#include "Library/Logger/Logger.h"

static LogCategory uiSystemLogCategory("UI SYSTEM");

LogLevel convertRmlLogTypeToLogLevel(Rml::Log::Type type) {
    switch (type) {
    case Rml::Log::Type::LT_ERROR:
    case Rml::Log::Type::LT_ASSERT: {
        return LogLevel::LOG_ERROR;
    }
    case Rml::Log::Type::LT_WARNING: {
        return LogLevel::LOG_WARNING;
    }
    case Rml::Log::Type::LT_INFO: {
        return LogLevel::LOG_INFO;
    }
    case Rml::Log::Type::LT_DEBUG: {
        return LogLevel::LOG_DEBUG;
    }
    }
    return LogLevel::LOG_DEBUG;
}

RmlSystemInterface::RmlSystemInterface() {
}

double RmlSystemInterface::GetElapsedTime() {
    return 0.0;
}

bool RmlSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String &message) {
    logger->log(uiSystemLogCategory, convertRmlLogTypeToLogLevel(type), message);
    return true;
}
