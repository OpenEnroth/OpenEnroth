#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <utility>

#include "Platform/PlatformEvents.h"

class PlatformApplication;

// TODO(captainurist): just make this into a POD, and add static methods saveToFile, loadFromFile, cloneEvent.
class EventTrace {
 public:
    // TODO(captainurist): Just add User to PlatformEvent::Type, handle it properly everywhere, and don't resort to
    // hacks here. Also, add current random state to PaintEvent, to debug later. And maybe also current tick count.
    static constexpr PlatformEvent::Type PaintEvent = PlatformEvent::Invalid;

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

