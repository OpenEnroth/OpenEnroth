#pragma once

#include <vector>
#include <string>

#include "GUI/GUIWindow.h"

class GUIWindow_CharacterRecord : public GUIWindow {
 public:
    GUIWindow_CharacterRecord(int uActiveCharacter, ScreenType screen);
    virtual ~GUIWindow_CharacterRecord() {}

    virtual void Update() override;

    void ShowStatsTab();
    void ShowSkillsTab();
    void ShowInventoryTab();
    void ShowAwardsTab();
    void ToggleRingsOverlay();

    void clickAwardsUp();
    void clickAwardsDown();
    void clickAwardsScroll(int yPos);

    void createAwardsScrollBar();
    void releaseAwardsScrollBar();

 protected:
    void CharacterUI_StatsTab_Draw(Character *);
    void CharacterUI_SkillsTab_Draw(Character *);

    /**
     * @offset 0x41A000
     */
    void CharacterUI_AwardsTab_Draw(Character *);

    void CharacterUI_SkillsTab_CreateButtons();

    /**
     * @offset 0x419100
     */
    void fillAwardsData();

    std::string getAchievedAwardsString(int idx);
    GUIWindow prepareAwardsWindow();

 private:
    bool _awardsScrollBarCreated = false;
    int _awardsCharacterId = -1;
    int _startAwardElem = 0;
    int _scrollableAwardSteps = 0;
    bool _awardLimitReached = false;
    std::vector<int> _achievedAwardsList;
};

bool ringscreenactive();
static void CharacterUI_DrawItem(int x, int y, Item *item, int id, GraphicsImage *item_texture = nullptr, bool doZDraw = false);

class GraphicsImage;
extern GraphicsImage *ui_character_skills_background;
extern GraphicsImage *ui_character_awards_background;
extern GraphicsImage *ui_character_stats_background;
extern GraphicsImage *ui_character_inventory_background;
extern GraphicsImage *ui_character_inventory_background_strip;
extern GraphicsImage *ui_character_inventory_paperdoll_background;

extern std::array<GraphicsImage *, 16> paperdoll_dbrds;
