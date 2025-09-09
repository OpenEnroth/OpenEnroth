#pragma once

#include <SDL3/SDL.h>

struct SdlFreeDeleter {
    void operator()(void *memory) const {
        SDL_free(memory);
    }
};
