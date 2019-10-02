#pragma once

#include <string>

#include "Engine/Graphics/IRenderFactory.h"


#define GAME_FLAGS_1_01                         0x01     // do not render terrain / bmodels(odm), lightmaps render bmodels in white(blv)
#define GAME_FLAGS_1_02                         0x02     // only 0th lod, no lightning
#define GAME_FLAGS_1_04                         0x04     // draw lightmaps / decals debug outlines
#define GAME_FLAGS_1_DRAW_BLV_DEBUGS            0x08     // draw portal debug outlines
#define GAME_FLAGS_1_40                         0x40     //
#define GAME_FLAGS_1_FORCE_LEGACY_PROJECTION    0x80     // force camera / culling / projection mode : BLV
#define GAME_FLAGS_1_400                        0x400    //
#define GAME_FLAGS_1_800                        0x800    //
#define GAME_FLAGS_1_TORCHLIGHT                 0x1000   // do Torchlight lightning
#define GAME_FLAGS_1_40000_sw_alter_palettes    0x40000  // software render alter palettes
#define GAME_FLAGS_1_HUGE_BLOODSPLATS           0x80000  // huge bloodsplats on bosses

#define GAME_FLAGS_2_FORCE_REDRAW               0x01     // force redraw this frame
#define GAME_FLAGS_2_SATURATE_LIGHTMAPS         0x02     // alter lightmaps saturation
#define GAME_FLAGS_2_ALLOW_DYNAMIC_BRIGHTNESS   0x04     // allows dynamic brightness calculation on lightmaps
#define GAME_FLAGS_2_ALTER_GRAVITY              0x08     // underwater effect, no gravity (for out15.odm - Shoals)
#define GAME_FLAGS_2_TARGETING_MODE             0x10     // spell targeting, current cursor is "MICON2" (crosshair)
#define GAME_FLAGS_2_DRAW_BLOODSPLATS           0x20

#define DEBUG_SETTINGS_RUN_IN_WIDOW     0x0001
#define DEBUG_SETTINGS_0002_SHOW_FR     0x0002
#define DEBUG_SETTINGS_NO_ACTORS        0x0004
#define DEBUG_SETTINGS_NO_DECORATIONS   0x0008
#define DEBUG_SETTINGS_NO_DAMAGE        0x0010

namespace Engine_ {

class EngineConfig {
 public:
    inline bool IsTargetingMode() const { return is_targeting; }
    inline void SetTargetingMode(bool is_targeting) { this->is_targeting = is_targeting; }
    inline bool IsUnderwater() const { return is_underwater; }
    inline void SetUnderwater(bool is_underwater) { this->is_underwater = is_underwater; }

    inline bool NoVideo() const { return no_video; }
    inline bool NoSound() const { return no_sound; }
    inline bool NoMargareth() const { return no_margareth; }
    inline bool NoShowDamage() const { return !show_damage; }
    inline bool NoWalkSound() const { return no_walk_sound; }
    inline bool NoBloodsplats() const { return no_bloodsplats; }
    inline bool NoActors() const { return no_actors; }
    inline bool NoDecorations() const { return no_decorations; }
    inline bool NoDamage() const { return no_damage; }

    inline void ToggleShowDamage() { show_damage = !show_damage; }
    inline void ToggleAlwaysRun() { always_run = !always_run; }
    inline void ToggleFlipOnExit() { flip_on_exit = !flip_on_exit; }
    inline void ToggleWalkSound() { no_walk_sound = !no_walk_sound; }
    inline void ToggleBloodsplats() { no_bloodsplats = !no_bloodsplats; }

    inline bool Flag1_1() const { return flags1 & GAME_FLAGS_1_01; }
    inline bool Flag1_2() const { return flags1 & GAME_FLAGS_1_02; }
    inline bool ForceLegacyProjection() const { return flags1 & GAME_FLAGS_1_FORCE_LEGACY_PROJECTION; }
    inline bool NoHugeBloodsplats() const { return flags1 & GAME_FLAGS_1_HUGE_BLOODSPLATS; }
    inline bool AlterPalettes() const { return flags1 & GAME_FLAGS_1_40000_sw_alter_palettes; }
    inline bool DrawBlvDebugs() const { return flags1 & GAME_FLAGS_1_DRAW_BLV_DEBUGS; }
    inline bool TorchlightEffect() const { return flags1 & GAME_FLAGS_1_TORCHLIGHT; }
    inline bool CanSaturateFaces() const { return flags2 & GAME_FLAGS_2_SATURATE_LIGHTMAPS; }
           void SetSaturateFaces(bool saturate);
    inline bool ForceRedraw() const { return flags2 & GAME_FLAGS_2_FORCE_REDRAW; }
           void SetForceRedraw(bool redraw);
    inline bool AllowDynamicBrigtness() const { return flags2 & GAME_FLAGS_2_ALLOW_DYNAMIC_BRIGHTNESS; }
    inline bool RunInWindow() const { return dword_6BE368_debug_settings_2 & DEBUG_SETTINGS_RUN_IN_WIDOW; }

    inline void ToggleDebugTownPortal() { debug_town_portal = !debug_town_portal; }
    inline void ToggleDebugWizardEye() { debug_wizard_eye = !debug_wizard_eye; }
    inline void ToggleDebugAllMagic() { debug_all_magic = !debug_all_magic; }
    inline void ToggleDebugTerrain() { debug_terrain = !debug_terrain; }
    inline void ToggleDebugLightmap() { debug_lightmaps_decals = !debug_lightmaps_decals; }
    inline void ToggleDebugTurboSpeed() { debug_turbo_speed = !debug_turbo_speed; }
    inline void ToggleDebugNoActors() { no_actors = !no_actors; }
    inline void ToggleDebugDrawDist() { extended_draw_distance = !extended_draw_distance; }
    inline void ToggleDebugSnow() { allow_snow = !allow_snow; }
    inline void ToggleDebugNoDamage() { no_damage = !no_damage; }
    inline void ToggleDebugPortalLines() { debug_portal_outlines = !debug_portal_outlines; }
    inline void ToggleDebugPickedFace() { show_picked_face = !show_picked_face; }
    inline void ToggleDebugShowFPS() { show_fps = !show_fps; }
    inline void ToggleDebugSeasonsChange() { seasons_change = !seasons_change; }

    // DEBUG_SETTINGS_*
    int dword_6BE368_debug_settings_2 = DEBUG_SETTINGS_RUN_IN_WIDOW;

    // GAME_FLAGS_1_*
    int flags1 = GAME_FLAGS_1_40 | GAME_FLAGS_1_800;

    // GAME_FLAGS_2_*
    int flags2 = GAME_FLAGS_2_ALLOW_DYNAMIC_BRIGHTNESS | GAME_FLAGS_2_DRAW_BLOODSPLATS | GAME_FLAGS_2_SATURATE_LIGHTMAPS;

    int turn_speed = 0;             // discrete turn speed, 0 for smooth
    float turn_speed_smooth = 1.0f;

    int sound_level = 4;    // 0..9
    int voice_level = 5;    // 0..9
    int music_level = 3;    // 0..9

    int gamma = 4;
    int max_flight_height = 4000;

    bool no_intro = false;
    bool no_logo = false;

    bool no_sound = false;
    bool no_video = false;
    bool no_walk_sound = false;
    bool no_margareth = false;
    bool no_bloodsplats = false;
    bool no_actors = false;
    bool no_decorations = false;
    bool no_damage = false;
    bool seasons_change = true;             // change sprites and tiles according to season
    bool allow_lightmaps = true;
    bool allow_snow = false;
    bool extended_draw_distance = true;    // 2.5x draw distance
    bool show_fps = true;
    bool show_picked_face = true;           // red flash face pointed by mouse
    bool debug_all_magic = false;            // toggle all spellbook
    bool debug_wizard_eye = false;           // wizard eye always on
    bool debug_portal_outlines = false;     // draw portal frames
    bool debug_turbo_speed = false;          // party movement 12x
    bool debug_lightmaps_decals = false;    // debug lightmap and decals outlines
    bool debug_terrain = false;
    bool debug_town_portal = true;

    bool always_run = true;
    bool show_damage = true;
    bool flip_on_exit = false;

    bool is_underwater = false;
    bool is_targeting = false;

    std::string renderer_name = "DirectDraw";
};

}  // namespace Engine_
