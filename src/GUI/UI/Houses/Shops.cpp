#include <cstdlib>
#include <algorithm>
#include <string>
#include <array>

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/Items.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Tables/ItemTable.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/Houses/Shops.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

#include "Utility/IndexedArray.h"

struct ITEM_VARIATION {
    ITEM_TREASURE_LEVEL treasure_level;
    std::array<uint16_t, 4> item_class;
};

// TODO(Nik-RE-dev): enumerate treasure types used in this file

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_WEAPON_SHOP, HOUSE_LAST_WEAPON_SHOP> weaponShopVariationStandart = {{
    {HOUSE_SMITH_EMERALD_ISLE,      { ITEM_TREASURE_LEVEL_1, { 23, 27, 20, 20 } }},
    {HOUSE_SMITH_HARMONDALE,        { ITEM_TREASURE_LEVEL_1, { 23, 24, 28, 20 } }},
    {HOUSE_SMITH_ERATHIA,           { ITEM_TREASURE_LEVEL_2, { 23, 24, 25, 20 } }},
    {HOUSE_SMITH_TULAREAN_FOREST,   { ITEM_TREASURE_LEVEL_2, { 27, 27, 26, 26 } }},
    {HOUSE_SMITH_CELESTE,           { ITEM_TREASURE_LEVEL_4, { 24, 30, 25, 27 } }},
    {HOUSE_SMITH_PIT,               { ITEM_TREASURE_LEVEL_4, { 24, 30, 25, 27 } }},
    {HOUSE_SMITH_NIGHON,            { ITEM_TREASURE_LEVEL_3, { 30, 24, 20, 20 } }},
    {HOUSE_SMITH_TATALIA_VANDERS,   { ITEM_TREASURE_LEVEL_2, { 20, 20, 20, 20 } }},
    {HOUSE_SMITH_AVLEE,             { ITEM_TREASURE_LEVEL_3, { 27, 27, 26, 26 } }},
    {HOUSE_SMITH_STONE_CITY,        { ITEM_TREASURE_LEVEL_3, { 28, 28, 25, 25 } }},
    {HOUSE_SMITH_CASTLE_HARMONDALE, { ITEM_TREASURE_LEVEL_2, { 23, 23, 24, 24 } }},
    {HOUSE_SMITH_TATALIA_ALLOYED,   { ITEM_TREASURE_LEVEL_3, { 23, 23, 26, 26 } }},
    {HOUSE_SMITH_13,                { ITEM_TREASURE_LEVEL_2, { 30, 26, 26, 26 } }},
    {HOUSE_SMITH_14,                { ITEM_TREASURE_LEVEL_2, { 28, 25, 28, 29 } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_WEAPON_SHOP, HOUSE_LAST_WEAPON_SHOP> weaponShopVariationSpecial = {{
    {HOUSE_SMITH_EMERALD_ISLE,      { ITEM_TREASURE_LEVEL_2, { 25, 30, 20, 20 } }},
    {HOUSE_SMITH_HARMONDALE,        { ITEM_TREASURE_LEVEL_2, { 23, 24, 28, 20 } }},
    {HOUSE_SMITH_ERATHIA,           { ITEM_TREASURE_LEVEL_3, { 23, 24, 25, 20 } }},
    {HOUSE_SMITH_TULAREAN_FOREST,   { ITEM_TREASURE_LEVEL_3, { 27, 27, 26, 26 } }},
    {HOUSE_SMITH_CELESTE,           { ITEM_TREASURE_LEVEL_5, { 23, 26, 28, 27 } }},
    {HOUSE_SMITH_PIT,               { ITEM_TREASURE_LEVEL_5, { 23, 26, 28, 27 } }},
    {HOUSE_SMITH_NIGHON,            { ITEM_TREASURE_LEVEL_4, { 30, 24, 20, 20 } }},
    {HOUSE_SMITH_TATALIA_VANDERS,   { ITEM_TREASURE_LEVEL_3, { 20, 20, 20, 20 } }},
    {HOUSE_SMITH_AVLEE,             { ITEM_TREASURE_LEVEL_4, { 27, 27, 26, 26 } }},
    {HOUSE_SMITH_STONE_CITY,        { ITEM_TREASURE_LEVEL_4, { 28, 28, 25, 25 } }},
    {HOUSE_SMITH_CASTLE_HARMONDALE, { ITEM_TREASURE_LEVEL_4, { 23, 23, 24, 24 } }},
    {HOUSE_SMITH_TATALIA_ALLOYED,   { ITEM_TREASURE_LEVEL_4, { 24, 24, 27, 20 } }},
    {HOUSE_SMITH_13,                { ITEM_TREASURE_LEVEL_4, { 30, 26, 26, 26 } }},
    {HOUSE_SMITH_14,                { ITEM_TREASURE_LEVEL_4, { 28, 25, 28, 29 } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopTopRowVariationStandart = {{
    {HOUSE_ARMOURER_EMERALD_ISLE,         { ITEM_TREASURE_LEVEL_1, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_HARMONDALE,           { ITEM_TREASURE_LEVEL_1, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_ERATHIA,              { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_TULAREAN_FOREST,      { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_CELESTE,              { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_PIT,                  { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_NIGHON,               { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_TATALIA_MISSING_LINK, { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_AVLEE,                { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_STONE_CITY,           { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_CASTLE_HARMONDALE,    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_TATALIA_ALLOYED,      { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_27,                   { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_28,                   { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopBottomRowVariationStandart = {{
    {HOUSE_ARMOURER_EMERALD_ISLE,         { ITEM_TREASURE_LEVEL_1, { 31, 31, 31, 34 } }},
    {HOUSE_ARMOURER_HARMONDALE,           { ITEM_TREASURE_LEVEL_1, { 31, 31, 32, 34 } }},
    {HOUSE_ARMOURER_ERATHIA,              { ITEM_TREASURE_LEVEL_2, { 31, 32, 32, 33 } }},
    {HOUSE_ARMOURER_TULAREAN_FOREST,      { ITEM_TREASURE_LEVEL_2, { 31, 31, 32, 32 } }},
    {HOUSE_ARMOURER_CELESTE,              { ITEM_TREASURE_LEVEL_4, { 31, 32, 33, 34 } }},
    {HOUSE_ARMOURER_PIT,                  { ITEM_TREASURE_LEVEL_4, { 31, 32, 33, 34 } }},
    {HOUSE_ARMOURER_NIGHON,               { ITEM_TREASURE_LEVEL_3, { 31, 31, 31, 31 } }},
    {HOUSE_ARMOURER_TATALIA_MISSING_LINK, { ITEM_TREASURE_LEVEL_2, { 31, 32, 34, 34 } }},
    {HOUSE_ARMOURER_AVLEE,                { ITEM_TREASURE_LEVEL_3, { 31, 31, 32, 32 } }},
    {HOUSE_ARMOURER_STONE_CITY,           { ITEM_TREASURE_LEVEL_3, { 32, 32, 32, 33 } }},
    {HOUSE_ARMOURER_CASTLE_HARMONDALE,    { ITEM_TREASURE_LEVEL_3, { 31, 31, 31, 32 } }},
    {HOUSE_ARMOURER_TATALIA_ALLOYED,      { ITEM_TREASURE_LEVEL_3, { 33, 31, 32, 34 } }},
    {HOUSE_ARMOURER_27,                   { ITEM_TREASURE_LEVEL_3, { 33, 31, 32, 34 } }},
    {HOUSE_ARMOURER_28,                   { ITEM_TREASURE_LEVEL_4, { 33, 31, 32, 34 } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopTopRowVariationSpecial = {{
    {HOUSE_ARMOURER_EMERALD_ISLE,         { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_HARMONDALE,           { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_ERATHIA,              { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_TULAREAN_FOREST,      { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_CELESTE,              { ITEM_TREASURE_LEVEL_5, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_PIT,                  { ITEM_TREASURE_LEVEL_5, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_NIGHON,               { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_TATALIA_MISSING_LINK, { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_AVLEE,                { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_STONE_CITY,           { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_CASTLE_HARMONDALE,    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_TATALIA_ALLOYED,      { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_27,                   { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } }},
    {HOUSE_ARMOURER_28,                   { ITEM_TREASURE_LEVEL_5, { 35, 35, 38, 38 } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopBottomRowVariationSpecial = {{
    {HOUSE_ARMOURER_EMERALD_ISLE,         { ITEM_TREASURE_LEVEL_2, { 31, 31, 31, 34 } }},
    {HOUSE_ARMOURER_HARMONDALE,           { ITEM_TREASURE_LEVEL_2, { 31, 31, 32, 34 } }},
    {HOUSE_ARMOURER_ERATHIA,              { ITEM_TREASURE_LEVEL_3, { 31, 32, 32, 33 } }},
    {HOUSE_ARMOURER_TULAREAN_FOREST,      { ITEM_TREASURE_LEVEL_3, { 31, 31, 32, 32 } }},
    {HOUSE_ARMOURER_CELESTE,              { ITEM_TREASURE_LEVEL_5, { 31, 32, 33, 34 } }},
    {HOUSE_ARMOURER_PIT,                  { ITEM_TREASURE_LEVEL_5, { 31, 32, 33, 34 } }},
    {HOUSE_ARMOURER_NIGHON,               { ITEM_TREASURE_LEVEL_4, { 31, 31, 31, 31 } }},
    {HOUSE_ARMOURER_TATALIA_MISSING_LINK, { ITEM_TREASURE_LEVEL_3, { 31, 32, 34, 34 } }},
    {HOUSE_ARMOURER_AVLEE,                { ITEM_TREASURE_LEVEL_4, { 31, 31, 32, 33 } }},
    {HOUSE_ARMOURER_STONE_CITY,           { ITEM_TREASURE_LEVEL_4, { 32, 32, 33, 34 } }},
    {HOUSE_ARMOURER_CASTLE_HARMONDALE,    { ITEM_TREASURE_LEVEL_4, { 31, 31, 31, 32 } }},
    {HOUSE_ARMOURER_TATALIA_ALLOYED,      { ITEM_TREASURE_LEVEL_4, { 32, 32, 32, 32 } }},
    {HOUSE_ARMOURER_27,                   { ITEM_TREASURE_LEVEL_4, { 34, 34, 34, 34 } }},
    {HOUSE_ARMOURER_28,                   { ITEM_TREASURE_LEVEL_5, { 33, 33, 33, 33 } }}
}};

static constexpr IndexedArray<ITEM_TREASURE_LEVEL, HOUSE_FIRST_MAGIC_SHOP, HOUSE_LAST_MAGIC_SHOP> magicShopVariationStandart = {{
    {HOUSE_MAGE_EMERALD_ISLE,    ITEM_TREASURE_LEVEL_1},
    {HOUSE_MAGE_HARMONDALE,      ITEM_TREASURE_LEVEL_1},
    {HOUSE_MAGE_ERATHIA,         ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_TULAREAN_FOREST, ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_DEYJA,           ITEM_TREASURE_LEVEL_4},
    {HOUSE_MAGE_BRACADA_DESERT,  ITEM_TREASURE_LEVEL_4},
    {HOUSE_MAGE_CELESTE,         ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGE_PIT,             ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_NIGHON,          ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_STONE_CITY,      ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_39,              ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_40,              ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_41,              ITEM_TREASURE_LEVEL_2}
}};

static constexpr IndexedArray<ITEM_TREASURE_LEVEL, HOUSE_FIRST_MAGIC_SHOP, HOUSE_LAST_MAGIC_SHOP> magicShopVariationSpecial = {{
    {HOUSE_MAGE_EMERALD_ISLE,    ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_HARMONDALE,      ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGE_ERATHIA,         ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGE_TULAREAN_FOREST, ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGE_DEYJA,           ITEM_TREASURE_LEVEL_5},
    {HOUSE_MAGE_BRACADA_DESERT,  ITEM_TREASURE_LEVEL_5},
    {HOUSE_MAGE_CELESTE,         ITEM_TREASURE_LEVEL_4},
    {HOUSE_MAGE_PIT,             ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGE_NIGHON,          ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGE_STONE_CITY,      ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGE_39,              ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGE_40,              ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGE_41,              ITEM_TREASURE_LEVEL_3}
}};

static constexpr IndexedArray<ITEM_TREASURE_LEVEL, HOUSE_FIRST_ALCHEMY_SHOP, HOUSE_LAST_ALCHEMY_SHOP> alchemyShopVariationStandart = {{
    {HOUSE_ALCHEMIST_EMERALD_ISLE,      ITEM_TREASURE_LEVEL_1},
    {HOUSE_ALCHEMIST_HARMONDALE,        ITEM_TREASURE_LEVEL_1},
    {HOUSE_ALCHEMIST_ERATHIA,           ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_TULAREAN_FOREST,   ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_DEYJA,             ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMIST_BRACADA_DESERT,    ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMIST_CELESTE,           ITEM_TREASURE_LEVEL_4},
    {HOUSE_ALCHEMIST_PIT,               ITEM_TREASURE_LEVEL_4},
    {HOUSE_ALCHEMIST_STONE_CITY,        ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_CASTLE_HARMONDALE, ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_52,                ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_53,                ITEM_TREASURE_LEVEL_2}
}};

static constexpr IndexedArray<ITEM_TREASURE_LEVEL, HOUSE_FIRST_ALCHEMY_SHOP, HOUSE_LAST_ALCHEMY_SHOP> alchemyShopVariationSpecial = {{
    {HOUSE_ALCHEMIST_EMERALD_ISLE,      ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_HARMONDALE,        ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_ERATHIA,           ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMIST_TULAREAN_FOREST,   ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMIST_DEYJA,             ITEM_TREASURE_LEVEL_4},
    {HOUSE_ALCHEMIST_BRACADA_DESERT,    ITEM_TREASURE_LEVEL_4},
    {HOUSE_ALCHEMIST_CELESTE,           ITEM_TREASURE_LEVEL_5},
    {HOUSE_ALCHEMIST_PIT,               ITEM_TREASURE_LEVEL_5},
    {HOUSE_ALCHEMIST_STONE_CITY,        ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMIST_CASTLE_HARMONDALE, ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_52,                ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMIST_53,                ITEM_TREASURE_LEVEL_2}
}};

GraphicsImage *shop_ui_background = nullptr;

std::array<GraphicsImage *, 12> shop_ui_items_in_store;
std::array<int, 6> weaponYPos;

bool isStealingModeActive() {
    return keyboardInputHandler->IsStealingToggled() && pParty->activeCharacter().CanSteal();
}

void addUniqueItemClasses(const ITEM_VARIATION &variation, std::vector<int> &set) {
    for (int itemClass : variation.item_class) {
        if (std::find(set.begin(), set.end(), itemClass) == set.end()) {
            set.push_back(itemClass);
        }
    }
}

DIALOGUE_TYPE getSkillLearnDualogueForItemClass(int itemClass) {
    switch (itemClass) {
      case 23:
        return DIALOGUE_LEARN_SWORD;
      case 24:
        return DIALOGUE_LEARN_DAGGER;
      case 25:
        return DIALOGUE_LEARN_AXE;
      case 26:
        return DIALOGUE_LEARN_SPEAR;
      case 27:
        return DIALOGUE_LEARN_BOW;
      case 28:
        return DIALOGUE_LEARN_MACE;
      case 30:
        return DIALOGUE_LEARN_STAFF;
      case 31:
        return DIALOGUE_LEARN_LEATHER;
      case 32:
        return DIALOGUE_LEARN_CHAIN;
      case 33:
        return DIALOGUE_LEARN_PLATE;
      case 34:
        return DIALOGUE_LEARN_SHIELD;
      default:
        return DIALOGUE_NULL;
    }
}

void GUIWindow_Shop::mainDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    std::vector<std::string> optionsText = {localization->GetString(LSTR_STANDARD), localization->GetString(LSTR_SPECIAL),
                                            localization->GetString(LSTR_DISPLAY), localization->GetString(LSTR_LEARN_SKILLS)};

    drawOptions(optionsText, colorTable.Sunflower);
}

void GUIWindow_Shop::displayEquipmentDialogue() {
    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    std::vector<std::string> optionsText = {localization->GetString(LSTR_SELL), localization->GetString(LSTR_IDENTIFY)};

    if (buildingType() != BUILDING_ALCHEMY_SHOP) {
        optionsText.push_back(localization->GetString(LSTR_REPAIR));
    }

    drawOptions(optionsText, colorTable.Sunflower);
}

void GUIWindow_Shop::sellDialogue() {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    if (checkIfPlayerCanInteract()) {
        engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_SELL), colorTable.White);

        Pointi pt = dialogwin.mouse->GetCursorPos();

        int invindex = ((pt.x - 14) / 32) + 14 * ((pt.y - 17) / 32);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
        if (pItemID) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            MerchantPhrase phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), 3);
            std::string str = BuildDialogueString(pMerchantsSellPhrases[phrases_id], pParty->activeCharacterIndex() - 1, item, houseId(), 3);
            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            dialogwin.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.White, str, 3);
        }
    }
}

void GUIWindow_Shop::identifyDialogue() {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    if (checkIfPlayerCanInteract()) {
        engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_IDENTIFY), colorTable.White);

        Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

        int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);

        if (pItemID) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];

            std::string str;
            if (!item->IsIdentified()) {
                MerchantPhrase phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), 4);
                str = BuildDialogueString(pMerchantsIdentifyPhrases[phrases_id], pParty->activeCharacterIndex() - 1, item, houseId(), 4);
            } else {
                str = BuildDialogueString("%24", pParty->activeCharacterIndex() - 1, item, houseId(), 4);
            }

            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            dialogwin.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.White, str, 3);
        }
    }
}

void GUIWindow_Shop::repairDialogue() {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    if (checkIfPlayerCanInteract()) {
        engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_REPAIR), colorTable.White);

        Pointi pt = dialogwin.mouse->GetCursorPos();

        int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
        if (pItemID == 0)
            return;

        if (pParty->activeCharacter().pOwnItems[pItemID - 1].uAttributes & ITEM_BROKEN) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            MerchantPhrase phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), 5);
            std::string str = BuildDialogueString(pMerchantsRepairPhrases[phrases_id], pParty->activeCharacterIndex() - 1, item, houseId(), 5);
            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            dialogwin.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.White, str, 3);
        }
    }
}

void GUIWindow_WeaponShop::shopWaresDialogue(bool isSpecial) {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
    int item_X = 0;

    for (int i = 0; i < 6; ++i) {
        bool itemPresent = (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        if (itemPresent) {
            render->DrawTextureNew(((60 - (shop_ui_items_in_store[i]->width() / 2)) + item_X) / 640.0f, (weaponYPos[i] + 30) / 480.0f, shop_ui_items_in_store[i]);
        }

        item_X += 70;
    }

    if (checkIfPlayerCanInteract()) {
        int item_num = 0;
        for (int i = 0; i < 6; ++i) {
            item_num += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            engine->_statusBar->drawForced(localization->GetString(LSTR_STEAL_ITEM), colorTable.White);
        } else {
            engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), colorTable.White);
        }

        if (item_num) {
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();
            ItemGen *item;

            int testx = (pt.x - 30) / 70;
            if (testx >= 0 && testx < 6) {  // testx limits check
                if (isSpecial) {
                    item = &pParty->specialItemsInShops[houseId()][testx];
                } else {
                    item = &pParty->standartItemsInShops[houseId()][testx];
                }

                if (item->uItemID != ITEM_NULL) {  // item picking
                    int testpos = ((60 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 70);

                    if (pt.x >= testpos && pt.x < (testpos + shop_ui_items_in_store[testx]->width())) {
                        if (pt.y >= weaponYPos[testx] + 30 && pt.y < (weaponYPos[testx] + 30 + shop_ui_items_in_store[testx]->height())) {
                            std::string str;
                            if (!isStealingModeActive()) {
                                MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, BUILDING_WEAPON_SHOP, houseId(), 2);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, item, houseId(), 2);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, item, houseId(), 2);
                            }
                            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
                            dialogwin.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.White, str, 3);
                        }
                    }
                }
            }
        } else {
            dialogwin.DrawShops_next_generation_time_string(pParty->PartyTimes.shopNextRefreshTime[houseId()] - pParty->GetPlayingTime());
        }
    }
}

void GUIWindow_ArmorShop::shopWaresDialogue(bool isSpecial) {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
    int item_x = 0;

    for (int i = 0; i < 8; ++i) {
        bool itemPresent = (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;

        if (itemPresent) {
            if (i >= 4) {
                render->DrawTextureNew(((90 - (shop_ui_items_in_store[i]->width() / 2)) + item_x - 420) / 640.0f, 126 / 480.0f, shop_ui_items_in_store[i]);
            } else {
                int x = ((86 - (shop_ui_items_in_store[i]->width() / 2)) + item_x);
                int y = (98 - shop_ui_items_in_store[i]->height());
                render->DrawTextureNew(x / 640.0f, y / 480.0f, shop_ui_items_in_store[i]);
            }
        }
        item_x += 105;
    }

    if (checkIfPlayerCanInteract()) {
        int pItemCount = 0;
        for (int i = 0; i < 6; ++i) {
            pItemCount += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            engine->_statusBar->drawForced(localization->GetString(LSTR_STEAL_ITEM), colorTable.White);
        } else {
            engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), colorTable.White);
        }

        if (pItemCount) {
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

            // testx limits check
            int testx = (pt.x - 40) / 105;
            if (testx >= 0 && testx < 4) {
                if (pt.y >= 126) {
                    testx += 4;
                }

                ItemGen *item;
                if (isSpecial) {
                    item = &pParty->specialItemsInShops[houseId()][testx];
                } else {
                    item = &pParty->standartItemsInShops[houseId()][testx];
                }

                if (item->uItemID != ITEM_NULL) {
                    int testpos;
                    if (testx >= 4) {
                        testpos = ((90 - (shop_ui_items_in_store[testx]->width() / 2)) + (testx * 105) - 420);
                    } else {
                        testpos = ((86 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 105);
                    }

                    if (pt.x >= testpos &&
                        pt.x <= testpos + shop_ui_items_in_store[testx]->width()) {
                        if ((pt.y >= 126 && pt.y < (126 + shop_ui_items_in_store[testx]->height())) ||
                            (pt.y <= 98 && pt.y >= (98 - shop_ui_items_in_store[testx]->height()))) {
                            // y is 126 to 126 + height low or 98-height to 98

                            std::string str;
                            if (!isStealingModeActive()) {
                                MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), 2);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, item, houseId(), 2);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, item, houseId(), 2);
                            }
                            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
                            dialogwin.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.White, str, 3);
                        }
                    }
                }
            }
        } else {
            dialogwin.DrawShops_next_generation_time_string(pParty->PartyTimes.shopNextRefreshTime[houseId()] - pParty->GetPlayingTime());
        }
    }
}

void GUIWindow_MagicAlchemyShop::shopWaresDialogue(bool isSpecial) {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);

    for (uint i = 0; i < 12; ++i) {
        bool itemPresent = (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        int itemx, itemy;

        if (itemPresent) {
            if (i >= 6) {
                itemy = 308 - shop_ui_items_in_store[i]->height();
                if (itemy < 0)
                    itemy = 0;
                itemx = 75 * i - shop_ui_items_in_store[i]->width() / 2 + 40 - 450;
            } else {
                itemy = 152 - shop_ui_items_in_store[i]->height();
                if (itemy < 0)
                    itemy = 0;
                itemx = 75 * i - shop_ui_items_in_store[i]->width() / 2 + 40;
            }

            if (i == 0 || i == 6) {
                if (itemx < 18)
                    itemx = 18;
            }

            if (i == 5 || i == 11) {
                if (itemx > 457 - shop_ui_items_in_store[i]->width()) {
                    itemx = 457 - shop_ui_items_in_store[i]->width();
                }
            }

            render->DrawTextureNew(itemx / 640.0f, itemy / 480.0f, shop_ui_items_in_store[i]);
        }
    }

    if (checkIfPlayerCanInteract()) {
        int item_num = 0;

        for (int i = 0; i < 12; ++i) {
            item_num += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            engine->_statusBar->drawForced(localization->GetString(LSTR_STEAL_ITEM), colorTable.White);
        } else {
            engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), colorTable.White);
        }

        if (item_num) {
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

            // testx limits check
            int testx = (pt.x) / 75;
            if (testx >= 0 && testx < 6) {
                if (pt.y > 152) {
                    testx += 6;
                }

                ItemGen *item;
                if (isSpecial) {
                    item = &pParty->specialItemsInShops[houseId()][testx];
                } else {
                    item = &pParty->standartItemsInShops[houseId()][testx];
                }

                if (item->uItemID != ITEM_NULL) {
                    int testpos;
                    if (pt.y > 152) {
                        testpos = 75 * testx - shop_ui_items_in_store[testx]->width() / 2 + 40 - 450;
                    } else {
                        testpos = 75 * testx - shop_ui_items_in_store[testx]->width() / 2 + 40;
                    }

                    if (pt.x >= testpos &&
                        pt.x <= testpos + shop_ui_items_in_store[testx]->width()) {
                        if ((pt.y <= 308 && pt.y >= (308 - shop_ui_items_in_store[testx]->height())) ||
                            (pt.y <= 152 && pt.y >= (152 - shop_ui_items_in_store[testx]->height()))) {
                            // y is 152-h to 152 or 308-height to 308

                            std::string str;
                            if (!isStealingModeActive()) {
                                MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), 2);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, item, houseId(), 2);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, item, houseId(), 2);
                            }
                            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
                            dialogwin.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.White, str, 3);
                        }
                    }
                }
            }
        } else {
            dialogwin.DrawShops_next_generation_time_string(pParty->PartyTimes.shopNextRefreshTime[houseId()] - pParty->GetPlayingTime());
        }
    }
}

void GUIWindow_WeaponShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    const ITEM_VARIATION variation = isSpecial ? weaponShopVariationSpecial[houseId()] : weaponShopVariationStandart[houseId()];

    for (int i = 0; i < itemAmountInShop[buildingType()]; i++) {
        int itemClass = variation.item_class[grng->random(4)];
        pItemTable->generateItem(variation.treasure_level, itemClass, &itemArray[i]);
        itemArray[i].SetIdentified();
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

void GUIWindow_ArmorShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    const ITEM_VARIATION variationTop = isSpecial ? armorShopTopRowVariationSpecial[houseId()] : armorShopTopRowVariationStandart[houseId()];
    const ITEM_VARIATION variationBottom = isSpecial ? armorShopBottomRowVariationSpecial[houseId()] : armorShopBottomRowVariationStandart[houseId()];

    for (int i = 0; i < itemAmountInShop[buildingType()]; i++) {
        int itemClass;
        ITEM_TREASURE_LEVEL treasureLvl;

        if (i >= 4) {
            treasureLvl = variationBottom.treasure_level;
            itemClass = variationBottom.item_class[grng->random(4)];
        } else {
            treasureLvl = variationTop.treasure_level;
            itemClass = variationTop.item_class[grng->random(4)];
        }
        pItemTable->generateItem(treasureLvl, itemClass, &itemArray[i]);
        itemArray[i].SetIdentified();
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

void GUIWindow_MagicShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    ITEM_TREASURE_LEVEL treasureLvl = isSpecial ? magicShopVariationSpecial[houseId()] : magicShopVariationStandart[houseId()];

    for (int i = 0; i < itemAmountInShop[buildingType()]; i++) {
        pItemTable->generateItem(treasureLvl, 22, &itemArray[i]);
        itemArray[i].SetIdentified();
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

void GUIWindow_AlchemyShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    ITEM_TREASURE_LEVEL treasureLvl = isSpecial ? alchemyShopVariationSpecial[houseId()] : alchemyShopVariationStandart[houseId()];
    int bottomRowItemClass = isSpecial ? 44 : 45;

    for (int i = 0; i < itemAmountInShop[buildingType()]; i++) {
        if (i < 6) {
            itemArray[i].Reset();
            if (isSpecial) {
                itemArray[i].uItemID = grng->randomSample(allRecipeScrolls());
            } else {
                itemArray[i].uItemID = ITEM_POTION_BOTTLE;
            }
        } else {
            pItemTable->generateItem(treasureLvl, bottomRowItemClass, &itemArray[i]);
            itemArray[i].SetIdentified();
        }
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

std::vector<DIALOGUE_TYPE> GUIWindow_WeaponShop::listShopLearnableSkills() {
    std::vector<int> itemClasses;
    std::vector<DIALOGUE_TYPE> skillsOptions;

    addUniqueItemClasses(weaponShopVariationStandart[houseId()], itemClasses);
    addUniqueItemClasses(weaponShopVariationSpecial[houseId()], itemClasses);

    for (int itemClass : itemClasses) {
        DIALOGUE_TYPE dialogue = getSkillLearnDualogueForItemClass(itemClass);
        if (dialogue != DIALOGUE_NULL) {
            skillsOptions.push_back(dialogue);
        }
    }

    return skillsOptions;
}

std::vector<DIALOGUE_TYPE> GUIWindow_ArmorShop::listShopLearnableSkills() {
    std::vector<int> itemClasses;
    std::vector<DIALOGUE_TYPE> skillsOptions;

    addUniqueItemClasses(armorShopTopRowVariationStandart[houseId()], itemClasses);
    addUniqueItemClasses(armorShopBottomRowVariationStandart[houseId()], itemClasses);
    addUniqueItemClasses(armorShopTopRowVariationSpecial[houseId()], itemClasses);
    addUniqueItemClasses(armorShopBottomRowVariationSpecial[houseId()], itemClasses);

    for (int itemClass : itemClasses) {
        DIALOGUE_TYPE dialogue = getSkillLearnDualogueForItemClass(itemClass);
        if (dialogue != DIALOGUE_NULL) {
            skillsOptions.push_back(dialogue);
        }
    }

    return skillsOptions;
}

std::vector<DIALOGUE_TYPE> GUIWindow_MagicShop::listShopLearnableSkills() {
    return {DIALOGUE_LEARN_ITEM_ID, DIALOGUE_LEARN_REPAIR};
}

std::vector<DIALOGUE_TYPE> GUIWindow_AlchemyShop::listShopLearnableSkills() {
    return {DIALOGUE_LEARN_ALCHEMY, DIALOGUE_LEARN_MONSTER_ID};
}

void GUIWindow_Shop::houseSpecificDialogue() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_SHOP_BUY_STANDARD:
        shopWaresDialogue(false);
        break;
      case DIALOGUE_SHOP_BUY_SPECIAL:
        shopWaresDialogue(true);
        break;
      case DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
        displayEquipmentDialogue();
        break;
      case DIALOGUE_SHOP_SELL:
        sellDialogue();
        break;
      case DIALOGUE_SHOP_IDENTIFY:
        identifyDialogue();
        break;
      case DIALOGUE_SHOP_REPAIR:
        repairDialogue();
        break;
      case DIALOGUE_LEARN_SKILLS:
        learnSkillsDialogue(colorTable.Sunflower);
        break;
      default:
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        break;
    }
}

void GUIWindow_Shop::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    _currentDialogue = option;
    if (option == DIALOGUE_SHOP_BUY_STANDARD || option == DIALOGUE_SHOP_BUY_SPECIAL) {
        if (pParty->PartyTimes.shopNextRefreshTime[houseId()] < pParty->GetPlayingTime()) {
            generateShopItems(false);
            generateShopItems(true);
            GameTime nextGenTime = pParty->GetPlayingTime() + GameTime::FromDays(buildingTable[houseId()].generation_interval_days);
            pParty->PartyTimes.shopNextRefreshTime[houseId()] = nextGenTime;
        }

        BuildingType shopType = buildingType();
        const std::array<ItemGen, 12> &itemArray = (option == DIALOGUE_SHOP_BUY_STANDARD) ? pParty->standartItemsInShops[houseId()] : pParty->specialItemsInShops[houseId()];
        for (int i = 0; i < itemAmountInShop[shopType]; ++i) {
            if (itemArray[i].uItemID != ITEM_NULL) {
                shop_ui_items_in_store[i] = assets->getImage_ColorKey(itemArray[i].GetIconName());
            }
        }
        if (shopType == BUILDING_WEAPON_SHOP) {
            for (int i = 0; i < itemAmountInShop[shopType]; ++i) {
                if (itemArray[i].uItemID != ITEM_NULL) {
                    // Note that we're using grng here for a reason - we want recorded mouse clicks to work.
                    weaponYPos[i] = grng->random(300 - shop_ui_items_in_store[i]->height());
                }
            }
        }
    } else if (option == DIALOGUE_SHOP_SELL || option == DIALOGUE_SHOP_IDENTIFY || option == DIALOGUE_SHOP_REPAIR) {
        pParty->placeHeldItemInInventoryOrDrop();
    } else if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

std::vector<DIALOGUE_TYPE> GUIWindow_Shop::listDialogueOptions() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        return {DIALOGUE_SHOP_BUY_STANDARD, DIALOGUE_SHOP_BUY_SPECIAL, DIALOGUE_SHOP_DISPLAY_EQUIPMENT, DIALOGUE_LEARN_SKILLS};
      case DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
        return {DIALOGUE_SHOP_SELL, DIALOGUE_SHOP_IDENTIFY, DIALOGUE_SHOP_REPAIR};
      case DIALOGUE_LEARN_SKILLS:
        return listShopLearnableSkills();
      default:
        return {};
    }
}

std::vector<DIALOGUE_TYPE> GUIWindow_AlchemyShop::listDialogueOptions() {
    if (_currentDialogue == DIALOGUE_SHOP_DISPLAY_EQUIPMENT) {
        return {DIALOGUE_SHOP_SELL, DIALOGUE_SHOP_IDENTIFY};
    }
    return GUIWindow_Shop::listDialogueOptions();
}

void GUIWindow_Shop::updateDialogueOnEscape() {
    if (IsSkillLearningDialogue(_currentDialogue)) {
        _currentDialogue = DIALOGUE_LEARN_SKILLS;
        return;
    }
    if (_currentDialogue == DIALOGUE_SHOP_SELL || _currentDialogue == DIALOGUE_SHOP_IDENTIFY || _currentDialogue == DIALOGUE_SHOP_REPAIR) {
        _currentDialogue = DIALOGUE_SHOP_DISPLAY_EQUIPMENT;
        return;
    }
    if (_currentDialogue == DIALOGUE_MAIN) {
        _currentDialogue = DIALOGUE_NULL;
        return;
    }
    _currentDialogue = DIALOGUE_MAIN;
}

void GUIWindow_Shop::playHouseGoodbyeSpeech() {
    bool rudeReaction = true;

    if (pParty->PartyTimes.shopBanTimes[houseId()] <= pParty->GetPlayingTime()) {
        if (pParty->GetGold() <= 10000) {
            if (_transactionPerformed) {
                playHouseSound(houseId(), HOUSE_SOUND_SHOP_GOODBYE_POLITE);
            }
            return;
        }
        playHouseSound(houseId(), _transactionPerformed ? HOUSE_SOUND_SHOP_GOODBYE_POLITE : HOUSE_SOUND_SHOP_GOODBYE_RUDE);
        rudeReaction = !_transactionPerformed;
    }
    if (rudeReaction && !pParty->_delayedReactionTimer) {
        int id = pParty->getRandomActiveCharacterId(vrng.get());

        if (id != -1) {
            pParty->setDelayedReaction(SPEECH_SHOP_RUDE, id);
            return;
        }
    }
}

// Alchemy shop is special. Instead of polite and rude goodbye dialogue it have
// polite and enthusiastic ones.
void GUIWindow_AlchemyShop::playHouseGoodbyeSpeech() {
    if (pParty->PartyTimes.shopBanTimes[houseId()] <= pParty->GetPlayingTime()) {
        playHouseSound(houseId(), _transactionPerformed ? HOUSE_SOUND_ALCHEMY_SHOP_GOODBYE_BOUGHT : HOUSE_SOUND_ALCHEMY_SHOP_GOODBYE_REGULAR);
    } else if (!pParty->_delayedReactionTimer) {
        int id = pParty->getRandomActiveCharacterId(vrng.get());

        if (id != -1) {
            pParty->setDelayedReaction(SPEECH_SHOP_RUDE, id);
        }
    }
}

void GUIWindow_Shop::houseScreenClick() {
    if (current_screen_type == SCREEN_SHOP_INVENTORY) {
        pParty->activeCharacter().OnInventoryLeftClick();
        return;
    }

    if (!checkIfPlayerCanInteract()) {
        pAudioPlayer->playUISound(SOUND_error);
        return;
    }

    Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

    switch (_currentDialogue) {
        case DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            pParty->activeCharacter().OnInventoryLeftClick();
            break;
        }

        case DIALOGUE_SHOP_SELL: {
            int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462) {
                return;
            }

            int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
            if (!pItemID) {
                return;
            }

            if (pParty->activeCharacter().pInventoryItemList[pItemID - 1].MerchandiseTest(houseId())) {
                _transactionPerformed = true;
                pParty->activeCharacter().SalesProcess(invindex, pItemID - 1, houseId());
                render->ClearZBuffer();
                pParty->activeCharacter().playReaction(SPEECH_ITEM_SOLD);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_WRONG_SHOP);
            pAudioPlayer->playUISound(SOUND_error);
            break;
        }

        case DIALOGUE_SHOP_IDENTIFY: {
            int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462) {
                return;
            }

            int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
            if (!pItemID) {
                return;
            }

            float fPriceMultiplier = buildingTable[houseId()].fPriceMultiplier;
            int uPriceItemService = PriceCalculator::itemIdentificationPriceForPlayer(&pParty->activeCharacter(), fPriceMultiplier);
            ItemGen &item = pParty->activeCharacter().pInventoryItemList[pItemID - 1];

            if (!(item.uAttributes & ITEM_IDENTIFIED)) {
                if (item.MerchandiseTest(houseId())) {
                    if (pParty->GetGold() >= uPriceItemService) {
                        _transactionPerformed = true;
                        pParty->TakeGold(uPriceItemService);
                        item.uAttributes |= ITEM_IDENTIFIED;
                        pParty->activeCharacter().playReaction(SPEECH_SHOP_IDENTIFY);
                        engine->_statusBar->setEvent(LSTR_DONE);
                        return;
                    }

                    playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                    engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
                    return;
                }

                pAudioPlayer->playUISound(SOUND_error);
                pParty->activeCharacter().playReaction(SPEECH_WRONG_SHOP);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_ALREADY_INDENTIFIED);
            break;
        }

        case DIALOGUE_SHOP_REPAIR: {
            int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462) {
                return;
            }

            int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
            if (!pItemID) {
                return;
            }

            ItemGen &item = pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            float fPriceMultiplier = buildingTable[houseId()].fPriceMultiplier;
            int uPriceItemService = PriceCalculator::itemRepairPriceForPlayer(&pParty->activeCharacter(), item.GetValue(), fPriceMultiplier);

            if (item.uAttributes & ITEM_BROKEN) {
                if (item.MerchandiseTest(houseId())) {
                    if (pParty->GetGold() >= uPriceItemService) {
                        _transactionPerformed = true;
                        pParty->TakeGold(uPriceItemService);
                        item.uAttributes = (item.uAttributes & ~ITEM_BROKEN) | ITEM_IDENTIFIED;
                        pParty->activeCharacter().playReaction(SPEECH_SHOP_REPAIR);
                        engine->_statusBar->setEvent(LSTR_GOOD_AS_NEW);
                        return;
                    }

                    playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                    engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
                    return;
                }

                pAudioPlayer->playUISound(SOUND_error);
                pParty->activeCharacter().playReaction(SPEECH_WRONG_SHOP);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_ALREADY_INDENTIFIED);
            break;
        }

        case DIALOGUE_SHOP_BUY_STANDARD:
        case DIALOGUE_SHOP_BUY_SPECIAL: {
            int testx;
            int testpos;
            ItemGen *boughtItem = nullptr;

            switch (buildingType()) {
              case BUILDING_WEAPON_SHOP:
                testx = (pt.x - 30) / 70;
                if (testx >= 0 && testx < 6) {
                    if (_currentDialogue == DIALOGUE_SHOP_BUY_STANDARD)
                        boughtItem = &pParty->standartItemsInShops[houseId()][testx];
                    else
                        boughtItem = &pParty->specialItemsInShops[houseId()][testx];

                    if (boughtItem->uItemID != ITEM_NULL) {
                        testpos = ((60 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 70);
                        if (pt.x >= testpos && pt.x < (testpos + shop_ui_items_in_store[testx]->width())) {
                            if (pt.y >= weaponYPos[testx] + 30 && pt.y < (weaponYPos[testx] + 30 + shop_ui_items_in_store[testx]->height())) {
                                break;  // good
                            }
                        }
                    }
                }
                return;

              case BUILDING_ARMOR_SHOP:
                testx = (pt.x - 40) / 105;
                if (testx >= 0 && testx < 4) {
                    if (pt.y >= 126) {
                        testx += 4;
                    }

                    if (_currentDialogue == DIALOGUE_SHOP_BUY_STANDARD)
                        boughtItem = &pParty->standartItemsInShops[houseId()][testx];
                    else
                        boughtItem = &pParty->specialItemsInShops[houseId()][testx];

                    if (boughtItem->uItemID != ITEM_NULL) {
                        if (testx >= 4) {
                            testpos = ((90 - (shop_ui_items_in_store[testx]->width() / 2)) + (testx * 105) - 420);  // low row
                        } else {
                            testpos = ((86 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 105);
                        }

                        if (pt.x >= testpos && pt.x <= testpos + shop_ui_items_in_store[testx]->width()) {
                            if ((pt.y >= 126 && pt.y < (126 + shop_ui_items_in_store[testx]->height())) ||
                                (pt.y <= 98 && pt.y >= (98 - shop_ui_items_in_store[testx]->height()))) {
                                break;  // good
                            }
                        }
                    }
                }
                return;

              case BUILDING_ALCHEMY_SHOP:
              case BUILDING_MAGIC_SHOP:
                testx = (pt.x) / 75;
                if (testx >= 0 && testx < 6) {
                    if (pt.y > 152) {
                        testx += 6;
                    }

                    if (_currentDialogue == DIALOGUE_SHOP_BUY_STANDARD)
                        boughtItem = &pParty->standartItemsInShops[houseId()][testx];
                    else
                        boughtItem = &pParty->specialItemsInShops[houseId()][testx];

                    if (boughtItem->uItemID != ITEM_NULL) {
                        if (pt.y > 152) {
                            testpos = 75 * testx - shop_ui_items_in_store[testx]->width() / 2 + 40 - 450;
                        } else {
                            testpos = 75 * testx - shop_ui_items_in_store[testx]->width() / 2 + 40;
                        }

                        if (pt.x >= testpos && pt.x <= testpos + shop_ui_items_in_store[testx]->width()) {
                            if ((pt.y <= 308 && pt.y >= (308 - shop_ui_items_in_store[testx]->height())) ||
                                (pt.y <= 152 && pt.y >= (152 - shop_ui_items_in_store[testx]->height()))) {
                                // y is 152-h to 152 or 308-height to 308
                                break;  // good
                            }
                        }
                    }
                }
                return;

              default:
                return;
            }

            float fPriceMultiplier = buildingTable[houseId()].fPriceMultiplier;
            int uPriceItemService = PriceCalculator::itemBuyingPriceForPlayer(&pParty->activeCharacter(), boughtItem->GetValue(), fPriceMultiplier);
            int stealResult = 0;
            int stealDifficulty = 0;
            int fine;
            if (pMapStats->GetMapInfo(pCurrentMapName) != MAP_INVALID) {
                stealDifficulty = pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)]._steal_perm;
            }
            int partyReputation = pParty->GetPartyReputation();
            if (isStealingModeActive()) {
                stealResult = pParty->activeCharacter().StealFromShop(boughtItem, stealDifficulty, partyReputation, 0, &fine);
                if (!stealResult) {
                    // caught stealing no item
                    processStealingResult(0, fine);
                    return;
                }
            } else if (pParty->GetGold() < uPriceItemService) {
                playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
                return;
            }

            int itemSlot = pParty->activeCharacter().AddItem(-1, boughtItem->uItemID);
            if (itemSlot) {
                boughtItem->SetIdentified();
                pParty->activeCharacter().pInventoryItemList[itemSlot - 1] = *boughtItem;
                if (stealResult != 0) {  // stolen
                    pParty->activeCharacter().pInventoryItemList[itemSlot - 1].SetStolen();
                    processStealingResult(stealResult, fine);
                } else {
                    _transactionPerformed = true;
                    pParty->TakeGold(uPriceItemService);
                }
                boughtItem->Reset();
                render->ClearZBuffer();
                pParty->activeCharacter().playReaction(SPEECH_ITEM_BUY);
                return;
            } else {
                pParty->activeCharacter().playReaction(SPEECH_NO_ROOM);
                engine->_statusBar->setEvent(LSTR_INVENTORY_IS_FULL);
                return;
            }
            break;
        }

        default:
            assert(false);
            break;
    }
}

void GUIWindow_Shop::processStealingResult(int stealingResult, int fineToAdd) {  // not working properly??
    int reputationDelta = 0;

    if (stealingResult == 0 || stealingResult == 1) {  // got caught
        pParty->uFine = std::clamp(pParty->uFine + fineToAdd, 0, 4000000);
        if (pParty->uFine) {
            for (Character &player : pParty->pCharacters) {
                if (!player._achievedAwardsBits[Award_Fine]) {
                    player._achievedAwardsBits.set(Award_Fine);
                }
            }
        }
        if (stealingResult == 1)
            reputationDelta = 2;
        else
            reputationDelta = 1;

        // only ban when caught
        pParty->PartyTimes.shopBanTimes[houseId()] = pParty->GetPlayingTime().AddDays(1);
    } else {
        reputationDelta = 2;
    }

    pParty->InTheShopFlags[std::to_underlying(houseId())] = 1;

    LocationInfo &loc = currentLocationInfo();
    loc.reputation += reputationDelta;
    if (loc.reputation > 10000)
        loc.reputation = 10000;
}
