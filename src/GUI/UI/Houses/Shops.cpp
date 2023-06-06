#include <cstdlib>
#include <algorithm>
#include <string>

#include "Engine/Engine.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Graphics/Viewport.h"
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
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/Houses/Shops.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

#include "Utility/IndexedArray.h"

struct ITEM_VARIATION {
    ITEM_TREASURE_LEVEL treasure_level;
    uint16_t item_class[4];
};

// TODO(Nik-RE-dev): enumerate treasure types used in this file

const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_WEAPON_SHOP, HOUSE_LAST_WEAPON_SHOP> weaponShopVariationStandart = {{
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

const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_WEAPON_SHOP, HOUSE_LAST_WEAPON_SHOP> weaponShopVariationSpecial = {{
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

const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopTopRowVariationStandart = {{
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

const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopBottomRowVariationStandart = {{
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

const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopTopRowVariationSpecial = {{
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

const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopBottomRowVariationSpecial = {{
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

const IndexedArray<ITEM_TREASURE_LEVEL, HOUSE_FIRST_MAGIC_SHOP, HOUSE_LAST_MAGIC_SHOP> magicShopVariationStandart = {{
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

const IndexedArray<ITEM_TREASURE_LEVEL, HOUSE_FIRST_MAGIC_SHOP, HOUSE_LAST_MAGIC_SHOP> magicShopVariationSpecial = {{
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

const IndexedArray<ITEM_TREASURE_LEVEL, HOUSE_FIRST_ALCHEMY_SHOP, HOUSE_LAST_ALCHEMY_SHOP> alchemyShopVariationStandart = {{
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

const IndexedArray<ITEM_TREASURE_LEVEL, HOUSE_FIRST_ALCHEMY_SHOP, HOUSE_LAST_ALCHEMY_SHOP> alchemyShopVariationSpecial = {{
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

void GUIWindow_Shop::mainDialogue() {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (HouseUI_CheckIfPlayerCanInteract()) {
        pShopOptions[0] = localization->GetString(LSTR_STANDARD);
        pShopOptions[1] = localization->GetString(LSTR_SPECIAL);
        pShopOptions[2] = localization->GetString(LSTR_DISPLAY);
        pShopOptions[3] = localization->GetString(LSTR_LEARN_SKILLS);

        int all_text_height = 0;
        for (int i = 0; i < 4; ++i)
            all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], dialogwin.uFrameWidth, 0);

        int textspacings = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / 4;
        int textoffset = SIDE_TEXT_BOX_BODY_TEXT_OFFSET - (textspacings / 2);
        int pNumString = 0;

        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
            GUIButton *pButton = pDialogueWindow->GetControl(i);
            pButton->uY = textspacings + textoffset;
            pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], dialogwin.uFrameWidth, 0);
            textoffset = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], dialogwin.uFrameWidth, 0) - 1;
            pButton->uW = textoffset + 6;

            Color pColorText = colorTable.Jonquil;
            if (pDialogueWindow->pCurrentPosActiveItem != i) {
                pColorText = colorTable.White;
            }

            dialogwin.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
            ++pNumString;
        }
    }
}

void GUIWindow_Shop::displayEquipmentDialogue() {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    pShopOptions[0] = localization->GetString(LSTR_SELL);
    pShopOptions[1] = localization->GetString(LSTR_IDENTIFY);
    pShopOptions[2] = localization->GetString(LSTR_REPAIR);

    int options = (buildingTable[wData.val - 1].uType == BuildingType_AlchemistShop) ? 2 : 3;
    int all_text_height = 0;
    for (int i = 0; i < options; ++i)
        all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], dialogwin.uFrameWidth, 0);

    int textspacings = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / options;
    int textoffset = SIDE_TEXT_BOX_BODY_TEXT_OFFSET - (textspacings / 2);
    int pNumString = 0;

    for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
        GUIButton *pButton = pDialogueWindow->GetControl(i);
        pButton->uY = textspacings + textoffset;
        pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], dialogwin.uFrameWidth, 0);
        textoffset = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], dialogwin.uFrameWidth, 0) - 1;
        pButton->uW = textoffset + 6;

        Color pColorText = colorTable.Jonquil;
        if (pDialogueWindow->pCurrentPosActiveItem != i)
            pColorText = colorTable.White;
        dialogwin.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
        ++pNumString;
    }
}

void GUIWindow_Shop::sellDialogue() {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    if (HouseUI_CheckIfPlayerCanInteract()) {
        GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_SELL), Color());

        Pointi pt = dialogwin.mouse->GetCursorPos();

        int invindex = ((pt.x - 14) / 32) + 14 * ((pt.y - 17) / 32);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
        if (pItemID) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            MERCHANT_PHRASE phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingTable[wData.val - 1].uType, wData.val, 3);
            std::string str = BuildDialogueString(pMerchantsSellPhrases[phrases_id], pParty->activeCharacterIndex() - 1, item, wData.val, 3);
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

    if (HouseUI_CheckIfPlayerCanInteract()) {
        GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_IDENTIFY), Color());

        Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

        int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);

        if (pItemID) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];

            std::string str;
            if (!item->IsIdentified()) {
                MERCHANT_PHRASE phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingTable[wData.val - 1].uType, wData.val, 4);
                str = BuildDialogueString(pMerchantsIdentifyPhrases[phrases_id], pParty->activeCharacterIndex() - 1, item, wData.val, 4);
            } else {
                str = BuildDialogueString("%24", pParty->activeCharacterIndex() - 1, item, wData.val, 4);
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

    if (HouseUI_CheckIfPlayerCanInteract()) {
        GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_REPAIR), Color());

        Pointi pt = dialogwin.mouse->GetCursorPos();

        int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
        if (pItemID == 0)
            return;

        if (pParty->activeCharacter().pOwnItems[pItemID - 1].uAttributes & ITEM_BROKEN) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            MERCHANT_PHRASE phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingTable[wData.val - 1].uType, wData.val, 5);
            std::string str = BuildDialogueString(pMerchantsRepairPhrases[phrases_id], pParty->activeCharacterIndex() - 1, item, wData.val, 5);
            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            dialogwin.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.White, str, 3);
        }
    }
}

void GUIWindow_Shop::learnSkillsDialogue() {
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (!HouseUI_CheckIfPlayerCanInteract())
        return;

    int item_num = 0;
    int all_text_height = 0;
    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);

    for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
        auto skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
        if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE && !pParty->activeCharacter().pActiveSkills[skill]) {
            all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(skill), dialogwin.uFrameWidth, 0);
            item_num++;
        }
    }

    SkillTrainingDialogue(&dialogwin, item_num, all_text_height, pPrice);
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

    if (HouseUI_CheckIfPlayerCanInteract()) {
        int item_num = 0;
        for (int i = 0; i < 6; ++i) {
            item_num += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_STEAL_ITEM), Color());
        } else {
            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), Color());
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
                                MERCHANT_PHRASE phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, BuildingType_WeaponShop, wData.val, 2);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, item, wData.val, 2);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, item, wData.val, 2);
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

    if (HouseUI_CheckIfPlayerCanInteract()) {
        int pItemCount = 0;
        for (int i = 0; i < 6; ++i) {
            pItemCount += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_STEAL_ITEM), Color());
        } else {
            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), Color());
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
                                MERCHANT_PHRASE phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingTable[wData.val - 1].uType, wData.val, 2);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, item, wData.val, 2);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, item, wData.val, 2);
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

    if (HouseUI_CheckIfPlayerCanInteract()) {
        int item_num = 0;

        for (int i = 0; i < 12; ++i) {
            item_num += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_STEAL_ITEM), Color());
        } else {
            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), Color());
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
                                MERCHANT_PHRASE phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingTable[wData.val - 1].uType, wData.val, 2);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, item, wData.val, 2);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, item, wData.val, 2);
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

    for (int i = 0; i < itemAmountInShop[buildingTable[wData.val - 1].uType]; i++) {
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

    for (int i = 0; i < itemAmountInShop[buildingTable[wData.val - 1].uType]; i++) {
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

    for (int i = 0; i < itemAmountInShop[buildingTable[wData.val - 1].uType]; i++) {
        pItemTable->generateItem(treasureLvl, 22, &itemArray[i]);
        itemArray[i].SetIdentified();
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

void GUIWindow_AlchemyShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    ITEM_TREASURE_LEVEL treasureLvl = isSpecial ? alchemyShopVariationSpecial[houseId()] : alchemyShopVariationStandart[houseId()];
    int bottomRowItemClass = isSpecial ? 44 : 45;

    for (int i = 0; i < itemAmountInShop[buildingTable[wData.val - 1].uType]; i++) {
        if (i < 6) {
            itemArray[i].Reset();
            if (isSpecial) {
                itemArray[i].uItemID = grng->randomSample(recipeScrolls());
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

    for (int itemClass : weaponShopVariationStandart[houseId()].item_class) {
        if (std::find(itemClasses.begin(), itemClasses.end(), itemClass) == itemClasses.end()) {
            itemClasses.push_back(itemClass);
        }
    }
    for (int itemClass : weaponShopVariationSpecial[houseId()].item_class) {
        if (std::find(itemClasses.begin(), itemClasses.end(), itemClass) == itemClasses.end()) {
            itemClasses.push_back(itemClass);
        }
    }

    for (int itemClass : itemClasses) {
        switch (itemClass) {
          case 23:
            skillsOptions.push_back(DIALOGUE_LEARN_SWORD);
            break;
          case 24:
            skillsOptions.push_back(DIALOGUE_LEARN_DAGGER);
            break;
          case 25:
            skillsOptions.push_back(DIALOGUE_LEARN_AXE);
            break;
          case 26:
            skillsOptions.push_back(DIALOGUE_LEARN_SPEAR);
            break;
          case 27:
            skillsOptions.push_back(DIALOGUE_LEARN_BOW);
            break;
          case 28:
            skillsOptions.push_back(DIALOGUE_LEARN_MACE);
            break;
          case 30:
            skillsOptions.push_back(DIALOGUE_LEARN_STAFF);
            break;
          default:
            continue;
        }
    }

    return skillsOptions;
}

std::vector<DIALOGUE_TYPE> GUIWindow_ArmorShop::listShopLearnableSkills() {
    std::vector<int> itemClasses;
    std::vector<DIALOGUE_TYPE> skillsOptions;

    for (int itemClass : armorShopTopRowVariationStandart[houseId()].item_class) {
        if (std::find(itemClasses.begin(), itemClasses.end(), itemClass) == itemClasses.end()) {
            itemClasses.push_back(itemClass);
        }
    }
    for (int itemClass : armorShopBottomRowVariationStandart[houseId()].item_class) {
        if (std::find(itemClasses.begin(), itemClasses.end(), itemClass) == itemClasses.end()) {
            itemClasses.push_back(itemClass);
        }
    }
    for (int itemClass : armorShopTopRowVariationSpecial[houseId()].item_class) {
        if (std::find(itemClasses.begin(), itemClasses.end(), itemClass) == itemClasses.end()) {
            itemClasses.push_back(itemClass);
        }
    }
    for (int itemClass : armorShopBottomRowVariationSpecial[houseId()].item_class) {
        if (std::find(itemClasses.begin(), itemClasses.end(), itemClass) == itemClasses.end()) {
            itemClasses.push_back(itemClass);
        }
    }

    for (int itemClass : itemClasses) {
        switch (itemClass) {
          case 31:
            skillsOptions.push_back(DIALOGUE_LEARN_LEATHER);
            break;
          case 32:
            skillsOptions.push_back(DIALOGUE_LEARN_CHAIN);
            break;
          case 33:
            skillsOptions.push_back(DIALOGUE_LEARN_PLATE);
            break;
          case 34:
            skillsOptions.push_back(DIALOGUE_LEARN_SHIELD);
            break;
          default:
            continue;
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
    switch (dialog_menu_id) {
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
        learnSkillsDialogue();
        break;
      default:
        break;
    }
}

void GUIWindow_Shop::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    if (option == DIALOGUE_SHOP_BUY_STANDARD || option == DIALOGUE_SHOP_BUY_SPECIAL) {
        if (pParty->PartyTimes.shopNextRefreshTime[houseId()] < pParty->GetPlayingTime()) {
            generateShopItems(false);
            generateShopItems(true);
            GameTime nextGenTime = pParty->GetPlayingTime() + GameTime::FromDays(buildingTable[wData.val - 1].generation_interval_days);
            pParty->PartyTimes.shopNextRefreshTime[houseId()] = nextGenTime;
        }

        const std::array<ItemGen, 12> &itemArray = (option == DIALOGUE_SHOP_BUY_STANDARD) ? pParty->standartItemsInShops[houseId()] : pParty->specialItemsInShops[houseId()];
        for (int i = 0; i < itemAmountInShop[buildingTable[wData.val - 1].uType]; ++i) {
            if (itemArray[i].uItemID != ITEM_NULL) {
                shop_ui_items_in_store[i] = assets->getImage_ColorKey(itemArray[i].GetIconName());
            }
        }
        if (buildingTable[wData.val - 1].uType == BuildingType_WeaponShop) {
            for (int i = 0; i < itemAmountInShop[buildingTable[wData.val - 1].uType]; ++i) {
                if (itemArray[i].uItemID != ITEM_NULL) {
                    // Note that we're using grng here for a reason - we want recorded mouse clicks to work.
                    weaponYPos[i] = grng->random(300 - shop_ui_items_in_store[i]->height());
                }
            }
        }
    } else if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

std::vector<DIALOGUE_TYPE> GUIWindow_Shop::listDialogueOptions(DIALOGUE_TYPE option) {
    switch (dialog_menu_id) {
      case DIALOGUE_MAIN:
        return {DIALOGUE_SHOP_BUY_STANDARD, DIALOGUE_SHOP_BUY_SPECIAL, DIALOGUE_SHOP_DISPLAY_EQUIPMENT, DIALOGUE_LEARN_SKILLS};
      case DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
        if (buildingTable[wData.val - 1].uType == BuildingType_AlchemistShop) {
            return {DIALOGUE_SHOP_SELL, DIALOGUE_SHOP_IDENTIFY};
        } else {
            return {DIALOGUE_SHOP_SELL, DIALOGUE_SHOP_IDENTIFY, DIALOGUE_SHOP_REPAIR};
        }
      case DIALOGUE_LEARN_SKILLS:
        return listShopLearnableSkills();
      default:
        return {};
    }
}

void GUIWindow_Shop::processStealingResult(int stealingResult, int fineToAdd) {  // not working properly??
    int reputationDelta = 0;

    if (stealingResult == 0 || stealingResult == 1) {  // got caught
        pParty->uFine = std::clamp(pParty->uFine + fineToAdd, 0, 4000000);
        if (pParty->uFine) {
            for (Player &player : pParty->pPlayers) {
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

//----- (004BDB56) --------------------------------------------------------
void UIShop_Buy_Identify_Repair() {
    unsigned int pItemID;  // esi@20
    ItemGen *item;         // esi@21

    // int v18;                   // ecx@37
    float pPriceMultiplier;    // ST1C_4@38
    int taken_item;            // eax@40
    ItemGen *bought_item = nullptr;      // esi@51
    int party_reputation;      // eax@55
    int v39;                   // eax@63
    uint16_t *pSkill;  // esi@77
    int v55;                   // [sp+0h] [bp-B4h]@26
    int a6;                    // [sp+98h] [bp-1Ch]@57
    int a3;                    // [sp+9Ch] [bp-18h]@53
    unsigned int uNumSeconds;  // [sp+A4h] [bp-10h]@53
    unsigned int invindex;     // [sp+A8h] [bp-Ch]@9
    int uPriceItemService;     // [sp+ACh] [bp-8h]@12

    if (current_screen_type == CURRENT_SCREEN::SCREEN_SHOP_INVENTORY) {
        pParty->activeCharacter().OnInventoryLeftClick();
        return;
    }

    if (!HouseUI_CheckIfPlayerCanInteract()) {
        pAudioPlayer->playUISound(SOUND_error);
        return;
    }

    Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

    switch (dialog_menu_id) {
        case DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            pParty->activeCharacter().OnInventoryLeftClick();
            break;
        }

        case DIALOGUE_GUILD_BUY_BOOKS: {
            int testx = (pt.x - 32) / 70;
            if (testx >= 0 && testx < 6) {
                if (pt.y >= 250) {
                    testx += 6;
                }

                bought_item = &pParty->spellBooksInGuilds[window_SpeakInHouse->houseId()][testx];
                if (bought_item->uItemID != ITEM_NULL) {
                    int testpos;
                    if (pt.y >= 250) {
                        testpos = 32 + 70 * testx - 420;
                    } else {
                        testpos = 32 + 70 * testx;
                    }

                    if (pt.x >= testpos && pt.x <= testpos + shop_ui_items_in_store[testx]->width()) {
                        if ((pt.y >= 90 && pt.y <= (90 + shop_ui_items_in_store[testx]->height())) ||
                            (pt.y >= 250 && pt.y <= (250 + shop_ui_items_in_store[testx]->height()))) {
                            pPriceMultiplier = buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier;
                            uPriceItemService = PriceCalculator::itemBuyingPriceForPlayer(&pParty->activeCharacter(),
                                                                                          bought_item->GetValue(), pPriceMultiplier);

                            if (pParty->GetGold() < uPriceItemService) {
                                PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)2);
                                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                                return;
                            }

                            taken_item = pParty->activeCharacter().AddItem(-1, bought_item->uItemID);
                            if (taken_item) {
                                bought_item->SetIdentified();
                                pParty->activeCharacter().pInventoryItemList[taken_item - 1] = *bought_item;
                                dword_F8B1E4 = 1;
                                pParty->TakeGold(uPriceItemService);
                                bought_item->Reset();
                                render->ClearZBuffer();
                                pParty->activeCharacter().playReaction(SPEECH_ItemBuy);
                                return;
                            }

                            pParty->activeCharacter().playReaction(SPEECH_NoRoom);
                            GameUI_SetStatusBar(LSTR_INVENTORY_IS_FULL);
                            break;
                        }
                    }
                }
            }

            return;  // no item
            break;
        }

        case DIALOGUE_SHOP_SELL: {
            invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462 ||
                (pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex),
                 !pItemID))
                return;

            if (pParty->activeCharacter()
                    .pInventoryItemList[pItemID - 1]
                    .MerchandiseTest(window_SpeakInHouse->wData.val)) {
                dword_F8B1E4 = 1;
                pParty->activeCharacter().SalesProcess(
                    invindex, pItemID - 1, window_SpeakInHouse->wData.val);
                render->ClearZBuffer();
                pParty->activeCharacter().playReaction(SPEECH_ItemSold);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_WrongShop);
            pAudioPlayer->playUISound(SOUND_error);
            break;
        }

        case DIALOGUE_SHOP_IDENTIFY: {
            invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462 ||
                (pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex),
                 !pItemID))
                return;

            uPriceItemService = PriceCalculator::itemIdentificationPriceForPlayer(
                &pParty->activeCharacter(), buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
            item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];

            if (!(item->uAttributes & ITEM_IDENTIFIED)) {
                if (item->MerchandiseTest(window_SpeakInHouse->wData.val)) {
                    if (pParty->GetGold() >= uPriceItemService) {
                        dword_F8B1E4 = 1;
                        pParty->TakeGold(uPriceItemService);
                        item->uAttributes |= ITEM_IDENTIFIED;
                        pParty->activeCharacter().playReaction(SPEECH_ShopIdentify);
                        GameUI_SetStatusBar(LSTR_DONE);
                        return;
                    }

                    PlayHouseSound(window_SpeakInHouse->wData.val,
                                   (HouseSoundID)2);
                    return;
                }

                pAudioPlayer->playUISound(SOUND_error);
                pParty->activeCharacter().playReaction(SPEECH_WrongShop);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_AlreadyIdentified);
            break;
        }

        case DIALOGUE_SHOP_REPAIR: {
            invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462 ||
                (pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(
                         invindex),
                 !pItemID))
                return;

            item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            pPriceMultiplier =
                buildingTable[window_SpeakInHouse->wData.val - 1]
                    .fPriceMultiplier;
            uPriceItemService = PriceCalculator::itemRepairPriceForPlayer(&pParty->activeCharacter(), item->GetValue(), pPriceMultiplier);

            if (item->uAttributes & ITEM_BROKEN) {
                if (item->MerchandiseTest(window_SpeakInHouse->wData.val)) {
                    if (pParty->GetGold() >= uPriceItemService) {
                        dword_F8B1E4 = 1;
                        pParty->TakeGold(uPriceItemService);
                        item->uAttributes =
                            (item->uAttributes & ~ITEM_BROKEN) | ITEM_IDENTIFIED;
                        pParty->activeCharacter().playReaction(SPEECH_ShopRepair);
                        GameUI_SetStatusBar(LSTR_GOOD_AS_NEW);
                        return;
                    }

                    PlayHouseSound(window_SpeakInHouse->wData.val,
                                   (HouseSoundID)2);
                    return;
                }

                pAudioPlayer->playUISound(SOUND_error);
                pParty->activeCharacter().playReaction(SPEECH_WrongShop);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_AlreadyIdentified);
            break;
        }

        case DIALOGUE_SHOP_BUY_STANDARD:
        case DIALOGUE_SHOP_BUY_SPECIAL: {
            int testx;
            int testpos;

            switch (in_current_building_type) {
                case BuildingType_WeaponShop:

                    testx = (pt.x - 30) / 70;
                    if (testx >= 0 && testx < 6) {
                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            bought_item = &pParty->standartItemsInShops[window_SpeakInHouse->houseId()][testx];
                        else
                            bought_item = &pParty->specialItemsInShops[window_SpeakInHouse->houseId()][testx];

                        if (bought_item->uItemID != ITEM_NULL) {
                            testpos =
                                ((60 -
                                  ((signed int) shop_ui_items_in_store[testx]->width() /
                                   2)) +
                                 testx * 70);
                            if (pt.x >= testpos &&
                                pt.x <
                                    (testpos +
                                     (signed int) shop_ui_items_in_store[testx]->width())) {
                                if (pt.y >= weaponYPos[testx] + 30 &&
                                    pt.y < (weaponYPos[testx] + 30 + shop_ui_items_in_store[testx]->height())) {
                                    break;  // good
                                }
                            } else {
                                bought_item = nullptr;
                                return;
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                case BuildingType_ArmorShop:

                    testx = (pt.x - 40) / 105;
                    if (testx >= 0 && testx < 4) {
                        if (pt.y >= 126) {
                            testx += 4;
                        }

                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            bought_item = &pParty->standartItemsInShops[window_SpeakInHouse->houseId()][testx];
                        else
                            bought_item = &pParty->specialItemsInShops[window_SpeakInHouse->houseId()][testx];

                        if (bought_item->uItemID != ITEM_NULL) {
                            if (testx >= 4) {
                                testpos = ((90 - (shop_ui_items_in_store[testx]->width() /
                                                  2)) +
                                           (testx * 105) - 420);  // low row
                            } else {
                                testpos = ((86 - (shop_ui_items_in_store[testx]->width() /
                                                  2)) +
                                           testx * 105);
                            }

                            if (pt.x >= testpos &&
                                pt.x <=
                                    testpos + shop_ui_items_in_store[testx]->width()) {
                                if ((pt.y >= 126 &&
                                    pt.y <
                                         (126 + shop_ui_items_in_store[testx]->height())) ||
                                    (pt.y <= 98 &&
                                        pt.y >=
                                         (98 - shop_ui_items_in_store[testx]->height()))) {
                                    break;  // good
                                }
                            } else {
                                bought_item = nullptr;
                                return;
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                case BuildingType_AlchemistShop:
                case BuildingType_MagicShop:

                    testx = (pt.x) / 75;
                    if (testx >= 0 && testx < 6) {
                        if (pt.y > 152) {
                            testx += 6;
                        }

                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            bought_item = &pParty->standartItemsInShops[window_SpeakInHouse->houseId()][testx];
                        else
                            bought_item = &pParty->specialItemsInShops[window_SpeakInHouse->houseId()][testx];

                        if (bought_item->uItemID != ITEM_NULL) {
                            if (pt.y > 152) {
                                testpos =
                                    75 * testx - shop_ui_items_in_store[testx]->width() /
                                        2 +
                                    40 - 450;
                            } else {
                                testpos =
                                    75 * testx - shop_ui_items_in_store[testx]->width() /
                                        2 +
                                    40;
                            }

                            if (pt.x >= testpos &&
                                pt.x <=
                                    testpos + shop_ui_items_in_store[testx]->width()) {
                                if ((pt.y <= 308 &&
                                    pt.y >=
                                         (308 - shop_ui_items_in_store[testx]->height())) ||
                                    (pt.y <= 152 &&
                                        pt.y >=
                                         (152 - shop_ui_items_in_store[testx]->height()))) {
                                    // y is 152-h to 152 or 308-height to 308
                                    break;  // good
                                }
                            } else {
                                bought_item = nullptr;
                                return;
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                default:
                    return;
            }

            uPriceItemService = PriceCalculator::itemBuyingPriceForPlayer(&pParty->activeCharacter(), bought_item->GetValue(),
                                                                          buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
            uNumSeconds = 0;
            a3 = 0;
            if (pMapStats->GetMapInfo(pCurrentMapName))
                a3 = pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)]
                         ._steal_perm;
            party_reputation = pParty->GetPartyReputation();
            if (isStealingModeActive()) {
                uNumSeconds = pParty->activeCharacter().StealFromShop(bought_item, a3, party_reputation, 0, &a6);
                if (!uNumSeconds) {
                    // caught stealing no item
                    ((GUIWindow_Shop*)window_SpeakInHouse)->processStealingResult(0, a6);
                    return;
                }
            } else if (pParty->GetGold() < uPriceItemService) {
                PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)2);
                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                return;
            }

            v39 = pParty->activeCharacter().AddItem(-1, bought_item->uItemID);
            if (v39) {
                bought_item->SetIdentified();
                pParty->activeCharacter().pInventoryItemList[v39 - 1] = *bought_item;
                if (uNumSeconds != 0) {  // stolen
                    pParty->activeCharacter().pInventoryItemList[v39 - 1].SetStolen();
                    ((GUIWindow_Shop*)window_SpeakInHouse)->processStealingResult(uNumSeconds, a6);
                } else {
                    dword_F8B1E4 = 1;
                    pParty->TakeGold(uPriceItemService);
                }
                bought_item->Reset();
                render->ClearZBuffer();
                pParty->activeCharacter().playReaction(SPEECH_ItemBuy);
                return;
            } else {
                pParty->activeCharacter().playReaction(SPEECH_NoRoom);
                GameUI_SetStatusBar(LSTR_INVENTORY_IS_FULL);
                return;
            }
            break;
        }
        default:  // if click video screen in shop
        {
            if (IsSkillLearningDialogue(dialog_menu_id)) {
                PLAYER_SKILL_TYPE skill = GetLearningDialogueSkill(dialog_menu_id);
                uPriceItemService =
                    PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(),
                                                                                buildingTable[window_SpeakInHouse->wData.val - 1]);
                if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE) {
                    pSkill = &pParty->activeCharacter().pActiveSkills[skill];
                    if (!*pSkill) {
                        if (pParty->GetGold() < uPriceItemService) {
                            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                            if (in_current_building_type == BuildingType_Training)
                                v55 = HouseSound_Goodbye;
                            else
                                v55 = HouseSound_NotEnoughMoney;
                            PlayHouseSound(
                                window_SpeakInHouse->wData.val,
                                (HouseSoundID)v55);
                            return;
                        }
                        pParty->TakeGold(uPriceItemService);
                        dword_F8B1E4 = 1;
                        *pSkill = 1;
                        pParty->activeCharacter().playReaction(SPEECH_SkillLearned);
                        return;
                    }
                }
            }
            break;
        }
    }
}
