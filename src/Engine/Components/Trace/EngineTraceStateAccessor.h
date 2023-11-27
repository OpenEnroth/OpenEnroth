#pragma once

#include <vector>

#include "Library/Trace/EventTrace.h"

class GameConfig;
struct EventTraceGameState;

struct EngineTraceStateAccessor {
    static void prepareForRecording(GameConfig *config, std::vector<EventTraceConfigLine> *patch);
    static void prepareForPlayback(GameConfig *config, const std::vector<EventTraceConfigLine> &patch);

    static EventTraceGameState makeGameState();
};
