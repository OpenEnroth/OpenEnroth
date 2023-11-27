#pragma once

#include <vector>

#include "Library/Trace/EventTrace.h"

class GameConfig;
struct EventTraceGameState;

struct EngineTraceStateAccessor {
    static std::vector<EventTraceConfigLine> makeConfigPatch(const GameConfig *config);
    static void prepareForPlayback(GameConfig *config, const std::vector<EventTraceConfigLine>& withPatch);

    static EventTraceGameState makeGameState();
};
