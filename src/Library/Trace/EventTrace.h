#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <string>

#include "Library/Platform/Interface/PlatformEvents.h"

#include "Utility/Geometry/Vec.h"

// TODO(captainurist): this should go to Core/, not Library/,

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
    /** Size of the save file associated with the trace. */
    int saveFileSize = -1;

    /** Config diff relative to the default config. Applied before loading the save associated with the trace. */
    std::vector<EventTraceConfigLine> config;

    /** Game state at the start of the trace, right after the save is loaded. */
    EventTraceGameState startState;

    /** Game state at the end of the trace. */
    EventTraceGameState endState;

    /** Random state after loading the save file associated with the trace, as returned by `grng->peek(1024)`. */
    int afterLoadRandomState = -1;

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
