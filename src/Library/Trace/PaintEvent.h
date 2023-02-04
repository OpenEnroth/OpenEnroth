#pragma once

#include <cstdint>

#include "Platform/PlatformEvents.h"

#include "Utility/Workaround/ToUnderlying.h"

class PaintEvent : public PlatformEvent {
 public:
    // TODO(captainurist): Rename properly.
    static constexpr PlatformEvent::Type Paint = static_cast<PlatformEvent::Type>(std::to_underlying(PlatformEvent::LastEventType) + 1);

    /** Tick count for the next frame. */
    int64_t tickCount = -1; //

    /** Result of `grng->Random(1024)` call from inside `swapBuffers`, basically a random state at the start of the
     * next frame. */
    int randomState = -1;
};
