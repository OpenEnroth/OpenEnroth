#pragma once

#include <vector>

#include "Library/Trace/EventTrace.h"

class GameConfig;
struct EventTraceGameState;

struct EngineTraceStateAccessor {
    static void prepareForPlayback(GameConfig *config);

    static std::vector<EventTraceConfigLine> makeConfigPatch(const GameConfig *config);
    static void patchConfig(GameConfig *config, const std::vector<EventTraceConfigLine>& patch);

    static EventTraceGameState makeGameState();
};
