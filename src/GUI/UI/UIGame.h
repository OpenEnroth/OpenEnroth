#pragma once

#include <memory>
#include <string_view>

#include "GUI/GUIWindow.h"

void GameUI_LoadPlayerPortraitsAndVoices();
void GameUI_ReloadPlayerPortraits(int player_id, int face_id);
void GameUI_WritePointedObjectStatusString();
void GameUI_OnPlayerPortraitLeftClick(int uPlayerID);  // idb
void buttonbox(int x, int y, std::string_view text, int col);
void GameUI_handleHintMessage(UIMessageType type, int param);

// The Controls (options) screen's volume-slider track geometry (screen space) -
// GameMenu.cpp's UIMSG_Change{Sound,Music,Voice}Volume handlers need this to resolve a track
// click to the right level under MM8 (its track sits at different x/width than MM7's), same
// cross-TU sharing convention as UISaveLoad.h's own kMm8SaveLoadVisibleSlotsInGame. Full
// measurement/derivation comment lives with the rest of this screen's layout constants
// (UIGame.cpp, next to kMm8VolumeMarkerX).
inline constexpr int kMm8VolumeTrackX = 363;
inline constexpr int kMm8VolumeTrackWidth = 194;

class GUIWindow_GameMenu : public GUIWindow {
 public:
    GUIWindow_GameMenu();
    virtual ~GUIWindow_GameMenu() {}

    virtual void Update() override;
};

class GUIWindow_GameOptions : public GUIWindow {
 public:
    GUIWindow_GameOptions();
    virtual ~GUIWindow_GameOptions() {}

    virtual void Update() override;
};

class GUIWindow_GameKeyBindings : public GUIWindow {
 public:
    GUIWindow_GameKeyBindings();
    virtual ~GUIWindow_GameKeyBindings() {}

    void Update() override;
};



class GUIWindow_GameVideoOptions : public GUIWindow {
 public:
    GUIWindow_GameVideoOptions();
    virtual ~GUIWindow_GameVideoOptions() {}

    virtual void Update() override;
};

class GraphicsImage;
extern std::shared_ptr<GraphicsImage> game_ui_statusbar;
extern std::shared_ptr<GraphicsImage> game_ui_rightframe;
extern std::shared_ptr<GraphicsImage> game_ui_topframe;
extern std::shared_ptr<GraphicsImage> game_ui_leftframe;
extern std::shared_ptr<GraphicsImage> game_ui_bottomframe;

extern std::shared_ptr<GraphicsImage> game_ui_monster_hp_green;
extern std::shared_ptr<GraphicsImage> game_ui_monster_hp_yellow;
extern std::shared_ptr<GraphicsImage> game_ui_monster_hp_red;
extern std::shared_ptr<GraphicsImage> game_ui_monster_hp_background;
extern std::shared_ptr<GraphicsImage> game_ui_monster_hp_border_left;
extern std::shared_ptr<GraphicsImage> game_ui_monster_hp_border_right;

extern std::shared_ptr<GraphicsImage> game_ui_minimap_frame;    // 5079D8
extern std::shared_ptr<GraphicsImage> game_ui_minimap_compass;  // 5079B4
extern std::array<std::shared_ptr<GraphicsImage>, 8> game_ui_minimap_dirs;

// MM8's top-bar compass cartouche - a horizontally-scrolled direction-letter strip
// ("ib-comp-a", MM8's alignment-invariant copy of MM7's "IB-COMP-A/B/C") behind a gold cover
// frame, relocated from MM7's minimap-adjacent slot to its own top-bar box. See GameUI_DrawCompass
// in UIGame.cpp. game_ui_compass_strip is deliberately its own pointer, not a reuse of
// game_ui_minimap_compass just above - see that pointer's own declaration comment for why.
extern std::shared_ptr<GraphicsImage> game_ui_compass_frame;    // icons.UI.lod "compcovr"
extern std::shared_ptr<GraphicsImage> game_ui_compass_strip;    // icons.lod "ib-comp-a"

// MM8's portrait-adjacent HP/SP bar (icons.lod "mana*" family - not the
// "statbl"/"statg"/"staty"/"statr" gem icons UI_Create() loads into game_ui_bar_*, which are a
// different, small, unrelated status gem). See GameUI_DrawLifeManaBars in UIGame.cpp.
extern std::shared_ptr<GraphicsImage> game_ui_mm8_bar_frame;    // icons.lod "manafrm"
extern std::shared_ptr<GraphicsImage> game_ui_mm8_bar_green;    // icons.lod "manag"
extern std::shared_ptr<GraphicsImage> game_ui_mm8_bar_yellow;   // icons.lod "manay"
extern std::shared_ptr<GraphicsImage> game_ui_mm8_bar_red;      // icons.lod "manar"
extern std::shared_ptr<GraphicsImage> game_ui_mm8_bar_blue;     // icons.lod "manab"

extern std::shared_ptr<GraphicsImage> game_ui_menu_options;

extern std::shared_ptr<GraphicsImage> game_ui_tome_storyline;
extern std::shared_ptr<GraphicsImage> game_ui_tome_calendar;
extern std::shared_ptr<GraphicsImage> game_ui_tome_maps;
extern std::shared_ptr<GraphicsImage> game_ui_tome_autonotes;
extern std::shared_ptr<GraphicsImage> game_ui_tome_quests;

extern std::shared_ptr<GraphicsImage> game_ui_btn_rest;
extern std::shared_ptr<GraphicsImage> game_ui_btn_cast;
extern std::shared_ptr<GraphicsImage> game_ui_btn_zoomin;
extern std::shared_ptr<GraphicsImage> game_ui_btn_zoomout;
extern std::shared_ptr<GraphicsImage> game_ui_btn_quickref;
extern std::shared_ptr<GraphicsImage> game_ui_btn_settings;

extern std::array<std::shared_ptr<GraphicsImage>, 5> game_ui_options_controls;

extern std::shared_ptr<GraphicsImage> game_ui_evtnpc;  // 50795C

// Sized to match `Party::pCharacters` (5 for MM8's up-to-5-member
// party) - `GameUI_DrawPortraits()` indexes this per active character, so it must never be smaller.
extern std::array<std::array<std::shared_ptr<GraphicsImage>, 56>, 5> game_ui_player_faces;

/**
 * MM8's bottom bar holds five portrait slots. The first is under the ring, the other four are where the banners stand.
 *
 * @param slot                          Index of the party member, 0 to 4.
 * @return                              How far right of the first slot this slot is drawn, in pixels.
 */
int mm8PortraitSlotShift(int slot);
extern std::shared_ptr<GraphicsImage> game_ui_player_face_eradicated;
extern std::shared_ptr<GraphicsImage> game_ui_player_face_dead;

extern std::shared_ptr<GraphicsImage> game_ui_player_selection_frame;  // 50C98C
extern std::shared_ptr<GraphicsImage> game_ui_player_alert_yellow;     // 5079C8
extern std::shared_ptr<GraphicsImage> game_ui_player_alert_red;        // 5079CC
extern std::shared_ptr<GraphicsImage> game_ui_player_alert_green;      // 5079D0

// MM8's always-on bottom-bar HUD - a portrait ring ("selring") plus 4 fixed heraldic
// banners ("ibshield01".."04"), both from icons.lod. MM7 has no equivalent.
extern std::shared_ptr<GraphicsImage> game_ui_scorebg;         // icons.lod "scorebg" - MM8 top-bar food/gold icon strip
extern std::shared_ptr<GraphicsImage> game_ui_portrait_ring;   // icons.lod "selring" - MM8 bottom-bar portrait ring
extern std::array<std::shared_ptr<GraphicsImage>, 4> game_ui_party_banners;  // icons.lod "ibshield01".."04"

// The decorative full-width chrome each of the above sits on top of - a separate
// draw call, not baked into scorebg/selring/ibshieldNN (those are colorkeyed icon-only sprites
// with a transparent surround).
extern std::shared_ptr<GraphicsImage> game_ui_top_bar_bg;      // icons.lod (patch) "topbar" - MM8 top-bar background strip
extern std::shared_ptr<GraphicsImage> game_ui_bottom_bar_bg;   // icons.lod "basebar" - MM8 bottom-bar background panel

// The top bar's two bookends: a decorative figure on the left that does nothing when clicked, and a skull on the
// right that opens the spellbook. Neither varies by party alignment (no "-a"/"-b"/"-c" suffix in the archive).
extern std::shared_ptr<GraphicsImage> game_ui_topbar_figurine; // icons.lod "asil00" - MM8 top-bar left bookend (decorative)
extern std::shared_ptr<GraphicsImage> game_ui_topbar_skull;    // icons.lod "asir00" - MM8 top-bar right bookend (opens spellbook)

extern std::shared_ptr<GraphicsImage> game_ui_bar_red;
extern std::shared_ptr<GraphicsImage> game_ui_bar_yellow;
extern std::shared_ptr<GraphicsImage> game_ui_bar_green;
extern std::shared_ptr<GraphicsImage> game_ui_bar_blue;

extern std::shared_ptr<GraphicsImage> game_ui_playerbuff_pain_reflection;
extern std::shared_ptr<GraphicsImage> game_ui_playerbuff_hammerhands;
extern std::shared_ptr<GraphicsImage> game_ui_playerbuff_preservation;
extern std::shared_ptr<GraphicsImage> game_ui_playerbuff_bless;

extern int game_ui_wizardEye;
extern int game_ui_torchLight;

extern bool bFlashHistoryBook;
extern bool bFlashAutonotesBook;
extern bool bFlashQuestBook;

struct OptionsMenuSkin {
    std::shared_ptr<GraphicsImage> uTextureID_Background;                 // 507C60
    std::shared_ptr<GraphicsImage> uTextureID_TurnSpeed[3];               // 507C64
    std::shared_ptr<GraphicsImage> uTextureID_ArrowLeft;                  // 507C70
    std::shared_ptr<GraphicsImage> uTextureID_ArrowRight;                 // 507C74
    std::shared_ptr<GraphicsImage> uTextureID_unused_0;                   // 507C78
    std::shared_ptr<GraphicsImage> uTextureID_unused_1;                   // 507C7C
    std::shared_ptr<GraphicsImage> uTextureID_unused_2;                   // 507C80
    std::shared_ptr<GraphicsImage> uTextureID_FlipOnExit;                 // 507C84
    std::shared_ptr<GraphicsImage> uTextureID_SoundLevels[10];            // 507C88
    std::shared_ptr<GraphicsImage> uTextureID_AlwaysRun;                  // 507CB0
    std::shared_ptr<GraphicsImage> uTextureID_WalkSound;                  // 507CB4
    std::shared_ptr<GraphicsImage> uTextureID_ShowDamage;                 // 507CB8
};
extern OptionsMenuSkin options_menu_skin;  // 507C60
