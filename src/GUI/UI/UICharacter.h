#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include "Engine/HitMap.h"
#include "Engine/Data/AwardEnums.h"
#include "Engine/Objects/ItemEnums.h"

#include "GUI/GUIWindow.h"

class GUIWindow_CharacterRecord : public GUIWindow {
 public:
    explicit GUIWindow_CharacterRecord(ScreenType screen);
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

    // Test accessors for the awards list's scroll position.
    int startAwardElem() const { return _startAwardElem; }
    int scrollableAwardSteps() const { return _scrollableAwardSteps; }

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

 private:
    bool _awardsScrollBarCreated = false;
    int _awardsCharacterId = -1;
    int _startAwardElem = 0;
    int _scrollableAwardSteps = 0;
    bool _awardLimitReached = false;
    std::vector<AwardId> _achievedAwardsList;
};

bool ringscreenactive();

class GraphicsImage;
extern std::shared_ptr<GraphicsImage> ui_character_skills_background;
extern std::shared_ptr<GraphicsImage> ui_character_awards_background;
extern std::shared_ptr<GraphicsImage> ui_character_stats_background;
extern std::shared_ptr<GraphicsImage> ui_character_inventory_background;
extern std::shared_ptr<GraphicsImage> ui_character_inventory_background_strip;
extern std::shared_ptr<GraphicsImage> ui_character_inventory_paperdoll_background;

extern std::array<std::shared_ptr<GraphicsImage>, 16> paperdoll_dbrds;

extern HitMap<int> equipmentHitMap;

/**
 * Builds a paperdoll tier-index lookup for a given equipment slot's `ItemType`, ranking every real
 * `RARITY_COMMON` item of that type by `baseValue` (ties broken by id) and assigning it index
 * 0, 1, 2, ... MM8's `items.txt` orders its rows differently from MM7's, so a hardcoded `ItemId` table
 * doesn't work. Declared here for tests.
 */
std::unordered_map<ItemId, int> buildPaperdollIndexByType(ItemType type);

/**
 * Declared here for tests.
 */
void CharacterUI_LoadPaperdollTextures();
