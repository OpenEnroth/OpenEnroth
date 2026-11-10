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

extern std::shared_ptr<GraphicsImage> game_ui_menu_quit;
extern std::shared_ptr<GraphicsImage> game_ui_menu_resume;
extern std::shared_ptr<GraphicsImage> game_ui_menu_controls;
extern std::shared_ptr<GraphicsImage> game_ui_menu_save;
extern std::shared_ptr<GraphicsImage> game_ui_menu_load;
extern std::shared_ptr<GraphicsImage> game_ui_menu_new;
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

extern std::array<std::array<std::shared_ptr<GraphicsImage>, 56>, 4> game_ui_player_faces;
extern std::shared_ptr<GraphicsImage> game_ui_player_face_eradicated;
extern std::shared_ptr<GraphicsImage> game_ui_player_face_dead;

extern std::shared_ptr<GraphicsImage> game_ui_player_selection_frame;  // 50C98C
extern std::shared_ptr<GraphicsImage> game_ui_player_alert_yellow;     // 5079C8
extern std::shared_ptr<GraphicsImage> game_ui_player_alert_red;        // 5079CC
extern std::shared_ptr<GraphicsImage> game_ui_player_alert_green;      // 5079D0

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
