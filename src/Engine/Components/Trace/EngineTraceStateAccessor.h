#pragma once

#include <vector>

#include "Library/Trace/EventTrace.h"

class GameConfig;
struct EventTraceGameState;

struct EngineTraceStateAccessor {
    static void prepareForRecording(GameConfig *config, ConfigPatch *patch);
    static void prepareForPlayback(GameConfig *config, const ConfigPatch &patch);

    static EventTraceGameState makeGameState();
};
