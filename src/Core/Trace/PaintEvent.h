#pragma once

#include <cstdint>

#include "Library/Platform/Interface/PlatformEvents.h"


inline constexpr PlatformEventType EVENT_PAINT = static_cast<PlatformEventType>(std::to_underlying(EVENT_LAST) + 1);

class PaintEvent : public PlatformEvent {
 public:
    /** Tick count for the next frame. */
    int64_t tickCount = -1; //

    /** Random state at the start of the next frame, as returned by `grng->peek(1024)`. */
    int randomState = -1;
};
