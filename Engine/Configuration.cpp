#include "Engine/Configuration.h"

using Engine_::Configuration;

Configuration *engine_config = nullptr;


void Configuration::SetSaturateFaces(bool saturate) {
    if (saturate)
        flags2 |= GAME_FLAGS_2_SATURATE_LIGHTMAPS;
    else
        flags2 &= ~GAME_FLAGS_2_SATURATE_LIGHTMAPS;
}

void Configuration::SetForceRedraw(bool redraw) {
    if (redraw)
        flags2 |= GAME_FLAGS_2_FORCE_REDRAW;
    else
        flags2 &= ~GAME_FLAGS_2_FORCE_REDRAW;
}
