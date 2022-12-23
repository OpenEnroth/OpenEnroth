#include "SdlLogger.h"

#include <SDL_log.h>

#include "SdlEnumTranslation.h"

void SdlLogger::SetLogLevel(PlatformLogCategory category, PlatformLogLevel logLevel) {
    if (category == ApplicationLog) {
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, TranslatePlatformLogLevel(logLevel));
    } else {
        assert(category == PlatformLog);
        SDL_LogPriority applicationPriority = SDL_LogGetPriority(SDL_LOG_CATEGORY_APPLICATION);

        SDL_LogSetAllPriority(TranslatePlatformLogLevel(logLevel));
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, applicationPriority);
    }
}

PlatformLogLevel SdlLogger::LogLevel(PlatformLogCategory category) const {
    if (category == ApplicationLog) {
        return TranslateSdlLogLevel(SDL_LogGetPriority(SDL_LOG_CATEGORY_APPLICATION));
    } else {
        assert(category == PlatformLog);

        // SDL_LOG_CATEGORY_CUSTOM here is as good as any other category.
        return TranslateSdlLogLevel(SDL_LogGetPriority(SDL_LOG_CATEGORY_CUSTOM));
    }
}

void SdlLogger::Log(PlatformLogLevel logLevel, const char* message) {
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, TranslatePlatformLogLevel(logLevel), "%s", message);
}

void SdlLogger::LogSdlError(const char *sdlFunctionName) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL error in %s: %s.", sdlFunctionName, SDL_GetError());
}
