#pragma once

#include <string>

#include "GUI/GUIWindow.h"

void GameUI_LoadPlayerPortraitsAndVoices();
void GameUI_ReloadPlayerPortraits(int player_id, int face_id);
void GameUI_WritePointedObjectStatusString();
void GameUI_OnPlayerPortraitLeftClick(int uPlayerID);  // idb
void buttonbox(int x, int y, std::string_view text, int col);
void GameUI_handleHintMessage(UIMessageType type, int param);

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
extern GraphicsImage *game_ui_statusbar;
extern GraphicsImage *game_ui_rightframe;
extern GraphicsImage *game_ui_topframe;
extern GraphicsImage *game_ui_leftframe;
extern GraphicsImage *game_ui_bottomframe;

extern GraphicsImage *game_ui_monster_hp_green;
extern GraphicsImage *game_ui_monster_hp_yellow;
extern GraphicsImage *game_ui_monster_hp_red;
extern GraphicsImage *game_ui_monster_hp_background;
extern GraphicsImage *game_ui_monster_hp_border_left;
extern GraphicsImage *game_ui_monster_hp_border_right;

extern GraphicsImage *game_ui_minimap_frame;    // 5079D8
extern GraphicsImage *game_ui_minimap_compass;  // 5079B4
extern std::array<GraphicsImage *, 8> game_ui_minimap_dirs;

extern GraphicsImage *game_ui_menu_quit;
extern GraphicsImage *game_ui_menu_resume;
extern GraphicsImage *game_ui_menu_controls;
extern GraphicsImage *game_ui_menu_save;
extern GraphicsImage *game_ui_menu_load;
extern GraphicsImage *game_ui_menu_new;
extern GraphicsImage *game_ui_menu_options;

extern GraphicsImage *game_ui_tome_storyline;
extern GraphicsImage *game_ui_tome_calendar;
extern GraphicsImage *game_ui_tome_maps;
extern GraphicsImage *game_ui_tome_autonotes;
extern GraphicsImage *game_ui_tome_quests;

extern GraphicsImage *game_ui_btn_rest;
extern GraphicsImage *game_ui_btn_cast;
extern GraphicsImage *game_ui_btn_zoomin;
extern GraphicsImage *game_ui_btn_zoomout;
extern GraphicsImage *game_ui_btn_quickref;
extern GraphicsImage *game_ui_btn_settings;

extern GraphicsImage *game_ui_dialogue_background;

extern std::array<GraphicsImage *, 5> game_ui_options_controls;

extern GraphicsImage *game_ui_evtnpc;  // 50795C

extern std::array<std::array<GraphicsImage *, 56>, 4> game_ui_player_faces;
extern GraphicsImage *game_ui_player_face_eradicated;
extern GraphicsImage *game_ui_player_face_dead;

extern GraphicsImage *game_ui_player_selection_frame;  // 50C98C
extern GraphicsImage *game_ui_player_alert_yellow;     // 5079C8
extern GraphicsImage *game_ui_player_alert_red;        // 5079CC
extern GraphicsImage *game_ui_player_alert_green;      // 5079D0

extern GraphicsImage *game_ui_bar_red;
extern GraphicsImage *game_ui_bar_yellow;
extern GraphicsImage *game_ui_bar_green;
extern GraphicsImage *game_ui_bar_blue;

extern GraphicsImage *game_ui_playerbuff_pain_reflection;
extern GraphicsImage *game_ui_playerbuff_hammerhands;
extern GraphicsImage *game_ui_playerbuff_preservation;
extern GraphicsImage *game_ui_playerbuff_bless;

extern int game_ui_wizardEye;
extern int game_ui_torchLight;

extern bool bFlashHistoryBook;
extern bool bFlashAutonotesBook;
extern bool bFlashQuestBook;

struct OptionsMenuSkin {
    OptionsMenuSkin();
    void Release();

    GraphicsImage *uTextureID_Background;       // 507C60
    GraphicsImage *uTextureID_TurnSpeed[3];     // 507C64
    GraphicsImage *uTextureID_ArrowLeft;        // 507C70
    GraphicsImage *uTextureID_ArrowRight;       // 507C74
    GraphicsImage *uTextureID_unused_0;         // 507C78
    GraphicsImage *uTextureID_unused_1;         // 507C7C
    GraphicsImage *uTextureID_unused_2;         // 507C80
    GraphicsImage *uTextureID_FlipOnExit;       // 507C84
    GraphicsImage *uTextureID_SoundLevels[10];  // 507C88
    GraphicsImage *uTextureID_AlwaysRun;        // 507CB0
    GraphicsImage *uTextureID_WalkSound;        // 507CB4
    GraphicsImage *uTextureID_ShowDamage;       // 507CB8
};
extern OptionsMenuSkin options_menu_skin;  // 507C60
