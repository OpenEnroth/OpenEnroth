#pragma once

#include <vector>
#include <string_view>
#include <memory>

#include "Platform/PlatformEvents.h"

#include "Utility/Workaround/ToUnderlying.h"

struct EventTrace {
    // TODO(captainurist): Add current random state to PaintEvent, to debug later. And maybe also current tick count.
    static constexpr PlatformEvent::Type PaintEvent = static_cast<PlatformEvent::Type>(std::to_underlying(PlatformEvent::LastEventType) + 1);

    static void saveToFile(std::string_view path, const EventTrace &trace);
    static EventTrace loadFromFile(std::string_view path, PlatformWindow *window);
    static std::unique_ptr<PlatformEvent> cloneEvent(const PlatformEvent *event);

    std::vector<std::unique_ptr<PlatformEvent>> events;
};
