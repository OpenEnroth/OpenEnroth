#pragma once
#include "GUI/GUIWindow.h"

void GameUI_LoadPlayerPortraintsAndVoices();
void GameUI_ReloadPlayerPortraits(int player_id, int face_id);
void GameUI_WritePointedObjectStatusString();
void GameUI_OnPlayerPortraitLeftClick(unsigned int uPlayerID);  // idb
void buttonbox(int x, int y, const char *text, int col);

class GUIWindow_GameMenu : public GUIWindow {
 public:
    GUIWindow_GameMenu();
    virtual ~GUIWindow_GameMenu() {}

    virtual void Update();
};

class GUIWindow_GameOptions : public GUIWindow {
 public:
    GUIWindow_GameOptions();
    virtual ~GUIWindow_GameOptions() {}

    virtual void Update();
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

    virtual void Update();
};

class GUIWindow_DebugMenu : public GUIWindow {
 public:
     GUIWindow_DebugMenu();
     virtual ~GUIWindow_DebugMenu() {}

     virtual void Update();
};

class Image;
extern Image *game_ui_statusbar;
extern Image *game_ui_rightframe;
extern Image *game_ui_topframe;
extern Image *game_ui_leftframe;
extern Image *game_ui_bottomframe;

extern Image *game_ui_monster_hp_green;
extern Image *game_ui_monster_hp_yellow;
extern Image *game_ui_monster_hp_red;
extern Image *game_ui_monster_hp_background;
extern Image *game_ui_monster_hp_border_left;
extern Image *game_ui_monster_hp_border_right;

extern Image *game_ui_minimap_frame;    // 5079D8
extern Image *game_ui_minimap_compass;  // 5079B4
extern std::array<Image *, 8> game_ui_minimap_dirs;

extern Image *game_ui_menu_quit;
extern Image *game_ui_menu_resume;
extern Image *game_ui_menu_controls;
extern Image *game_ui_menu_save;
extern Image *game_ui_menu_load;
extern Image *game_ui_menu_new;
extern Image *game_ui_menu_options;

extern Image *game_ui_tome_storyline;
extern Image *game_ui_tome_calendar;
extern Image *game_ui_tome_maps;
extern Image *game_ui_tome_autonotes;
extern Image *game_ui_tome_quests;

extern Image *game_ui_btn_rest;
extern Image *game_ui_btn_cast;
extern Image *game_ui_btn_zoomin;
extern Image *game_ui_btn_zoomout;
extern Image *game_ui_btn_quickref;
extern Image *game_ui_btn_settings;

extern Image *game_ui_dialogue_background;

extern std::array<Image *, 5> game_ui_options_controls;

extern Image *game_ui_evtnpc;  // 50795C

extern std::array<std::array<Image *, 56>, 4> game_ui_player_faces;
extern Image *game_ui_player_face_eradicated;
extern Image *game_ui_player_face_dead;

extern Image *game_ui_player_selection_frame;  // 50C98C
extern Image *game_ui_player_alert_yellow;     // 5079C8
extern Image *game_ui_player_alert_red;        // 5079CC
extern Image *game_ui_player_alert_green;      // 5079D0

extern Image *game_ui_bar_red;
extern Image *game_ui_bar_yellow;
extern Image *game_ui_bar_green;
extern Image *game_ui_bar_blue;

extern Image *game_ui_playerbuff_pain_reflection;
extern Image *game_ui_playerbuff_hammerhands;
extern Image *game_ui_playerbuff_preservation;
extern Image *game_ui_playerbuff_bless;

extern bool bFlashHistoryBook;
extern bool bFlashAutonotesBook;
extern bool bFlashQuestBook;
