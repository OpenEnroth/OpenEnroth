#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <utility>

#include "Platform/PlatformEvents.h"

#include "Utility/Workaround/ToUnderlying.h"

class PlatformApplication;

// TODO(captainurist): just make this into a POD, and add static methods saveToFile, loadFromFile, cloneEvent.
class EventTrace {
 public:
    // TODO(captainurist): Add current random state to PaintEvent, to debug later. And maybe also current tick count.
    static constexpr PlatformEvent::Type PaintEvent = static_cast<PlatformEvent::Type>(std::to_underlying(PlatformEvent::LastEventType) + 1);

    void saveToFile(std::string_view path) const;
    static EventTrace loadFromFile(std::string_view path, PlatformWindow *window);

    void recordEvent(const PlatformEvent *event);
    void recordRepaint();
    void clear();

    std::vector<std::unique_ptr<PlatformEvent>> takeEvents() {
        return std::move(_events);
    }

 private:
    std::vector<std::unique_ptr<PlatformEvent>> _events;
};

