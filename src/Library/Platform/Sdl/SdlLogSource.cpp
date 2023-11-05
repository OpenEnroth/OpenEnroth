#include "SdlLogSource.h"

#include <SDL_log.h>

#include "SdlEnumTranslation.h"

LogLevel SdlLogSource::level() const {
    // `SDL_LOG_CATEGORY_CUSTOM` will query `SDL_default_priority` on a call to `SDL_LogGetPriority`.
    return translateSdlLogLevel(SDL_LogGetPriority(SDL_LOG_CATEGORY_CUSTOM));
}

void SdlLogSource::setLevel(LogLevel logLevel) {
    SDL_LogPriority assertPriority = SDL_LogGetPriority(SDL_LOG_CATEGORY_ASSERT);

    // This sets default log priority. Note that this call is better than going through all the different
    // categories and calling `SDL_LogSetPriority` because it sets a single global variable inside SDL
    // (`SDL_default_priority`). Making several calls would instead allocate a linked list of category-priority
    // pairs which will then be traversed in O(n) every time `SDL_Log` is called.
    SDL_LogSetAllPriority(translatePlatformLogLevel(logLevel));

    // Then we need to roll back assert priority. All SDL asserts are issued at `SDL_LOG_PRIORITY_WARN` which
    // makes little sense, they should really be at `SDL_LOG_PRIORITY_CRITICAL`. See code in `sdlLogCallback`.
    SDL_LogSetPriority(SDL_LOG_CATEGORY_ASSERT, assertPriority);

    // Note that we don't touch SDL_LOG_CATEGORY_APPLICATION because we're not using it.
}
