#pragma once

#include <cstdint>

#include "Platform/PlatformEvents.h"

#include "Utility/Workaround/ToUnderlying.h"

class PaintEvent : public PlatformEvent {
 public:
    // TODO(captainurist): Rename properly.
    static constexpr PlatformEvent::Type Paint = static_cast<PlatformEvent::Type>(std::to_underlying(PlatformEvent::LastEventType) + 1);

    int64_t tickCount = -1; // Tick count after this paint event, mainly for debugging.
    int randomState = -1; // Result of a `Random(1024)` call from inside `SwapBuffers`, mainly for debugging.
};
