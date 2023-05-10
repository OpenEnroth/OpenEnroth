#pragma once

#include <vector>

class GameConfig;
struct EventTraceConfigLine;
struct EventTraceGameState;

struct EngineTraceStateAccessor {
    static std::vector<EventTraceConfigLine> makeConfigPatch(const GameConfig *config);
    static void patchConfig(GameConfig *config, const std::vector<EventTraceConfigLine>& patch);

    static EventTraceGameState makeGameState();
};
