#pragma once

#include <vector>
#include <string_view>
#include <memory>

#include "Platform/PlatformEvents.h"

struct EventTrace {
    /**
     * 16ms translates to 62.5fps.
     *
     * It is possible to target 60fps and use double here (or store microseconds / nanoseconds), but since the platform
     * API is in milliseconds, this will result in staggered frame times, with every 1st and 2nd frame taking 17ms,
     * and every 3rd one taking 16ms. This might result in some non-determinism down the line, e.g. changing the code
     * in level loading will change the number of frames it takes to load a level, and this will shift the timing
     * sequence for the actual game frames after the level is loaded. Unlikely to really affect anything, but we'd
     * rather not find out.
     */
    static constexpr int64_t FRAME_TIME_MS = 16;

    static void saveToFile(std::string_view path, const EventTrace &trace);
    static EventTrace loadFromFile(std::string_view path, PlatformWindow *window);
    static std::unique_ptr<PlatformEvent> cloneEvent(const PlatformEvent *event);

    std::vector<std::unique_ptr<PlatformEvent>> events;
};
