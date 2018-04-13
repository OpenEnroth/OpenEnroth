#pragma once
#include "GUI/GUIWindow.h"

class GUIWindow_CharacterRecord : public GUIWindow {
 public:
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

bool ringscreenactive();

class Image;
extern Image *ui_character_skills_background;
extern Image *ui_character_awards_background;
extern Image *ui_character_stats_background;
extern Image *ui_character_inventory_background;
extern Image *ui_character_inventory_background_strip;
extern Image *ui_character_inventory_paperdoll_background;

extern std::array<Image *, 16> paperdoll_dbrds;
