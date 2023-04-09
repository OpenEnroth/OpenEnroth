#pragma once

#include <vector>

class GameConfig;
struct EventTraceConfigLine;

struct EngineTraceConfigurator {
    static std::vector<EventTraceConfigLine> makeConfigPatch(const GameConfig *config);
    static void patchConfig(GameConfig *config, const std::vector<EventTraceConfigLine>& patch);
};
