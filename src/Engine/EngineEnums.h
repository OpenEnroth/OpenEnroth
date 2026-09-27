#pragma once

#include <cstdint>

#include "Utility/Flags.h"

/**
 * Engine and application state bits, see `engineFlags`. Values are the bits of the same flag word at 0x6BE364 in
 * vanilla MM7.
 */
enum class EngineFlag : uint32_t {
    ENGINE_SKIP_NEXT_WORLD_UPDATE = 0x0001,               // Set on map change and on load, the next frame skips actor AI and input.
    ENGINE_DESKTOP_NOT_16BPP = 0x0002,                    // Vanilla MM7 refuses windowed mode then. Unused in OE.
    ENGINE_NO_MOVIES = 0x0004,                            // Vanilla MM7 nointro ini key, skips all fullscreen movies. Unused in OE.
    ENGINE_NO_LOGO = 0x0008,                              // Vanilla MM7 nologo ini key, never read. Unused in OE.
    ENGINE_NO_SOUND = 0x0010,                             // Vanilla MM7 -nosound switch and nosound ini key. Unused in OE.
    ENGINE_NO_WALK_SOUND = 0x0020,                        // Vanilla MM7 nowalksound ini key, never read. Unused in OE.
    ENGINE_NO_ANIM = 0x0040,                              // Vanilla MM7 -noanim switch, skips all fullscreen movies. Unused in OE.
    ENGINE_SKIP_NEXT_USER_INPUT = 0x0080,                 // Set after a level is prepared, the next input pass is dropped.
    ENGINE_APP_INACTIVE = 0x0100,                         // The window lost focus.
    ENGINE_GAME_TIMER_PAUSED_BEFORE_DEACTIVATE = 0x0200,  // Don't resume the game timer when focus comes back.
    ENGINE_ANIM_TIMER_PAUSED_BEFORE_DEACTIVATE = 0x0400,  // Don't resume the animation timer when focus comes back.
    ENGINE_GAME_TIMER_PAUSED_BEFORE_MODE_SWITCH = 0x0800, // The same around a fullscreen switch in vanilla MM7. Unused in OE.
    ENGINE_ANIM_TIMER_PAUSED_BEFORE_MODE_SWITCH = 0x1000, // The same around a fullscreen switch in vanilla MM7. Unused in OE.
    ENGINE_LOADING_SAVEGAME = 0x2000,                     // The map being loaded comes from a save, don't respawn or reinitialize it.
    ENGINE_ESCAPE_ENABLED = 0x4000,                       // Escape can leave party creation. Vanilla MM7 also gates skipping movies on it.
};
using enum EngineFlag;
MM_DECLARE_FLAGS(EngineFlags, EngineFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(EngineFlags)
