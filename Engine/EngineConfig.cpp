#include "Engine/EngineConfig.h"

using Engine_::EngineConfig;

EngineConfig *engine_config = nullptr;


void EngineConfig::SetSaturateFaces(bool saturate) {
    if (saturate)
        flags2 |= GAME_FLAGS_2_SATURATE_LIGHTMAPS;
    else
        flags2 &= ~GAME_FLAGS_2_SATURATE_LIGHTMAPS;
}

void EngineConfig::SetForceRedraw(bool redraw) {
    if (redraw)
        flags2 |= GAME_FLAGS_2_FORCE_REDRAW;
    else
        flags2 &= ~GAME_FLAGS_2_FORCE_REDRAW;
}
