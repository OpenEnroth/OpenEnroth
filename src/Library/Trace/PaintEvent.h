#pragma once

#include <cstdint>

#include "Platform/PlatformEvents.h"

#include "Utility/Workaround/ToUnderlying.h"

inline constexpr PlatformEventType EVENT_PAINT = static_cast<PlatformEventType>(std::to_underlying(EVENT_LAST) + 1);

class PaintEvent : public PlatformEvent {
 public:
    /** Tick count for the next frame. */
    int64_t tickCount = -1; //

    /** Result of `grng->Random(1024)` call from inside `swapBuffers`, basically a random state at the start of the
     * next frame. */
    int randomState = -1;
};
