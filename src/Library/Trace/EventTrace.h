#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <string>

#include "Platform/PlatformEvents.h"

#include "Utility/Geometry/Vec.h"

struct EventTraceConfigLine {
    std::string section;
    std::string key;
    std::string value;
};

struct EventTraceGameState {
    std::string locationName;
    Vec3i partyPosition;
};

struct EventTraceHeader {
    int saveFileSize = -1;
    std::vector<EventTraceConfigLine> config;
    EventTraceGameState startState;
    EventTraceGameState endState;

    // TODO(captainurist): std::string saveFileChecksum;
};

struct EventTrace {
    static void saveToFile(std::string_view path, const EventTrace &trace);
    static EventTrace loadFromFile(std::string_view path, PlatformWindow *window);

    static bool isTraceable(const PlatformEvent *event);
    static std::unique_ptr<PlatformEvent> cloneEvent(const PlatformEvent *event);

    EventTraceHeader header;
    std::vector<std::unique_ptr<PlatformEvent>> events;
};
