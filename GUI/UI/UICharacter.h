#pragma once
#include "GUI/GUIWindow.h"

struct GUIWindow_CharacterRecord : public GUIWindow
{
             GUIWindow_CharacterRecord(unsigned int uActiveCharacter, enum CURRENT_SCREEN screen);
    virtual ~GUIWindow_CharacterRecord() {}

    virtual void Update();

    void ShowStatsTab();
    void ShowSkillsTab();
    void ShowInventoryTab();
    void ShowAwardsTab();
    void ToggleRingsOverlay();

    protected:
        void CharacterUI_StatsTab_Draw(struct Player *);
        void CharacterUI_SkillsTab_Draw(struct Player *);
        void CharacterUI_AwardsTab_Draw(struct Player *);

        void CharacterUI_SkillsTab_CreateButtons();
};



extern class Image *ui_character_skills_background;
extern class Image *ui_character_awards_background;
extern class Image *ui_character_stats_background;
extern class Image *ui_character_inventory_background;
extern class Image *ui_character_inventory_background_strip;
extern class Image *ui_character_inventory_paperdoll_background;

extern std::array<class Image *, 16> paperdoll_dbrds;