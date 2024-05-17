#include <array>
#include <algorithm>
#include <string>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Random/Random.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/CharacterFrameTable.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Party.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Time/Timer.h"
#include "Engine/Conditions.h"

#include "Io/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/Books/MapBook.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIChest.h"
#include "GUI/UI/Houses/Shops.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"

GraphicsImage *parchment = nullptr;
GraphicsImage *messagebox_corner_x = nullptr;       // 5076AC
GraphicsImage *messagebox_corner_y = nullptr;       // 5076B4
GraphicsImage *messagebox_corner_z = nullptr;       // 5076A8
GraphicsImage *messagebox_corner_w = nullptr;       // 5076B0
GraphicsImage *messagebox_border_top = nullptr;     // 507698
GraphicsImage *messagebox_border_bottom = nullptr;  // 5076A4
GraphicsImage *messagebox_border_left = nullptr;    // 50769C
GraphicsImage *messagebox_border_right = nullptr;   // 5076A0

bool holdingMouseRightButton = false;
bool rightClickItemActionPerformed = false;
bool identifyOrRepairReactionPlayed = false;

struct stat_coord {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
};

std::array<stat_coord, 26> stat_string_coord =  // 4E2940
{{
     {0x1A, 0x39, 0xDC, 0x12},   {0x1A, 0x4A, 0xDC, 0x12},
     {0x1A, 0x5B, 0xDC, 0x12},   {0x1A, 0x6C, 0xDC, 0x12},
     {0x1A, 0x7D, 0xDC, 0x12},   {0x1A, 0x8E, 0xDC, 0x12},
     {0x1A, 0x9F, 0xDC, 0x12},   {0x1A, 0xC6, 0xDC, 0x12},
     {0x1A, 0xD7, 0xDC, 0x12},   {0x1A, 0xE8, 0xDC, 0x12},
     {0x1A, 0x10C, 0xDC, 0x12},  {0x1A, 0x11E, 0xDC, 0x12},
     {0x111, 0x36, 0xBA, 0x12},  {0x111, 0x47, 0xBA, 0x12},
     {0x111, 0x58, 0xBA, 0x12},  {0x111, 0x7E, 0xBA, 0x12},
     {0x111, 0x8F, 0xBA, 0x12},  {0x111, 0xA0, 0xBA, 0x12},
     {0x111, 0xB1, 0xBA, 0x12},  {0x111, 0xCA, 0xBA, 0x12},
     {0x111, 0xDD, 0xBA, 0x12},  {0x111, 0xF0, 0xBA, 0x12},
     {0x111, 0x103, 0xBA, 0x12}, {0x111, 0x116, 0xBA, 0x12},
     {0x111, 0x129, 0xBA, 0x12}, {0x13E, 0x12, 0x89, 0x12},
}};

std::array<int16_t, 4> RightClickPortraitXmin = {{20, 131, 242, 357}};
std::array<int16_t, 4> RightClickPortraitXmax = {{83, 198, 312, 423}};

IndexedArray<int, MONSTER_TYPE_FIRST, MONSTER_TYPE_LAST> monster_popup_y_offsets = {
    {MONSTER_TYPE_ANGEL,                    -20},
    {MONSTER_TYPE_ARCHER,                   20},
    {MONSTER_TYPE_BAT,                      0},
    {MONSTER_TYPE_BEHEMOTH,                 -40},
    {MONSTER_TYPE_BEHOLDER,                 0},
    {MONSTER_TYPE_CLERIC_MOON,              0},
    {MONSTER_TYPE_CLERIC_SUN,               0},
    {MONSTER_TYPE_DEVIL,                    0},
    {MONSTER_TYPE_DRAGON,                   0},
    {MONSTER_TYPE_DRAGONFLY,                0},
    {MONSTER_TYPE_DWARF,                    -50},
    {MONSTER_TYPE_ELEMENTAL_AIR,            20},
    {MONSTER_TYPE_ELEMENTAL_EARTH,          0},
    {MONSTER_TYPE_ELEMENTAL_FIRE,           -10},
    {MONSTER_TYPE_ELEMENTAL_LIGHT,          -10},
    {MONSTER_TYPE_ELEMENTAL_WATER,          -20},
    {MONSTER_TYPE_ELF_ARCHER,               10},
    {MONSTER_TYPE_ELF_SPEARMAN,             -10},
    {MONSTER_TYPE_FIGHTER_CHAIN,            0},
    {MONSTER_TYPE_FIGHTER_LEATHER,          0},
    {MONSTER_TYPE_FIGHTER_PLATE,            0},
    {MONSTER_TYPE_GARGOYLE,                 -20},
    {MONSTER_TYPE_GENIE,                    10},
    {MONSTER_TYPE_GHOST,                    -10},
    {MONSTER_TYPE_GOBLIN,                   0},
    {MONSTER_TYPE_GOG,                      0},
    {MONSTER_TYPE_GOLEM,                    0},
    {MONSTER_TYPE_GRIFFIN,                  -20},
    {MONSTER_TYPE_HARPY,                    -10},
    {MONSTER_TYPE_HYDRA,                    0},
    {MONSTER_TYPE_LICH,                     0},
    {MONSTER_TYPE_MAGE,                     0},
    {MONSTER_TYPE_MANTICORE,                -40},
    {MONSTER_TYPE_MEDUSA,                   -20},
    {MONSTER_TYPE_MINOTAUR,                 0},
    {MONSTER_TYPE_MONK,                     0},
    {MONSTER_TYPE_NECROMANCER,              0},
    {MONSTER_TYPE_OOZE,                     -50},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_A,   -30},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_B,   -30},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_C,   -30},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_A,     -30},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_B,     -30},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_C,     -30},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_A,     0},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_B,     0},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_C,     0},
    {MONSTER_TYPE_PEASANT_ELF_MALE_A,       0},
    {MONSTER_TYPE_PEASANT_ELF_MALE_B,       0},
    {MONSTER_TYPE_PEASANT_ELF_MALE_C,       0},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_A,  -20},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_B,  -20},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_C,  -20},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_A,    20},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_B,    20},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_C,    20},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_A,    10},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_B,    10},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_C,    10},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_A,  10},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_B,  10},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_C,  10},
    {MONSTER_TYPE_RAT,                      -90},
    {MONSTER_TYPE_ROBOT,                    -60},
    {MONSTER_TYPE_ROC,                      -40},
    {MONSTER_TYPE_SEA_MONSTER,              -20},
    {MONSTER_TYPE_SKELETON_WARRIOR,         -20},
    {MONSTER_TYPE_SPIDER,                   -80},
    {MONSTER_TYPE_SWORDSMAN,                -10},
    {MONSTER_TYPE_THIEF,                    0},
    {MONSTER_TYPE_TITAN,                    0},
    {MONSTER_TYPE_TROGLODYTE,               -40},
    {MONSTER_TYPE_VAMPIRE,                  0},
    {MONSTER_TYPE_WARLOCK,                  0},
    {MONSTER_TYPE_WIGHT,                    0},
    {MONSTER_TYPE_WYVERN,                   -20},
    {MONSTER_TYPE_ZOMBIE,                   10},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_A,  0},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_B,  0},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_C,  0},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_A,    0},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_B,    0},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_C,    0},
    {MONSTER_TYPE_TROLL,                    -60},
    {MONSTER_TYPE_TREANT,                   0},
    {MONSTER_TYPE_GHOUL,                    0},
    {MONSTER_TYPE_BLASTERGUY,               0},
    {MONSTER_TYPE_ULTRA_DRAGON,             0},

    // OE addition, original data table was smaller:
    {MONSTER_TYPE_UNUSED_CAT,               0},
    {MONSTER_TYPE_UNUSED_CHICKEN,           0},
    {MONSTER_TYPE_UNUSED_DOG,               0},
    {MONSTER_TYPE_UNUSED_RAT,               0},
};

void Inventory_ItemPopupAndAlchemy();
Color GetSpellColor(signed int a1);
uint64_t GetExperienceRequiredForLevel(int level);

/**
 * @offset 0x4179BC
 */
static void CharacterUI_DrawTooltip(std::string_view title, std::string_view content) {
    Pointi pt = mouse->GetCursorPos();

    GUIWindow popup_window;
    popup_window.uFrameWidth = 384;
    popup_window.uFrameHeight = 256;
    popup_window.uFrameX = 128;
    popup_window.uFrameY = pt.y + 30;
    popup_window.uFrameHeight = assets->pFontSmallnum->CalcTextHeight(content, popup_window.uFrameWidth, 24) + 2 * assets->pFontLucida->GetHeight() + 24;
    popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
    popup_window.uFrameW = popup_window.uFrameY + popup_window.uFrameHeight - 1;
    popup_window.DrawMessageBox(0);

    popup_window.uFrameX += 12;
    popup_window.uFrameWidth -= 28;
    popup_window.uFrameY += 12;
    popup_window.uFrameHeight -= 12;
    popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
    popup_window.uFrameW = popup_window.uFrameY + popup_window.uFrameHeight - 1;

    auto colored_title = fmt::format(
        "{::}{}\f00000\n", ui_character_tooltip_header_default_color.tag(), title);
    popup_window.DrawTitleText(assets->pFontCreate.get(), 0, 0, colorTable.White, colored_title, 3);
    popup_window.DrawText(assets->pFontSmallnum.get(), {1, assets->pFontLucida->GetHeight()}, colorTable.White, content);  // popup_window.uFrameY + popup_window.uFrameHeight
}

//----- (004151D9) --------------------------------------------------------
void DrawPopupWindow(unsigned int uX, unsigned int uY, unsigned int uWidth,
                     unsigned int uHeight) {
    unsigned int uNumTiles;  // [sp+2Ch] [bp-Ch]@6
    int coord_x;             // [sp+2Ch] [bp-Ch]@3
    int coord_y;             // [sp+34h] [bp-4h]@5

    if (!parchment) return;

    render->SetUIClipRect(uX, uY, uX + uWidth, uY + uHeight);

    Sizei renderdims = render->GetRenderDimensions();
    float renwidth = renderdims.w;
    float renheight = renderdims.h;

    unsigned int parchment_width = parchment->width();
    unsigned int parchment_height = parchment->height();

    uNumTiles = uWidth / parchment_width;
    if (uWidth % parchment_width) ++uNumTiles;
    coord_y = uY;
    for (unsigned j = 0; j <= uHeight / parchment_height; j++) {
        coord_x = uX - parchment_width;
        for (unsigned i = uNumTiles + 1; i; --i) {
            coord_x += parchment_width;
            render->DrawTextureNew(coord_x / renwidth, coord_y / renheight,
                                   parchment);
        }
        coord_y += parchment_height;
    }

    render->DrawTextureNew(uX / renwidth, uY / renheight, messagebox_corner_x);
    render->DrawTextureNew(
        uX / renwidth, (uY + uHeight - messagebox_corner_y->height()) / renheight,
        messagebox_corner_y);
    render->DrawTextureNew(
        (uX + uWidth - messagebox_corner_z->width()) / renwidth, uY / renheight,
        messagebox_corner_z);
    render->DrawTextureNew(
        (uX + uWidth - messagebox_corner_z->width()) / renwidth,
        (uY + uHeight - messagebox_corner_y->height()) / renheight,
        messagebox_corner_w);

    if (uWidth > messagebox_corner_x->width() + messagebox_corner_z->width()) {
        render->SetUIClipRect(uX + messagebox_corner_x->width(), uY,
                              uX + uWidth - messagebox_corner_z->width(),
                              uY + uHeight);

        // horizontal borders
        for (unsigned int x = uX + messagebox_corner_x->width();
             x < uX + uWidth - messagebox_corner_x->width();
             x += messagebox_border_top->width()) {
            render->DrawTextureNew(x / renwidth, uY / renheight,
                                        messagebox_border_top);
            render->DrawTextureNew(
                x / renwidth,
                (uY + uHeight - messagebox_border_bottom->height()) / renheight,
                messagebox_border_bottom);
        }
    }

    // vertical borders
    if (uHeight > messagebox_corner_x->height() + messagebox_corner_y->height()) {
        render->SetUIClipRect(uX, uY + messagebox_corner_x->height(),
                              uX + uWidth,
                              uY + uHeight - messagebox_corner_y->height());

        for (unsigned int y = uY + messagebox_corner_x->height();
             y < uY + uHeight - messagebox_corner_y->height();
             y += messagebox_border_right->height()) {
            render->DrawTextureNew(uX / renwidth, y / renheight,
                                        messagebox_border_left);
            render->DrawTextureNew(
                (uX + uWidth - messagebox_border_right->width() - 1) /
                renwidth,
                y / renheight, messagebox_border_right);
        }
    }
    render->ResetUIClipRect();
}

//----- (0041D895) --------------------------------------------------------
void GameUI_DrawItemInfo(ItemGen *inspect_item) {
    unsigned int frameXpos;     // eax@3
    int v34;             // esi@81
    GUIWindow iteminfo_window;  // [sp+208h] [bp-70h]@2
    int v85;

    if (inspect_item->uItemID == ITEM_NULL)
        return;

    auto inspect_item_image = assets->getImage_ColorKey(inspect_item->GetIconName());

    iteminfo_window.sHint.clear();
    iteminfo_window.uFrameWidth = 384;
    iteminfo_window.uFrameHeight = 180;
    iteminfo_window.uFrameY = 40;

    Pointi pt = mouse->GetCursorPos();
    if (pt.x <= 320)
        frameXpos = pt.x + 30;
    else
        frameXpos = pt.x - iteminfo_window.uFrameWidth - 30;

    iteminfo_window.uFrameX = frameXpos;
    int itemXspacing = 100 - inspect_item_image->width();
    int itemYspacing = 144 - inspect_item_image->height();
    if (itemXspacing > 0) itemXspacing = itemXspacing / 2;
    if (itemYspacing <= 0)
        itemYspacing = 0;
    else
        itemYspacing = itemYspacing / 2;

    // added so window is correct size with broken items
    iteminfo_window.uFrameHeight = inspect_item_image->height() + itemYspacing + 54;

    if (!pItemTable->pItems[inspect_item->uItemID].uItemID_Rep_St)
        inspect_item->SetIdentified();

    int GoldAmount = 0;
    if (inspect_item->isGold()) {
        GoldAmount = inspect_item->goldAmount;
    }

    if (pParty->hasActiveCharacter()) {
        // try to identify
        if (!inspect_item->IsIdentified()) {
            if (pParty->activeCharacter().CanIdentify(inspect_item) == 1)
                inspect_item->SetIdentified();
            CharacterSpeech speech = SPEECH_ID_ITEM_FAIL;
            if (!inspect_item->IsIdentified()) {
                engine->_statusBar->setEvent(LSTR_IDENTIFY_FAILED);
            } else {
                speech = SPEECH_ID_ITEM_STRONG;
                if (inspect_item->GetValue() < 100 * (pParty->activeCharacter().uLevel + 5)) {
                    speech = SPEECH_ID_ITEM_WEAK;
                }
            }
            if (!identifyOrRepairReactionPlayed) {
                pParty->activeCharacter().playReaction(speech);
                identifyOrRepairReactionPlayed = true;
            }
        }
        inspect_item->UpdateTempBonus(pParty->GetPlayingTime());
        if (inspect_item->IsBroken()) {
            if (pParty->activeCharacter().CanRepair(inspect_item) == 1)
                inspect_item->uAttributes = inspect_item->uAttributes & ~ITEM_BROKEN | ITEM_IDENTIFIED;
            CharacterSpeech speech = SPEECH_REPAIR_FAIL;
            if (!inspect_item->IsBroken())
                speech = SPEECH_REPAIR_SUCCESS;
            else
                engine->_statusBar->setEvent(LSTR_REPAIR_FAILED);
            if (!identifyOrRepairReactionPlayed) {
                pParty->activeCharacter().playReaction(speech);
                identifyOrRepairReactionPlayed = true;
            }
        }
    }

    if (inspect_item->IsBroken()) {
        iteminfo_window.DrawMessageBox(0);
        render->SetUIClipRect(
            iteminfo_window.uFrameX + 12, iteminfo_window.uFrameY + 12,
            iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 12,
            iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 12);
        iteminfo_window.uFrameWidth -= 24;
        iteminfo_window.uFrameHeight -= 12;
        iteminfo_window.uFrameZ =
            iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
        iteminfo_window.uFrameW =
            iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;

        render->DrawTransparentRedShade(
            (iteminfo_window.uFrameX + (float)itemXspacing) / 640.0f,
            (itemYspacing + (float)iteminfo_window.uFrameY + 30) / 480.0f, inspect_item_image);

        iteminfo_window.DrawTitleText(assets->pFontArrus.get(), 0, 0xCu, colorTable.PaleCanary, inspect_item->GetDisplayName(), 3);
        iteminfo_window.DrawTitleText(assets->pFontArrus.get(), 0x64u,
            ((signed int)iteminfo_window.uFrameHeight >> 1) - assets->pFontArrus->CalcTextHeight(localization->GetString(LSTR_BROKEN_ITEM), iteminfo_window.uFrameWidth, 0) / 2,
                                      colorTable.TorchRed, localization->GetString(LSTR_BROKEN_ITEM), 3);
        render->ResetUIClipRect();

        if (inspect_item_image) {
            inspect_item_image->Release();
            inspect_item_image = nullptr;
        }

        return;
    }

    if (!inspect_item->IsIdentified()) {
        iteminfo_window.DrawMessageBox(0);
        render->SetUIClipRect(
            iteminfo_window.uFrameX + 12, iteminfo_window.uFrameY + 12,
            iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 12,
            iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 12);
        iteminfo_window.uFrameWidth -= 24;
        iteminfo_window.uFrameHeight -= 12;
        iteminfo_window.uFrameZ =
            iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
        iteminfo_window.uFrameW =
            iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
        render->DrawTextureNew(
            (iteminfo_window.uFrameX + (float)itemXspacing) / 640.0f,
            (itemYspacing + (float)iteminfo_window.uFrameY + 30) / 480.0f, inspect_item_image);
        iteminfo_window.DrawTitleText(
            assets->pFontArrus.get(), 0, 0xCu, colorTable.PaleCanary,
            pItemTable->pItems[inspect_item->uItemID].pUnidentifiedName, 3);
        iteminfo_window.DrawTitleText(
            assets->pFontArrus.get(), 0x64u,
            ((int)iteminfo_window.uFrameHeight >> 1) -
                assets->pFontArrus->CalcTextHeight(localization->GetString(LSTR_NOT_IDENTIFIED),
                                           iteminfo_window.uFrameWidth, 0) / 2, colorTable.TorchRed, localization->GetString(LSTR_NOT_IDENTIFIED), 3);
        render->ResetUIClipRect();

        if (inspect_item_image) {
            inspect_item_image->Release();
            inspect_item_image = nullptr;
        }
        return;
    }

    std::array<std::string, 3> text;

    text[0] = localization->FormatString(
        LSTR_FMT_TYPE_S,
        pItemTable->pItems[inspect_item->uItemID].pUnidentifiedName);

    switch (inspect_item->GetItemEquipType()) {
        case ITEM_TYPE_SINGLE_HANDED:
        case ITEM_TYPE_TWO_HANDED: {
            text[1] = fmt::format("{}: +{}   {}: {}d{}",
                                  localization->GetString(LSTR_ATTACK),
                                  inspect_item->GetDamageMod(),
                                  localization->GetString(LSTR_DAMAGE),
                                  inspect_item->GetDamageDice(),
                                  inspect_item->GetDamageRoll());
            if (inspect_item->GetDamageMod())
                text[1] += fmt::format("+{}", inspect_item->GetDamageMod());
            break;
        }

        case ITEM_TYPE_BOW:
            text[1] = fmt::format("{}: +{}   {}: {}d{}",
                                  localization->GetString(LSTR_SHOOT),
                                  inspect_item->GetDamageMod(),
                                  localization->GetString(LSTR_DAMAGE),
                                  inspect_item->GetDamageDice(),
                                  inspect_item->GetDamageRoll());
            if (inspect_item->GetDamageMod())
                text[1] += fmt::format("+{}", inspect_item->GetDamageMod());
            break;

        case ITEM_TYPE_ARMOUR:
        case ITEM_TYPE_SHIELD:
        case ITEM_TYPE_HELMET:
        case ITEM_TYPE_BELT:
        case ITEM_TYPE_CLOAK:
        case ITEM_TYPE_GAUNTLETS:
        case ITEM_TYPE_BOOTS:
        case ITEM_TYPE_RING:
        case ITEM_TYPE_AMULET:
            if (inspect_item->GetDamageDice())
                text[1] = fmt::format("{}: +{}",
                                      localization->GetString(LSTR_ARMOR),
                                      inspect_item->GetDamageDice() + inspect_item->GetDamageMod());
            break;

        default:
            break;
    }

    if (!GoldAmount) {
        // this is CORRECT! do not move to switch!
        if (inspect_item->isPotion()) {
            if (inspect_item->potionPower)
                text[2] = fmt::format("{}: {}", localization->GetString(LSTR_POWER), inspect_item->potionPower);
        } else if (inspect_item->isReagent()) {
            text[2] = fmt::format("{}: {}", localization->GetString(LSTR_POWER), inspect_item->GetDamageDice());
        } else if (inspect_item->attributeEnchantment) {
            text[2] = fmt::format("{}: {} +{}",
                                  localization->GetString(LSTR_SPECIAL_2),
                                  pItemTable->standardEnchantments[*inspect_item->attributeEnchantment].pBonusStat,
                                  inspect_item->m_enchantmentStrength);
        } else if (inspect_item->special_enchantment != ITEM_ENCHANTMENT_NULL) {
            text[2] = fmt::format("{}: {}",
                                  localization->GetString(LSTR_SPECIAL_2),
                                  pItemTable->pSpecialEnchantments[inspect_item->special_enchantment].pBonusStatement);
        } else if (inspect_item->isWand()) {
            text[2] = fmt::sprintf(localization->GetString(LSTR_FMT_S_U_OUT_OF_U),
                                   localization->GetString(LSTR_CHARGES),
                                   inspect_item->uNumCharges,
                                   inspect_item->uMaxCharges);
        }
    }
    iteminfo_window.uFrameWidth -= 12;
    iteminfo_window.uFrameZ = iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
    iteminfo_window.uFrameW = iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
    int Str_int = (3 * (assets->pFontArrus->GetHeight() + 8));
    for (const std::string &s : text)
        if (!s.empty())
            Str_int += assets->pFontComic->CalcTextHeight(s, iteminfo_window.uFrameWidth, 100) + 3;
    if (!pItemTable->pItems[inspect_item->uItemID].pDescription.empty())
        Str_int += assets->pFontSmallnum->CalcTextHeight(
            pItemTable->pItems[inspect_item->uItemID].pDescription,
            iteminfo_window.uFrameWidth, 100);
    iteminfo_window.uFrameHeight = inspect_item_image->height() + itemYspacing + 54;
    if ((signed int)Str_int > (signed int)iteminfo_window.uFrameHeight)
        iteminfo_window.uFrameHeight = (unsigned int)Str_int;
    if (inspect_item->uAttributes & ITEM_TEMP_BONUS &&
        (inspect_item->special_enchantment != ITEM_ENCHANTMENT_NULL || inspect_item->attributeEnchantment))
        iteminfo_window.uFrameHeight += assets->pFontComic->GetHeight();
    v85 = 0;
    if (assets->pFontArrus->GetHeight()) {
        iteminfo_window.uFrameWidth -= 24;
        if (assets->pFontArrus->CalcTextHeight(inspect_item->GetIdentifiedName(),
                                       iteminfo_window.uFrameWidth, 0) /
            (signed int)assets->pFontArrus->GetHeight())
            v85 = assets->pFontArrus->GetHeight();
        iteminfo_window.uFrameWidth += 24;
    }
    iteminfo_window.uFrameWidth += 12;
    iteminfo_window.uFrameHeight += (unsigned int)v85;
    iteminfo_window.uFrameW =
        iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
    iteminfo_window.uFrameZ =
        iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;

    // flush draw before starting popup window
    render->DrawTwodVerts();

    iteminfo_window.DrawMessageBox(0);
    render->SetUIClipRect(
        iteminfo_window.uFrameX + 12, iteminfo_window.uFrameY + 12,
        iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 12,
        iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 12);
    iteminfo_window.uFrameWidth -= 12;
    iteminfo_window.uFrameHeight -= 12;
    iteminfo_window.uFrameZ =
        iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
    iteminfo_window.uFrameW =
        iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
    render->DrawTextureNew((iteminfo_window.uFrameX + (float)itemXspacing) / 640.0f,
                           (iteminfo_window.uFrameY + (float)(iteminfo_window.uFrameHeight - inspect_item_image->height()) / 2.) / 480.0f,
                                inspect_item_image);

    v34 = (int)(v85 + 35);

    for (const std::string &s : text) {
        if (!s.empty()) {
            iteminfo_window.DrawText(assets->pFontComic.get(), {100, v34}, colorTable.White, s);
            v34 += assets->pFontComic->CalcTextHeight(s, iteminfo_window.uFrameWidth, 100, 0) + 3;
        }
    }
    if (!pItemTable->pItems[inspect_item->uItemID].pDescription.empty())
        iteminfo_window.DrawText(assets->pFontSmallnum.get(), {100, v34}, colorTable.White, pItemTable->pItems[inspect_item->uItemID].pDescription);
    iteminfo_window.uFrameX += 12;
    iteminfo_window.uFrameWidth -= 24;
    iteminfo_window.DrawTitleText(assets->pFontArrus.get(), 0, 0xCu, colorTable.PaleCanary,
                                  inspect_item->GetIdentifiedName(), 3);
    iteminfo_window.uFrameWidth += 24;
    iteminfo_window.uFrameX -= 12;

    if (GoldAmount) {
        auto txt = fmt::format("{}: {}", localization->GetString(LSTR_VALUE), GoldAmount);
        iteminfo_window.DrawText(assets->pFontComic.get(), {100, iteminfo_window.uFrameHeight - assets->pFontComic->GetHeight()}, colorTable.White, txt);
        render->ResetUIClipRect();
    } else {
        if ((inspect_item->uAttributes & ITEM_TEMP_BONUS) &&
            (inspect_item->special_enchantment != ITEM_ENCHANTMENT_NULL || inspect_item->attributeEnchantment)) {
            LongCivilDuration d = (inspect_item->uExpireTime - pParty->GetPlayingTime()).toLongCivilDuration();

            std::string txt4 = "Duration:";
            bool formatting = false;

            // TODO(captainurist): check how other durations are formatted, this is not the only place that creates
            //                     a CivilDuration. Unify the code?

            formatting |= d.years != 0;
            if (formatting)
                txt4 += fmt::format(" {}:yr", d.years);

            formatting |= d.months != 0;
            if (formatting)
                txt4 += fmt::format(" {}:mo", d.months);

            formatting |= d.days != 0;
            if (formatting)
                txt4 += fmt::format(" {}:dy", d.days);

            formatting |= d.hours != 0;
            if (formatting)
                txt4 += fmt::format(" {}:hr", d.hours);

            formatting |= d.minutes != 0;
            if (formatting)
                txt4 += fmt::format(" {}:mn", d.minutes);

            iteminfo_window.DrawText(assets->pFontComic.get(), {100, iteminfo_window.uFrameHeight - 2 * assets->pFontComic->GetHeight()}, colorTable.White, txt4);
        }

        auto txt2 = fmt::format(
            "{}: {}", localization->GetString(LSTR_VALUE),
            inspect_item->GetValue()
        );
        iteminfo_window.DrawText(assets->pFontComic.get(), {100, iteminfo_window.uFrameHeight - assets->pFontComic->GetHeight()}, colorTable.White, txt2);

        std::string txt3;
        if (inspect_item->uAttributes & ITEM_STOLEN) {
            txt3 = localization->GetString(LSTR_STOLEN);
        } else {
            if (!(inspect_item->uAttributes & ITEM_HARDENED)) {
                render->ResetUIClipRect();
                return;
            }
            txt3 = localization->GetString(LSTR_HARDENED);
        }

        iteminfo_window.DrawText(assets->pFontComic.get(),
            {assets->pFontComic->GetLineWidth(txt2) + 132, iteminfo_window.uFrameHeight - assets->pFontComic->GetHeight()}, colorTable.Red, txt3);
        render->ResetUIClipRect();
    }
}

void MonsterPopup_Draw(unsigned int uActorID, GUIWindow *pWindow) {
    static Actor pMonsterInfoUI_Doll;

    int Popup_Y_Offset = monster_popup_y_offsets[monsterTypeForMonsterId(pActors[uActorID].monsterInfo.id)] - 40;

    Duration actionLen;
    if (pActors[uActorID].monsterInfo.id == pMonsterInfoUI_Doll.monsterInfo.id) {
        actionLen = pMonsterInfoUI_Doll.currentActionLength;
    } else {
        // copy actor info if different
        pMonsterInfoUI_Doll = pActors[uActorID];
        pMonsterInfoUI_Doll.currentActionAnimation = ANIM_Bored;
        pMonsterInfoUI_Doll.currentActionTime = 0_ticks;
        actionLen = Duration::randomRealtimeSeconds(vrng, 1, 3);
        pMonsterInfoUI_Doll.currentActionLength = actionLen;
    }

    if (pMonsterInfoUI_Doll.currentActionTime > actionLen) {
        pMonsterInfoUI_Doll.currentActionTime = 0_ticks;
        if (pMonsterInfoUI_Doll.currentActionAnimation == ANIM_Bored ||
            pMonsterInfoUI_Doll.currentActionAnimation == ANIM_AtkMelee) {
            pMonsterInfoUI_Doll.currentActionAnimation = ANIM_Standing;
            pMonsterInfoUI_Doll.currentActionLength = Duration::randomRealtimeSeconds(vrng, 1, 2);
        } else {
            // rand();
            pMonsterInfoUI_Doll.currentActionAnimation = ANIM_Bored;
            if (!isPeasant(pMonsterInfoUI_Doll.monsterInfo.id) && vrng->random(30) < 100)
                pMonsterInfoUI_Doll.currentActionAnimation = ANIM_AtkMelee;
            pMonsterInfoUI_Doll.currentActionLength =
                pSpriteFrameTable
                    ->pSpriteSFrames[pActors[uActorID].spriteIds[pMonsterInfoUI_Doll.currentActionAnimation]]
                    .uAnimLength;
        }
    }

    Recti doll_rect(pWindow->uFrameX + 13, pWindow->uFrameY + 52, 128, 128);

    {
        SpriteFrame *Portrait_Sprite = pSpriteFrameTable->GetFrame(
            pActors[uActorID]
                .spriteIds[pMonsterInfoUI_Doll.currentActionAnimation],
            pMonsterInfoUI_Doll.currentActionTime);

        // Draw portrait border
        render->ResetUIClipRect();
        render->FillRectFast(doll_rect.x, doll_rect.y, 128, 128, colorTable.Black);
        render->BeginLines2D();
        int x0 = doll_rect.x;
        int x1 = doll_rect.x + doll_rect.w;
        int y0 = doll_rect.y;
        int y1 = doll_rect.y + doll_rect.h;
        render->RasterLine2D(x0 - 1, y0 - 1, x1 + 1, y0 - 1, colorTable.Jonquil);  // горизонтальная верхняя линия
        render->RasterLine2D(x0 - 1, y1 + 1, x0 - 1, y0 - 1, colorTable.Jonquil);  // горизонтальная нижняя линия
        render->RasterLine2D(x1 + 1, y1 + 1, x0 - 1, y1 + 1, colorTable.Jonquil);  // левая вертикальная линия
        render->RasterLine2D(x1 + 1, y0 - 1, x1 + 1, y1 + 1, colorTable.Jonquil);  // правая вертикальная линия
        render->EndLines2D();

        // Draw portrait
        render->DrawMonsterPortrait(doll_rect, Portrait_Sprite, Popup_Y_Offset);
    }
    pMonsterInfoUI_Doll.currentActionTime += pMiscTimer->dt();

    // Draw name and profession
    std::string str;
    if (pActors[uActorID].npcId) {
        str = NameAndTitle(getNPCData(pActors[uActorID].npcId));
    } else {
        str = GetDisplayName(&pActors[uActorID]);
    }
    pWindow->DrawTitleText(assets->pFontComic.get(), 0, 0xCu, colorTable.PaleCanary, str, 3);

    // health bar
    Actor::DrawHealthBar(&pActors[uActorID], pWindow);

    bool normal_level = false;
    bool expert_level = false;
    bool master_level = false;
    bool grandmaster_level = false;
    bool for_effects = false;

    if (pParty->hasActiveCharacter()) {
        int skill_points = 0;
        CharacterSkillMastery skill_mastery = CHARACTER_SKILL_MASTERY_NONE;
        CombinedSkillValue idMonsterSkill = pParty->activeCharacter().getActualSkillValue(CHARACTER_SKILL_MONSTER_ID);

        if ((skill_points = idMonsterSkill.level()) > 0) {
            skill_mastery = idMonsterSkill.mastery();
            if (skill_mastery == CHARACTER_SKILL_MASTERY_NOVICE) {
                if (skill_points + 10 >= pActors[uActorID].monsterInfo.level) {
                    normal_level = true;
                }
            } else if (skill_mastery == CHARACTER_SKILL_MASTERY_EXPERT) {
                if (2 * skill_points + 10 >= pActors[uActorID].monsterInfo.level) {
                    normal_level = true;
                    expert_level = true;
                }
            } else if (skill_mastery == CHARACTER_SKILL_MASTERY_MASTER) {
                if (3 * skill_points + 10 >= pActors[uActorID].monsterInfo.level) {
                    normal_level = true;
                    expert_level = true;
                    master_level = true;
                    for_effects = true;
                }
            } else if (skill_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                normal_level = true;
                expert_level = true;
                master_level = true;
                grandmaster_level = true;
                for_effects = true;
            }
        }

        // Only play reaction when right click on actor initially
        if (pActors[uActorID].aiState != Dead && pActors[uActorID].aiState != Dying &&
            !holdingMouseRightButton && skill_mastery != CHARACTER_SKILL_MASTERY_NONE) {
            CharacterSpeech speech;
            if (normal_level || expert_level || master_level || grandmaster_level) {
                if (pActors[uActorID].monsterInfo.level >= pParty->activeCharacter().uLevel - 5)
                    speech = SPEECH_ID_MONSTER_STRONG;
                else
                    speech = SPEECH_ID_MONSTER_WEAK;
            } else {
                speech = SPEECH_ID_MONSTER_FAIL;
            }
            pParty->activeCharacter().playReaction(speech);
        }
    }

    // Debug option for full info
    bool monster_full_informations = engine->config->debug.FullMonsterID.value();
    if (monster_full_informations == true) {
        normal_level = true;       //
        expert_level = true;       //
        master_level = true;       //
        grandmaster_level = true;  //
        for_effects = true;
    }

    int pTextHeight = 0;
    int pTextColorID = 0;
    pWindow->DrawText(assets->pFontSmallnum.get(), {12, 196}, colorTable.Jonquil, localization->GetString(LSTR_EFFECTS));
    if (!for_effects) {
        pWindow->DrawText(assets->pFontSmallnum.get(), {28, assets->pFontSmallnum->GetHeight() + 193}, colorTable.White, localization->GetString(LSTR_UNKNOWN_VALUE));
    } else {
        std::string pText;
        pTextHeight = assets->pFontSmallnum->GetHeight() + 193;
        for (ActorBuff buff : pActors[uActorID].buffs.indices()) {
            if (pActors[uActorID].buffs[buff].Active()) {
                switch (buff) {
                    case ACTOR_BUFF_CHARM:
                        pTextColorID = 60;
                        pText = localization->GetString(LSTR_CHARMED);
                        break;
                    case ACTOR_BUFF_SUMMONED:
                        pTextColorID = 82;
                        pText = localization->GetString(LSTR_SUMMONED);
                        break;
                    case ACTOR_BUFF_SHRINK:
                        pTextColorID = 92;
                        pText = localization->GetString(LSTR_SHRUNK);
                        break;
                    case ACTOR_BUFF_AFRAID:
                        pTextColorID = 63;
                        pText = localization->GetString(LSTR_AFRAID);
                        break;
                    case ACTOR_BUFF_STONED:
                        pText = localization->GetString(LSTR_STONED);
                        pTextColorID = 81;
                        break;
                    case ACTOR_BUFF_PARALYZED:
                        pText = localization->GetString(LSTR_PARALYZED);
                        pTextColorID = 81;
                        break;
                    case ACTOR_BUFF_SLOWED:
                        pText = localization->GetString(LSTR_SLOWED);
                        pTextColorID = 35;
                        break;
                    case ACTOR_BUFF_BERSERK:
                        pText = localization->GetString(LSTR_BERSERK);
                        pTextColorID = 62;
                        break;
                    case ACTOR_BUFF_SOMETHING_THAT_HALVES_AC:
                    case ACTOR_BUFF_MASS_DISTORTION:
                        pTextColorID = 0;
                        continue;
                    case ACTOR_BUFF_FATE:
                        pTextColorID = 47;
                        pText = localization->GetString(LSTR_FATE);
                        break;
                    case ACTOR_BUFF_ENSLAVED:
                        pTextColorID = 66;
                        pText = localization->GetString(LSTR_ENSLAVED);
                        break;
                    case ACTOR_BUFF_DAY_OF_PROTECTION:
                        pTextColorID = 85;
                        pText =
                            localization->GetString(LSTR_DAY_OF_PROTECTION);
                        break;
                    case ACTOR_BUFF_HOUR_OF_POWER:
                        pTextColorID = 86;
                        pText = localization->GetString(LSTR_HOUR_OF_POWER);
                        break;
                    case ACTOR_BUFF_SHIELD:
                        pTextColorID = 17;
                        pText = localization->GetString(LSTR_SHIELD);
                        break;
                    case ACTOR_BUFF_STONESKIN:
                        pTextColorID = 38;
                        pText = localization->GetString(LSTR_STONESKIN);
                        break;
                    case ACTOR_BUFF_BLESS:
                        pTextColorID = 46;
                        pText = localization->GetString(LSTR_BLESS);
                        break;
                    case ACTOR_BUFF_HEROISM:
                        pTextColorID = 51;
                        pText = localization->GetString(LSTR_HEROISM);
                        break;
                    case ACTOR_BUFF_HASTE:
                        pTextColorID = 5;
                        pText = localization->GetString(LSTR_HASTE);
                        break;
                    case ACTOR_BUFF_PAIN_REFLECTION:
                        pTextColorID = 95;
                        pText =
                            localization->GetString(LSTR_PAIN_REFLECTION);
                        break;
                    case ACTOR_BUFF_HAMMERHANDS:
                        pTextColorID = 73;
                        pText = localization->GetString(LSTR_HAMMERHANDS);
                        break;
                    default:
                        break;
                }
                if (!pText.empty()) {
                    pWindow->DrawText(assets->pFontSmallnum.get(), {28, pTextHeight}, GetSpellColor(pTextColorID), pText);
                    pTextHeight = pTextHeight + assets->pFontSmallnum->GetHeight() - 3;
                }
            }
        }
        if (pText.empty()) {
            pWindow->DrawText(assets->pFontSmallnum.get(), {28, pTextHeight}, colorTable.White, localization->GetString(LSTR_NONE));
        }
    }

    pTextHeight = doll_rect.y;

    std::string hpStr, acStr;
    if (normal_level) {
        hpStr = fmt::format("{}", pActors[uActorID].monsterInfo.hp);
        acStr = fmt::format("{}", pActors[uActorID].monsterInfo.ac);
    } else {
        hpStr = acStr = localization->GetString(LSTR_UNKNOWN_VALUE);
    }
    pWindow->DrawText(assets->pFontSmallnum.get(), {150, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_HIT_POINTS));
    pWindow->DrawText(assets->pFontSmallnum.get(), {252, pTextHeight}, colorTable.White, hpStr);
    pTextHeight = doll_rect.y + assets->pFontSmallnum->GetHeight() - 3;
    pWindow->DrawText(assets->pFontSmallnum.get(), {150, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_ARMOR_CLASS));
    pWindow->DrawText(assets->pFontSmallnum.get(), {252, pTextHeight}, colorTable.White, acStr);
    pTextHeight = pTextHeight + assets->pFontSmallnum->GetHeight() - 6 + assets->pFontSmallnum->GetHeight();

    std::string attackStr, damageStr;
    if (expert_level) {
        attackStr = displayNameForDamageType(pActors[uActorID].monsterInfo.attack1Type, localization);
        if (pActors[uActorID].monsterInfo.attack1DamageBonus) {
            damageStr = fmt::format("{}d{}+{}", pActors[uActorID].monsterInfo.attack1DamageDiceRolls, pActors[uActorID].monsterInfo.attack1DamageDiceSides,
                                    pActors[uActorID].monsterInfo.attack1DamageBonus);
        } else {
            damageStr = fmt::format("{}d{}", pActors[uActorID].monsterInfo.attack1DamageDiceRolls, pActors[uActorID].monsterInfo.attack1DamageDiceSides);
        }
    } else {
        attackStr = damageStr = localization->GetString(LSTR_UNKNOWN_VALUE);
    }
    pWindow->DrawText(assets->pFontSmallnum.get(), {150, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_ATTACK));
    pWindow->DrawText(assets->pFontSmallnum.get(), {231, pTextHeight}, colorTable.White, attackStr);
    pTextHeight += assets->pFontSmallnum->GetHeight() - 3;
    pWindow->DrawText(assets->pFontSmallnum.get(), {150, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_DAMAGE));
    pWindow->DrawText(assets->pFontSmallnum.get(), {231, pTextHeight}, colorTable.White, damageStr);
    pTextHeight += assets->pFontSmallnum->GetHeight() - 6 + assets->pFontSmallnum->GetHeight();

    std::string spellTitleStr = localization->GetString(LSTR_SPELL);
    std::string spell1Str, spell2Str;
    if (master_level) {
        if (pActors[uActorID].monsterInfo.spell1Id == SPELL_NONE && pActors[uActorID].monsterInfo.spell2Id == SPELL_NONE) {
            spell1Str = localization->GetString(LSTR_NONE);
        }
        if (pActors[uActorID].monsterInfo.spell1Id != SPELL_NONE && pActors[uActorID].monsterInfo.spell2Id != SPELL_NONE) {
            spellTitleStr = localization->GetString(LSTR_SPELLS);
        }
        if (pActors[uActorID].monsterInfo.spell1Id != SPELL_NONE) {
            spell1Str = pSpellStats->pInfos[pActors[uActorID].monsterInfo.spell1Id].pShortName;
        }
        if (pActors[uActorID].monsterInfo.spell2Id != SPELL_NONE) {
            spell2Str = pSpellStats->pInfos[pActors[uActorID].monsterInfo.spell2Id].pShortName;
        }
    } else {
        spell1Str = localization->GetString(LSTR_UNKNOWN_VALUE);
    }
    pWindow->DrawText(assets->pFontSmallnum.get(), {150, pTextHeight}, colorTable.Jonquil, spellTitleStr);
    pWindow->DrawText(assets->pFontSmallnum.get(), {220, pTextHeight}, colorTable.White, spell1Str);
    pTextHeight += assets->pFontSmallnum->GetHeight() - 3;
    if (!spell2Str.empty()) {
        pWindow->DrawText(assets->pFontSmallnum.get(), {220, pTextHeight}, colorTable.White, spell2Str);
        pTextHeight += assets->pFontSmallnum->GetHeight() - 3;
    }
    pTextHeight += assets->pFontSmallnum->GetHeight() - 3;

    std::array<std::string, 10> resTypes = {
        localization->GetSpellSchoolName(MAGIC_SCHOOL_FIRE),
        localization->GetSpellSchoolName(MAGIC_SCHOOL_AIR),
        localization->GetSpellSchoolName(MAGIC_SCHOOL_WATER),
        localization->GetSpellSchoolName(MAGIC_SCHOOL_EARTH),
        localization->GetSpellSchoolName(MAGIC_SCHOOL_SPIRIT),
        localization->GetSpellSchoolName(MAGIC_SCHOOL_MIND),
        localization->GetSpellSchoolName(MAGIC_SCHOOL_BODY),
        localization->GetSpellSchoolName(MAGIC_SCHOOL_LIGHT),
        localization->GetSpellSchoolName(MAGIC_SCHOOL_DARK),
        localization->GetString(LSTR_PHYSICAL)
    };

    std::array<int, 10> resValues = {
        pActors[uActorID].monsterInfo.resFire,
        pActors[uActorID].monsterInfo.resAir,
        pActors[uActorID].monsterInfo.resWater,
        pActors[uActorID].monsterInfo.resEarth,
        pActors[uActorID].monsterInfo.resSpirit,
        pActors[uActorID].monsterInfo.resMind,
        pActors[uActorID].monsterInfo.resBody,
        pActors[uActorID].monsterInfo.resLight,
        pActors[uActorID].monsterInfo.resDark,
        pActors[uActorID].monsterInfo.resPhysical, // Physical & Dark were switched, was a bug?
    };

    pWindow->DrawText(assets->pFontSmallnum.get(), {150, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_RESISTANCES));
    pTextHeight = pTextHeight + assets->pFontSmallnum->GetHeight() - 3;

    if (grandmaster_level) {
        std::string resStr;
        for (int i = 0; i < 10; i++) {
            if (resValues[i] == 200) {
                resStr = localization->GetString(LSTR_IMMUNE);
            } else {
                if (resValues[i]) {
                    resStr = localization->GetString(LSTR_RESISTANT);
                } else {
                    resStr = localization->GetString(LSTR_NONE);
                }
            }

            pWindow->DrawText(assets->pFontSmallnum.get(), {170, pTextHeight}, colorTable.Jonquil, resTypes[i]);
            pWindow->DrawText(assets->pFontSmallnum.get(), {241, pTextHeight}, colorTable.White, resStr);
            pTextHeight += assets->pFontSmallnum->GetHeight() - 3;
        }
    } else {
        for (int i = 0; i < 10; ++i) {
            pWindow->DrawText(assets->pFontSmallnum.get(), {170, pTextHeight}, colorTable.Jonquil, resTypes[i]);
            pWindow->DrawText(assets->pFontSmallnum.get(), {241, pTextHeight}, colorTable.White, localization->GetString(LSTR_UNKNOWN_VALUE));
            pTextHeight += assets->pFontSmallnum->GetHeight() - 3;
        }
    }

    // cast spell: Detect life
    if (pParty->pPartyBuffs[PARTY_BUFF_DETECT_LIFE].Active()) {
        std::string str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_HIT_POINTS), pActors[uActorID].currentHP);
        assets->pFontSmallnum->GetLineWidth(str);
        pWindow->DrawTitleText(assets->pFontSmallnum.get(), 0, pWindow->uFrameHeight - assets->pFontSmallnum->GetHeight() - 12, colorTable.White, str, 3);
    }

    // Debug - show actor AI state with full information
    if (monster_full_informations) {
        std::string str = fmt::format("AI State: {}", std::to_underlying(pActors[uActorID].aiState));
        assets->pFontSmallnum->GetLineWidth(str);
        pWindow->DrawTitleText(assets->pFontSmallnum.get(), 0, pWindow->uFrameHeight - assets->pFontSmallnum->GetHeight() - 12, colorTable.White, str, 3);
    }
}

/**
  * @offset 0x00417BB5
  *
  * @brief Generating message for skill description popup.
  *
  * @param uPlayerID                     Character identifier.
  * @param uPlayerSkillType              Skill type identifier.
  */
std::string CharacterUI_GetSkillDescText(int uPlayerID, CharacterSkillType uPlayerSkillType) {
    size_t line_width = std::max({
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_NORMAL)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_EXPERT)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_MASTER)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_BONUS_2))
    });

    int base_skill = pParty->pCharacters[uPlayerID].getSkillValue(uPlayerSkillType).level();
    int actual_skill = pParty->pCharacters[uPlayerID].getActualSkillValue(uPlayerSkillType).level();

    std::string Description = localization->GetSkillDescription(uPlayerSkillType);
    if (!localization->GetSkillDescriptionNormal(uPlayerSkillType).empty()) {
        Description = fmt::format("{}\n\n", Description);

        for (CharacterSkillMastery mastery : allSkillMasteries()) {
            Description += fmt::format(
                "{::}{}\t{:03}:\t{:03}{}\t000\n",
                GetSkillColor(pParty->pCharacters[uPlayerID].classType, uPlayerSkillType, mastery).tag(),
                localization->MasteryName(mastery),
                line_width + 3,
                line_width + 10,
                localization->GetSkillDescription(uPlayerSkillType, mastery)
            );
        }
    }

    if (base_skill != actual_skill)
        Description += fmt::format("{::}\n{}\t{:03}:\t{:03}+{}\n", colorTable.White.tag(), localization->GetString(LSTR_BONUS_2), line_width + 3, line_width + 10, actual_skill - base_skill);

    return Description;
}

void CharacterUI_SkillsTab_ShowHint() {
    int pX = 0;
    int pY = 0;
    mouse->GetClickPos(&pX, &pY);

    if (pX < 24 || pX > 455 || pY < 18 || pY > 36) {
        for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
            if (pButton->msg == UIMSG_SkillUp && pX >= pButton->uX &&
                pX < pButton->uZ && pY >= pButton->uY && pY < pButton->uW) {
                CharacterSkillType skill = static_cast<CharacterSkillType>(pButton->msg_param);
                std::string pSkillDescText = CharacterUI_GetSkillDescText(pParty->activeCharacterIndex() - 1, skill);
                CharacterUI_DrawTooltip(localization->GetSkillName(skill), pSkillDescText);
            }
        }
    } else {
        CharacterUI_DrawTooltip(localization->GetString(LSTR_SKILL_POINTS), localization->getSkillPointsDescription());
    }
}

//----- (00418083) --------------------------------------------------------
void CharacterUI_StatsTab_ShowHint() {
    int pStringNum;         // edi@1
    Color pTextColor;  // eax@15
    std::string pHourWord;  // ecx@17
    std::string pDayWord;   // eax@20

    Pointi pt = mouse->GetCursorPos();
    for (pStringNum = 0; pStringNum < stat_string_coord.size(); ++pStringNum) {
        if (pt.x >= stat_string_coord[pStringNum].x &&
            pt.x <= stat_string_coord[pStringNum].x +
                        stat_string_coord[pStringNum].width) {
            if (pt.y >= stat_string_coord[pStringNum].y &&
                pt.y <= stat_string_coord[pStringNum].y +
                            stat_string_coord[pStringNum].height)
                break;
        }
    }

    switch (pStringNum) {
        case 0:  // Attributes
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            CharacterUI_DrawTooltip(
                localization->GetAttirubteName(static_cast<CharacterAttributeType>(pStringNum)),
                localization->GetAttributeDescription(static_cast<CharacterAttributeType>(pStringNum)));
            break;
        case 7:  // Health Points
            CharacterUI_DrawTooltip(localization->GetString(LSTR_HIT_POINTS), localization->getHPDescription());
            break;
        case 8:  // Spell Points
            CharacterUI_DrawTooltip(localization->GetString(LSTR_SPELL_POINTS), localization->getSPDescription());
            break;
        case 9:  // Armour class
            CharacterUI_DrawTooltip(localization->GetString(LSTR_ARMOR_CLASS), localization->getArmourClassDescription());
            break;
        case 10:  // Character Condition
        {
            std::string str = std::string(localization->getCharacterConditionDescription()) + "\n";

            for (Condition condition : conditionImportancyTable()) {
                if (pParty->activeCharacter().conditions.Has(condition)) {
                    str += " \n";
                    Duration condition_time = pParty->GetPlayingTime() - pParty->activeCharacter().conditions.Get(condition);
                    CivilDuration d = condition_time.toCivilDuration();
                    pTextColor = GetConditionDrawColor(condition);
                    str += fmt::format("{::}{}\f00000 - ", pTextColor.tag(), localization->GetCharacterConditionName(condition));
                    if (d.hours && d.hours <= 1)
                        pHourWord = localization->GetString(LSTR_HOUR);
                    else
                        pHourWord = localization->GetString(LSTR_HOURS);
                    if (!d.days || (pDayWord = localization->GetString(LSTR_DAY_CAPITALIZED), d.days > 1))
                        pDayWord = localization->GetString(LSTR_DAYS);
                    str += fmt::format("{} {}, {} {}", d.days, pDayWord, d.hours, pHourWord);
                }
            }

            if (!str.empty())
                CharacterUI_DrawTooltip(localization->GetString(LSTR_CONDITION), str);
            break;
        }

        case 11:  // Fast Spell
            CharacterUI_DrawTooltip(localization->GetString(LSTR_QUICK_SPELL), localization->getFastSpellDescription());
            break;

        case 12:  // Character Age
            CharacterUI_DrawTooltip(localization->GetString(LSTR_AGE), localization->getAgeDescription());
            break;

        case 13:  // Character Level
            CharacterUI_DrawTooltip(localization->GetString(LSTR_LEVEL), localization->getLevelDescription());
            break;

        case 14:  // Experience
        {
            int eligbleLevel = pParty->activeCharacter().uLevel;
            do {
                if (pParty->activeCharacter().experience < GetExperienceRequiredForLevel(eligbleLevel)) {
                    break;
                }
                ++eligbleLevel;
            } while (eligbleLevel <= 10000);

            std::string str;
            if (eligbleLevel > pParty->activeCharacter().uLevel) {
                str = localization->FormatString(LSTR_ELIGIBLE_TO_LEVELUP, eligbleLevel);
            }
            int nextLevelExp = GetExperienceRequiredForLevel(eligbleLevel) - pParty->activeCharacter().experience;
            str += "\n" + localization->FormatString(LSTR_XP_UNTIL_NEXT_LEVEL, nextLevelExp, eligbleLevel + 1);
            str = std::string(localization->getExpDescription()) + "\n \n" + str;

            CharacterUI_DrawTooltip(localization->GetString(LSTR_EXPERIENCE), str);
            break;
        }

        case 15:  // Attack Bonus
        {
            Duration meleerecov = pParty->activeCharacter().GetAttackRecoveryTime(false);
            // TODO(captainurist): fmt can throw
            // TODO(captainurist): write it out in game seconds?
            std::string description = fmt::sprintf(localization->GetString(LSTR_FMT_RECOVERY_TIME_D), meleerecov.ticks());
            description = fmt::format("{}\n\n{}", localization->getMeleeAttackDescription(), description);
            CharacterUI_DrawTooltip(localization->GetString(LSTR_ATTACK_BONUS), description);
            break;
        }
        case 16:  // Attack Damage
            CharacterUI_DrawTooltip(localization->GetString(LSTR_ATTACK_DAMAGE), localization->getMeleeDamageDescription());
            break;

        case 17:  // Missle Bonus
        {
            bool hasBow = pParty->activeCharacter().GetBowItem() != nullptr;
            auto weapon = pParty->activeCharacter().GetMainHandItem();
            bool hasBlaster = weapon && weapon->GetPlayerSkillType() == CHARACTER_SKILL_BLASTER;
            // TODO(captainurist): fmt can throw
            std::string description;
            if (hasBow || hasBlaster) {
                // Blaster takes precendence in the event both are equipped
                Duration missRecov = pParty->activeCharacter().GetAttackRecoveryTime(!hasBlaster);
                description = fmt::sprintf(localization->GetString(LSTR_FMT_RECOVERY_TIME_D), missRecov.ticks()); // NOLINT: this is not ::sprintf.
            } else {
                description = localization->GetString(LSTR_RECOVERY_TIME_NA);
            }
            description = fmt::format("{}\n\n{}", localization->getRangedAttackDescription(), description);
            CharacterUI_DrawTooltip(localization->GetString(LSTR_SHOOT_BONUS), description);
            break;
        }
        case 18:  // Missle Damage
            CharacterUI_DrawTooltip(localization->GetString(LSTR_SHOOT_DAMAGE), localization->getRangedDamageDescription());
            break;

        case 19:  // Fire Resistance
            CharacterUI_DrawTooltip(localization->GetString(LSTR_FIRE), localization->getFireResistanceDescription());
            break;

        case 20:  // Air Resistance
            CharacterUI_DrawTooltip(localization->GetString(LSTR_AIR), localization->getAirResistanceDescription());
            break;

        case 21:  // Water Resistance
            CharacterUI_DrawTooltip(localization->GetString(LSTR_WATER), localization->getWaterResistanceDescription());
            break;

        case 22:  // Earth Resistance
            CharacterUI_DrawTooltip(localization->GetString(LSTR_EARTH), localization->getEarthResistanceDescription());
            break;

        case 23:  // Mind Resistance
            CharacterUI_DrawTooltip(localization->GetString(LSTR_MIND), localization->getMindResistanceDescription());
            break;

        case 24:  // Body Resistance
            CharacterUI_DrawTooltip(localization->GetString(LSTR_BODY), localization->getBodyResistanceDescription());
            break;

        case 25:  // Skill Points
            CharacterUI_DrawTooltip(localization->GetString(LSTR_SKILL_POINTS), localization->getSkillPointsDescription());
            break;

        case 26:  // Class description
            CharacterUI_DrawTooltip(localization->GetClassName(pParty->activeCharacter().classType),
                                    localization->GetClassDescription(pParty->activeCharacter().classType));
            break;

        default:
            break;
    }
}

//----- (00410B28) --------------------------------------------------------
void DrawSpellDescriptionPopup(SpellId spell_id) {
    SpellInfo *spell;             // esi@1
    unsigned int v3;              // eax@2
    GUIWindow spell_info_window;  // [sp+Ch] [bp-68h]@4

    Pointi pt = mouse->GetCursorPos();

    spell = &pSpellStats->pInfos[spell_id];
    if (pt.y <= 250)
        v3 = pt.y + 30;
    else
        v3 = 30;
    spell_info_window.uFrameY = v3;
    spell_info_window.uFrameWidth = 328;
    spell_info_window.uFrameHeight = 68;
    spell_info_window.uFrameX = 90;
    spell_info_window.uFrameZ = 417;
    spell_info_window.uFrameW = v3 + 67;
    spell_info_window.sHint.clear();

    int v5 = std::max({
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_NORMAL)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_EXPERT)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_MASTER)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND))
    });

    std::string str = fmt::format(
        "{}\n\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}",
        spell->pDescription,
        localization->GetString(LSTR_NORMAL), v5 + 3, v5 + 10, spell->pBasicSkillDesc,
        localization->GetString(LSTR_EXPERT), v5 + 3, v5 + 10, spell->pExpertSkillDesc,
        localization->GetString(LSTR_MASTER), v5 + 3, v5 + 10, spell->pMasterSkillDesc,
        localization->GetString(LSTR_GRAND), v5 + 3, v5 + 10, spell->pGrandmasterSkillDesc
    );
    spell_info_window.uFrameHeight += assets->pFontSmallnum->CalcTextHeight(
        str, spell_info_window.uFrameWidth, 0
    );
    if ((signed int)spell_info_window.uFrameHeight < 150)
        spell_info_window.uFrameHeight = 150;
    spell_info_window.uFrameWidth = game_viewport_width;
    spell_info_window.DrawMessageBox(0);
    spell_info_window.uFrameWidth -= 12;
    spell_info_window.uFrameHeight -= 12;
    spell_info_window.uFrameZ = spell_info_window.uFrameX + spell_info_window.uFrameWidth - 1;
    spell_info_window.uFrameW = spell_info_window.uFrameHeight + spell_info_window.uFrameY - 1;
    spell_info_window.DrawTitleText(
        assets->pFontArrus.get(), 0x78u, 0xCu, colorTable.PaleCanary, spell->name, 3);
    spell_info_window.DrawText(assets->pFontSmallnum.get(), {120, 44}, colorTable.White, str);
    spell_info_window.uFrameWidth = 108;
    spell_info_window.uFrameZ = spell_info_window.uFrameX + 107;
    CharacterSkillType skill = skillForMagicSchool(pParty->activeCharacter().lastOpenedSpellbookPage);
    CharacterSkillMastery skill_mastery = pParty->activeCharacter().getSkillValue(skill).mastery();
    spell_info_window.DrawTitleText(assets->pFontComic.get(), 12, 75, colorTable.White, localization->GetSkillName(skill), 3);

    if (skill_mastery == CharacterSkillMastery::CHARACTER_SKILL_MASTERY_NONE) {
        if (engine->config->debug.AllMagic.value()) {
            skill_mastery = CharacterSkillMastery::CHARACTER_SKILL_MASTERY_GRANDMASTER;
        } else {
            assert(false && "Character doesnt have this magic skill!");
        }
    }

    auto str2 = fmt::format(
        "{}\n{}", localization->GetString(LSTR_SP_COST),
        pSpellDatas[spell_id].mana_per_skill[skill_mastery]);
    spell_info_window.DrawTitleText(
        assets->pFontComic.get(), 12,
        spell_info_window.uFrameHeight - assets->pFontComic->GetHeight() - 16, colorTable.White, str2,
        3);
    dword_507B00_spell_info_to_draw_in_popup = SPELL_NONE;
}

/**
 * @offset 0x41D73D
 */
static void drawBuffPopupWindow() {
    GUIWindow popupWindow;
    int stringCount;

    static constexpr IndexedArray<Color, PARTY_BUFF_FIRST, PARTY_BUFF_LAST> spellTooltipColors = {
        {PARTY_BUFF_RESIST_AIR,             colorTable.Anakiwa},
        {PARTY_BUFF_RESIST_BODY,            colorTable.FlushOrange},
        {PARTY_BUFF_DAY_OF_GODS,            colorTable.PaleCanary},
        {PARTY_BUFF_DETECT_LIFE,            colorTable.Mercury},
        {PARTY_BUFF_RESIST_EARTH,           colorTable.Gray},
        {PARTY_BUFF_FEATHER_FALL,           colorTable.Anakiwa},
        {PARTY_BUFF_RESIST_FIRE,            colorTable.DarkOrange},
        {PARTY_BUFF_FLY,                    colorTable.Anakiwa},
        {PARTY_BUFF_HASTE,                  colorTable.DarkOrange},
        {PARTY_BUFF_HEROISM,                colorTable.Mercury},
        {PARTY_BUFF_IMMOLATION,             colorTable.DarkOrange},
        {PARTY_BUFF_INVISIBILITY,           colorTable.Anakiwa},
        {PARTY_BUFF_RESIST_MIND,            colorTable.PurplePink},
        {PARTY_BUFF_PROTECTION_FROM_MAGIC,  colorTable.FlushOrange},
        {PARTY_BUFF_SHIELD,                 colorTable.Anakiwa},
        {PARTY_BUFF_STONE_SKIN,             colorTable.Gray},
        {PARTY_BUFF_TORCHLIGHT,             colorTable.DarkOrange},
        {PARTY_BUFF_RESIST_WATER,           colorTable.AzureRadiance},
        {PARTY_BUFF_WATER_WALK,             colorTable.AzureRadiance},
        {PARTY_BUFF_WIZARD_EYE,             colorTable.Anakiwa}
    };

    popupWindow.sHint.clear();
    popupWindow.uFrameWidth = 400;
    popupWindow.uFrameX = 38;
    popupWindow.uFrameY = 60;

    stringCount = 0;
    for (SpellBuff &spellBuff : pParty->pPartyBuffs) {
        stringCount += (spellBuff.Active()) ? 1 : 0;
    }

    popupWindow.uFrameHeight = assets->pFontArrus->GetHeight() + 72;
    popupWindow.uFrameHeight += (stringCount - 1) * assets->pFontArrus->GetHeight();
    popupWindow.uFrameZ = popupWindow.uFrameWidth + popupWindow.uFrameX - 1;
    popupWindow.uFrameW = popupWindow.uFrameY + popupWindow.uFrameHeight - 1;
    popupWindow.DrawMessageBox(0);
    popupWindow.DrawTitleText(assets->pFontArrus.get(), 0, 12, colorTable.White, localization->GetString(LSTR_ACTIVE_PARTY_SPELLS), 3);
    if (!stringCount) {
        popupWindow.DrawTitleText(assets->pFontComic.get(), 0, 40, colorTable.White, localization->GetString(LSTR_NONE), 3);
    }

    stringCount = 0;
    for (PartyBuff i : pParty->pPartyBuffs.indices()) {
        if (pParty->pPartyBuffs[i].Active()) {
            Duration remaingTime = pParty->pPartyBuffs[i].GetExpireTime() - pParty->GetPlayingTime();
            int yPos = stringCount * assets->pFontComic->GetHeight() + 40;
            popupWindow.DrawText(assets->pFontComic.get(), {52, yPos}, spellTooltipColors[i], localization->GetPartyBuffName(i));
            DrawBuff_remaining_time_string(yPos, &popupWindow, remaingTime, assets->pFontComic.get());
            stringCount++;
        }
    }
}

/**
 * @offset 0x4B1523
 */
void showSpellbookInfo(ItemId spellbook) {
    assert(isSpellbook(spellbook));
    SpellId spell = spellForSpellbook(spellbook);

    Pointi cursorPos = EngineIocContainer::ResolveMouse()->GetCursorPos();
    int popupVertPos = 30;
    if (cursorPos.y <= 320) {
        popupVertPos = cursorPos.y + 30;
    }

    GUIWindow popup;
    popup.uFrameY = popupVertPos;
    popup.uFrameWidth = 328;
    popup.uFrameHeight = 68;
    popup.uFrameX = 90;
    popup.uFrameZ = 417;
    popup.uFrameW = popupVertPos + 67;

    int maxLineWidth = std::max({
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_NORMAL)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_EXPERT)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_MASTER)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND))});

    std::string str = fmt::format("{}\n\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}",
                                  pSpellStats->pInfos[spell].pDescription,
                                  localization->GetString(LSTR_NORMAL), maxLineWidth + 3, maxLineWidth + 10, pSpellStats->pInfos[spell].pBasicSkillDesc,
                                  localization->GetString(LSTR_EXPERT), maxLineWidth + 3, maxLineWidth + 10, pSpellStats->pInfos[spell].pExpertSkillDesc,
                                  localization->GetString(LSTR_MASTER), maxLineWidth + 3, maxLineWidth + 10, pSpellStats->pInfos[spell].pMasterSkillDesc,
                                  localization->GetString(LSTR_GRAND), maxLineWidth + 3, maxLineWidth + 10, pSpellStats->pInfos[spell].pGrandmasterSkillDesc);

    popup.uFrameHeight += assets->pFontSmallnum->CalcTextHeight(str, popup.uFrameWidth, 0);
    if (popup.uFrameHeight < 150) {
        popup.uFrameHeight = 150;
    }
    popup.uFrameWidth = game_viewport_width;
    popup.DrawMessageBox(0);
    popup.uFrameWidth -= 12;
    popup.uFrameHeight -= 12;
    popup.uFrameZ = popup.uFrameX + popup.uFrameWidth - 1;
    popup.uFrameW = popup.uFrameHeight + popup.uFrameY - 1;
    popup.DrawTitleText(assets->pFontArrus.get(), 0x78u, 0xCu, colorTable.PaleCanary, pSpellStats->pInfos[spell].name, 3u);
    popup.DrawText(assets->pFontSmallnum.get(), {120, 44}, colorTable.White, str);
    popup.uFrameZ = popup.uFrameX + 107;
    popup.uFrameWidth = 108;
    popup.DrawTitleText(assets->pFontComic.get(), 0xCu, 0x4Bu, colorTable.White, localization->GetSkillName(skillForSpell(spell)), 3u);

    str = fmt::format("{}\n{}", localization->GetString(LSTR_SP_COST), pSpellDatas[spell].mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE]);
    popup.DrawTitleText(assets->pFontComic.get(), 0xCu, popup.uFrameHeight - assets->pFontComic->GetHeight() - 16, colorTable.White, str, 3);
}

//----- new function
void ShowPopupShopSkills() {
    int pX = 0;
    int pY = 0;
    mouse->GetClickPos(&pX, &pY);

    if (pDialogueWindow && pDialogueWindow->pNumPresenceButton != 0) {
        for (GUIButton *pButton : pDialogueWindow->vButtons) {
            if (pX >= pButton->uX && pX < pButton->uZ && pY >= pButton->uY && pY < pButton->uW) {
                if (IsSkillLearningDialogue((DialogueId)pButton->msg_param)) {
                    auto skill_id = GetLearningDialogueSkill((DialogueId)pButton->msg_param);
                    if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill_id] != CHARACTER_SKILL_MASTERY_NONE &&
                        !pParty->activeCharacter().pActiveSkills[skill_id]) {
                        // is this skill visible
                        std::string pSkillDescText = CharacterUI_GetSkillDescText(pParty->activeCharacterIndex() - 1, skill_id);
                        CharacterUI_DrawTooltip(localization->GetSkillName(skill_id), pSkillDescText);
                    }
                }
            }
        }
    }
}


//----- (004B1A2D) --------------------------------------------------------
void ShowPopupShopItem() {
    // TODO(pskelton): Extract common item picking code
    ItemGen *item;  // ecx@13
    int invindex;
    int testpos;
    BuildingType buildingType = window_SpeakInHouse->buildingType();
    DialogueId dialogue = window_SpeakInHouse->getCurrentDialogue();

    if (buildingType == BUILDING_INVALID)
        return;

    if (dialogue < DIALOGUE_SHOP_BUY_STANDARD)
        return;

    Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();
    int testx;

    if (buildingType <= BUILDING_ALCHEMY_SHOP) {
        if (dialogue == DIALOGUE_SHOP_BUY_STANDARD || dialogue == DIALOGUE_SHOP_BUY_SPECIAL) {
            switch (buildingType) {
                case BUILDING_WEAPON_SHOP: {
                    testx = (pt.x - 30) / 70;
                    if (testx >= 0 && testx < 6) {
                        if (dialogue == DIALOGUE_SHOP_BUY_STANDARD)
                            item = &pParty->standartItemsInShops[window_SpeakInHouse->houseId()][testx];
                        else
                            item = &pParty->specialItemsInShops[window_SpeakInHouse->houseId()][testx];

                        if (item->uItemID != ITEM_NULL) {
                            testpos = ((60 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 70);
                            if (pt.x >= testpos && pt.x < (testpos + (shop_ui_items_in_store[testx]->width()))) {
                                if (pt.y >= weaponYPos[testx] + 30 && pt.y < (weaponYPos[testx] + 30 + (shop_ui_items_in_store[testx]->height()))) {
                                    GameUI_DrawItemInfo(item);
                                }
                            } else {
                                return;
                            }
                        }
                    } else {
                        return;
                    }

                    break;
                }

                case BUILDING_ARMOR_SHOP:
                    testx = (pt.x - 40) / 105;
                    if (testx >= 0 && testx < 4) {
                        if (pt.y >= 126) {
                            testx += 4;
                        }

                        if (dialogue == DIALOGUE_SHOP_BUY_STANDARD)
                            item = &pParty->standartItemsInShops[window_SpeakInHouse->houseId()][testx];
                        else
                            item = &pParty->specialItemsInShops[window_SpeakInHouse->houseId()][testx];

                        if (item->uItemID != ITEM_NULL) {
                            if (testx >= 4) {
                                testpos = ((90 - (shop_ui_items_in_store[testx]->width() / 2)) + (testx * 105) - 420);  // low row
                            } else {
                                testpos = ((86 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 105);
                            }

                            if (pt.x >= testpos && pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                                if ((pt.y >= 126 && pt.y < (126 + (shop_ui_items_in_store[testx]->height()))) ||
                                    (pt.y <= 98 && pt.y >= (98 - (shop_ui_items_in_store[testx]->height())))) {
                                    GameUI_DrawItemInfo(item);
                                } else {
                                    return;
                                }
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                case BUILDING_ALCHEMY_SHOP:
                case BUILDING_MAGIC_SHOP:
                    testx = (pt.x) / 75;
                    // testx limits check
                    if (testx >= 0 && testx < 6) {
                        if (pt.y >= 152) {
                            testx += 6;
                        }

                        if (dialogue == DIALOGUE_SHOP_BUY_STANDARD)
                            item = &pParty->standartItemsInShops[window_SpeakInHouse->houseId()][testx];
                        else
                            item = &pParty->specialItemsInShops[window_SpeakInHouse->houseId()][testx];

                        if (item->uItemID != ITEM_NULL) {
                            if (pt.y > 152) {
                                testpos = 75 * testx - (shop_ui_items_in_store[testx]->width() / 2) + 40 - 450;
                            } else {
                                testpos = 75 * testx - (shop_ui_items_in_store[testx]->width() / 2) + 40;
                            }

                            if (pt.x >= testpos && pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                                if ((pt.y <= 308 && pt.y >= (308 - (shop_ui_items_in_store[testx]->height()))) ||
                                    (pt.y <= 152 && pt.y >= (152 - (shop_ui_items_in_store[testx]->height())))) {
                                    GameUI_DrawItemInfo(item);
                                } else {
                                    return;
                                }
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                default:
                    // v3 = render->pActiveZBuffer[mouse.x +
                    // pSRZBufferLineOffsets[mouse.y]] & 0xFFFF; if (!v3)
                    // return;
                    // v7 = &pParty->StandartItemsInShops[(unsigned
                    // int)window_SpeakInHouse->ptr_1C][v3 - 1]; if
                    // (dialogue == DIALOGUE_SHOP_BUY_SPECIAL) v7 =
                    // &pParty->SpecialItemsInShops[(unsigned
                    // int)window_SpeakInHouse->ptr_1C][v3 - 1];
                    // GameUI_DrawItemInfo(v7);
                    return;
                    break;
            }
        }

        if (dialogue >= DIALOGUE_SHOP_SELL && dialogue <= DIALOGUE_SHOP_REPAIR || dialogue == DIALOGUE_SHOP_DISPLAY_EQUIPMENT) {
            invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462 || !pParty->activeCharacter().GetItemListAtInventoryIndex(invindex))
                return;

            GameUI_DrawItemInfo(pParty->activeCharacter().GetItemAtInventoryIndex(invindex));
            return;
        }
    }

    if (buildingType <= BUILDING_MIRRORED_PATH_GUILD && dialogue == DIALOGUE_GUILD_BUY_BOOKS) {
        int testx = (pt.x - 32) / 70;
        if (testx >= 0 && testx < 6) {
            if (pt.y >= 250) {
                testx += 6;
            }

            item = &pParty->spellBooksInGuilds[window_SpeakInHouse->houseId()][testx];

            if (item->uItemID != ITEM_NULL) {
                int testpos;
                if (pt.y >= 250) {
                    testpos = 32 + 70 * testx - 420;
                } else {
                    testpos = 32 + 70 * testx;
                }

                if (pt.x >= testpos && pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                    if ((pt.y >= 90 && pt.y <= (90 + (shop_ui_items_in_store[testx]->height()))) || (pt.y >= 250 && pt.y <= (250 + (shop_ui_items_in_store[testx]->height())))) {
                        showSpellbookInfo(pParty->spellBooksInGuilds[window_SpeakInHouse->houseId()][testx].uItemID);
                    }
                }
            }
        }
    }
}

//----- (0041D3B7) --------------------------------------------------------
void GameUI_CharacterQuickRecord_Draw(GUIWindow *window, int characterIndex) {
    GraphicsImage *v13;              // eax@6
    PlayerFrame *v15;        // eax@12
    std::string spellName;   // eax@16
    int v36;                 // esi@22
    signed int uFramesetID;  // [sp+20h] [bp-8h]@9
    int uFramesetIDa;        // [sp+20h] [bp-8h]@18
    Character *player = &pParty->pCharacters[characterIndex];

    unsigned numActivePlayerBuffs = 0;
    for (const SpellBuff &buff : player->pCharacterBuffs)
        if (buff.Active())
            ++numActivePlayerBuffs;

    window->uFrameHeight = ((assets->pFontArrus->GetHeight() + 162) + ((numActivePlayerBuffs - 1) * assets->pFontArrus->GetHeight()));
    window->uFrameZ = window->uFrameWidth + window->uFrameX - 1;
    window->uFrameW = ((assets->pFontArrus->GetHeight() + 162) + ((numActivePlayerBuffs - 1) * assets->pFontArrus->GetHeight())) + window->uFrameY - 1;
    window->DrawMessageBox(0);

    if (player->IsEradicated()) {
        v13 = game_ui_player_face_eradicated;
    } else if (player->IsDead()) {
        v13 = game_ui_player_face_dead;
    } else {
        uFramesetID = pPlayerFrameTable->GetFrameIdByExpression(player->expression);
        if (!uFramesetID)
            uFramesetID = 1;
        if (player->expression == CHARACTER_EXPRESSION_TALK)
            v15 = pPlayerFrameTable->GetFrameBy_y(&player->_expression21_frameset, &player->_expression21_animtime, pMiscTimer->dt());
        else
            v15 = pPlayerFrameTable->GetFrameBy_x(uFramesetID, pMiscTimer->time());
        player->uExpressionImageIndex = v15->uTextureID - 1;
        v13 = game_ui_player_faces[characterIndex][v15->uTextureID - 1];
    }

    render->DrawTextureNew((window->uFrameX + 24) / 640.0f, (window->uFrameY + 24) / 480.0f, v13);

    // TODO(captainurist): do a 2nd rewrite here
    auto str =
        fmt::format("{::}{}\f00000\n", ui_character_header_text_color.tag(), NameAndTitle(player->name, player->classType))
        + fmt::format("{} : {::}{}\f00000 / {}\n",
                      localization->GetString(LSTR_HIT_POINTS),
                      UI_GetHealthManaAndOtherQualitiesStringColor(player->health, player->GetMaxHealth()).tag(),
                      player->health, player->GetMaxHealth())
        + fmt::format("{} : {::}{}\f00000 / {}\n",
                      localization->GetString(LSTR_SPELL_POINTS),
                      UI_GetHealthManaAndOtherQualitiesStringColor(player->mana, player->GetMaxMana()).tag(),
                      player->mana, player->GetMaxMana())
        + fmt::format("{}: {::}{}\f00000\n",
                     localization->GetString(LSTR_CONDITION),
                     GetConditionDrawColor(player->GetMajorConditionIdx()).tag(),
                     localization->GetCharacterConditionName(player->GetMajorConditionIdx()));

    if (player->uQuickSpell != SPELL_NONE)
        spellName = pSpellStats->pInfos[player->uQuickSpell].pShortName;
    else
        spellName = localization->GetString(LSTR_NONE);

    str += fmt::format("{}: {}", localization->GetString(LSTR_QUICK_SPELL), spellName);

    window->DrawText(assets->pFontArrus.get(), {120, 22}, colorTable.White, str);

    uFramesetIDa = 0;
    for (CharacterBuff i : player->pCharacterBuffs.indices()) {
        SpellBuff *buff = &player->pCharacterBuffs[i];
        if (buff->Active()) {
            v36 = uFramesetIDa++ * assets->pFontComic->GetHeight() + 134;
            window->DrawText(assets->pFontComic.get(), {52, v36},
                             ui_game_character_record_playerbuff_colors[i],
                             localization->GetCharacterBuffName(i));
            DrawBuff_remaining_time_string(
                v36, window, buff->GetExpireTime() - pParty->GetPlayingTime(),
                assets->pFontComic.get());
        }
    }

    auto active_spells = localization->FormatString(
        LSTR_FMT_ACTIVE_SPELLS_S,
        uFramesetIDa == 0 ? localization->GetString(LSTR_NONE) : "");
    window->DrawText(assets->pFontArrus.get(), {14, 114}, colorTable.White, active_spells);
}

void GameUI_DrawNPCPopup(int _this) {  // PopupWindowForBenefitAndJoinText
    if (!isHirelingsBlockedOnMap(engine->_currentLoadedMapId)) {
        FlatHirelings buf;
        buf.Prepare();

        if (_this + pParty->hirelingScrollPosition < buf.Size()) {
            NPCData *pNPC = getNPCData(-1 - pParty->hirelingScrollPosition - _this);
            std::string pText;

            if (pNPC) {
                if (pNPC->name == "Baby Dragon")
                    pText = pNPCTopics[512].pText;  // Baby dragon
                else
                    pText = pNPCStats->pProfessions[pNPC->profession].pBenefits;
                if (pText.empty()) {
                    pText = pNPCStats->pProfessions[pNPC->profession].pJoinText;
                }

                GUIWindow popup_window;
                popup_window.sHint.clear();
                popup_window.uFrameX = 38;
                popup_window.uFrameY = 60;
                popup_window.uFrameWidth = 276;
                popup_window.uFrameZ = 313;
                popup_window.uFrameHeight = assets->pFontArrus->CalcTextHeight(pText, popup_window.uFrameWidth, 0) + 2 * assets->pFontArrus->GetHeight() + 24;
                if (popup_window.uFrameHeight < 130)
                    popup_window.uFrameHeight = 130;
                popup_window.uFrameWidth = 400;
                popup_window.uFrameZ = popup_window.uFrameX + 399;
                popup_window.DrawMessageBox(0);

                auto tex_name = fmt::format("NPC{:03}", pNPC->uPortraitID);
                render->DrawTextureNew(
                    (popup_window.uFrameX + 22) / 640.0f,
                    (popup_window.uFrameY + 36) / 480.0f,
                    assets->getImage_ColorKey(tex_name));

                popup_window.DrawTitleText(assets->pFontArrus.get(), 0, 12, colorTable.PaleCanary, NameAndTitle(pNPC), 3);
                popup_window.uFrameWidth -= 24;
                popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
                popup_window.DrawText(assets->pFontArrus.get(), {100, 36}, colorTable.White, BuildDialogueString(pText, 0, pNPC));
            }
        }
    }
}

//----- (00416D62) --------------------------------------------------------
void UI_OnMouseRightClick(int mouse_x, int mouse_y) {
    std::string pStr;        // edi@85
    GUIWindow popup_window;  // [sp+4h] [bp-74h]@32

    if (current_screen_type == SCREEN_VIDEO || GetCurrentMenuID() == MENU_MAIN)
        return;


    unsigned int pX = mouse_x;
    unsigned int pY = mouse_y;

    // if ( render->bWindowMode )
    {
        // Reset right click mode and restart timer if cursor went to the very edge of screen
        // To enter it again need to release right mouse button and press it again inside game screen
        Pointi pt = Pointi(pX, pY);
        if (pt.x < 1 || pt.y < 1 || pt.x > 638 || pt.y > 478) {
            back_to_game();
            return;
        }
    }

    if (pParty->pPickedItem.uItemID != ITEM_NULL) {
        // Use item on character portrait
        for (int i = 0; i < pParty->pCharacters.size(); ++i) {
            if (pX > RightClickPortraitXmin[i] && pX < RightClickPortraitXmax[i] && pY > 375 && pY < 466) {
                pParty->activeCharacter().useItem(i, true);
                // Do not enter right click mode
                return;
            }
        }
    }

    // Otherwise pause game and enter right click mode until button will be released
    pEventTimer->setPaused(true);
    switch (current_screen_type) {
        case SCREEN_CASTING: {
            Inventory_ItemPopupAndAlchemy();
            break;
        }
        case SCREEN_CHEST: {
            if (!pParty->activeCharacter().CanAct()) {
                static std::string hint_reference;
                hint_reference = localization->FormatString(
                    LSTR_FMT_S_IS_IN_NO_CODITION_TO_S,
                    pParty->activeCharacter().name,
                    localization->GetString(LSTR_IDENTIFY_ITEMS)
                );

                popup_window.sHint = hint_reference;
                popup_window.uFrameWidth = 384;
                popup_window.uFrameHeight = 180;
                popup_window.uFrameY = 40;
                if ((signed int)pX <= 320)
                    popup_window.uFrameX = pX + 30;
                else
                    popup_window.uFrameX = pX - 414;
                popup_window.DrawMessageBox(0);
            } else {
                // this could be put into a chest function

                int chestheight =
                    9;  // pChestHeightsByType[pChests[(int)pGUIWindow_CurrentMenu->par1C].uChestBitmapID];
                int chestwidth = 9;
                int inventoryYCoord = (pY - 34) / 32;  // use pchestoffsets??
                int inventoryXCoord = (pX - 42) / 32;
                int invMatrixIndex =
                    inventoryXCoord + (chestheight * inventoryYCoord);

                if (inventoryYCoord >= 0 && inventoryYCoord < chestheight &&
                    inventoryXCoord >= 0 && inventoryXCoord < chestwidth) {
                    int chestindex = vChests[pGUIWindow_CurrentChest->chestId()].pInventoryIndices[invMatrixIndex];
                    if (chestindex < 0) {
                        invMatrixIndex = (-(chestindex + 1));
                        chestindex = vChests[pGUIWindow_CurrentChest->chestId()].pInventoryIndices[invMatrixIndex];
                    }

                    if (chestindex) {
                        int itemindex = chestindex - 1;

                        GameUI_DrawItemInfo(&vChests[pGUIWindow_CurrentChest->chestId()].igChestItems[itemindex]);
                    }
                }
            }
            break;
        }

        case SCREEN_GAME:  // In the main menu displays a pop-up window(В
                           // главном меню показывает всплывающее окно)
        {
            if (GetCurrentMenuID() > MENU_MAIN) break;
            if ((signed int)pY > (signed int)pViewport->uViewportBR_Y) {
                int characterIndex = pX / 118;
                if (characterIndex < 4) { // portaits zone
                    popup_window.sHint.clear();
                    popup_window.uFrameWidth = 400;
                    popup_window.uFrameHeight = 200;
                    popup_window.uFrameX = 38;
                    popup_window.uFrameY = 60;
                    GameUI_CharacterQuickRecord_Draw(&popup_window, characterIndex);
                }
            } else if ((int)pX > pViewport->uViewportBR_X) {
                if (pY >= 130) {
                    if (pX >= 476 && pX <= 636 && pY >= 240 && pY <= 300) {  // buff_tooltip zone
                        drawBuffPopupWindow();
                    } else if ((int)pX < 485 || (int)pX > 548 ||
                               (int)pY < 156 || (int)pY > 229) {  // NPC zone
                        if (!((signed int)pX < 566 || (signed int)pX > 629 ||
                              (signed int)pY < 156 || (signed int)pY > 229)) {
                            GameUI_DrawNPCPopup(1);  // NPC 2
                        }
                    } else {
                        GameUI_DrawNPCPopup(0);  // NPC 1
                    }
                } else {  // minimap zone
                    popup_window.sHint = GameUI_GetMinimapHintText();
                    popup_window.uFrameWidth = 256;
                    popup_window.uFrameX = 130;
                    popup_window.uFrameY = 140;
                    popup_window.uFrameHeight = 64;
                    popup_window.DrawMessageBox(0);
                }
            } else {  // game zone
                popup_window.sHint.clear();
                popup_window.uFrameWidth = 320;
                popup_window.uFrameHeight = 320;
                popup_window.uFrameX = pX - 350;
                if ((signed int)pX <= 320) popup_window.uFrameX = pX + 30;
                popup_window.uFrameY = 40;
                // if ( render->pRenderD3D )

                Pid v5 = engine->PickMouseInfoPopup().pid;
                /*else
                v5 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]];*/
                if (v5.type() == OBJECT_Actor) {
                    render->BeginScene2D();
                    popup_window.DrawMessageBox(1);
                    MonsterPopup_Draw(v5.id(), &popup_window);
                }
                if (v5.type() == OBJECT_Item) {
                    if (!(pObjectList->pObjects[pSpriteObjects[v5.id()].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE)) {
                        GameUI_DrawItemInfo(&pSpriteObjects[v5.id()].containing_item);
                    }
                }
            }
            break;
        }
        case SCREEN_BOOKS: {
            if (pGUIWindow_CurrentMenu->eWindowType != WINDOW_MapsBook ||
                (signed int)pX < (signed int)pViewport->uViewportTL_X ||
                (signed int)pX > (signed int)pViewport->uViewportBR_X ||
                (signed int)pY < (signed int)pViewport->uViewportTL_Y ||
                (signed int)pY > (signed int)pViewport->uViewportBR_Y ||
                ((popup_window.sHint = GetMapBookHintText(mouse_x, mouse_y)).empty())) {
                break;
            }
            popup_window.uFrameWidth = (assets->pFontArrus->GetLineWidth(popup_window.sHint) + 32) + 0.5f;
            popup_window.uFrameX = pX + 5;
            popup_window.uFrameY = pY + 5;
            popup_window.uFrameHeight = 64;
            popup_window.DrawMessageBox(0);
            break;
        }
        case SCREEN_CHARACTERS:
        case SCREEN_SHOP_INVENTORY:
        case SCREEN_CHEST_INVENTORY: {
            if ((signed int)pX > 467 && current_screen_type != SCREEN_SHOP_INVENTORY)
                Inventory_ItemPopupAndAlchemy();
            else if ((signed int)pY >= 345)
                break;
            else if (current_character_screen_window ==
                     WINDOW_CharacterWindow_Stats)  // 2DEvent -
                                                    // CharacerScreenStats
                CharacterUI_StatsTab_ShowHint();
            else if (current_character_screen_window ==
                     WINDOW_CharacterWindow_Skills)  // 2DEvent -
                                                     // CharacerScreenSkills
                CharacterUI_SkillsTab_ShowHint();
            else if (
                current_character_screen_window ==
                WINDOW_CharacterWindow_Inventory)  // 2DEvent -
                                                   // CharacerScreenInventory
                Inventory_ItemPopupAndAlchemy();
            break;
        }
        case SCREEN_SPELL_BOOK: {
            if (dword_507B00_spell_info_to_draw_in_popup != SPELL_NONE)
                DrawSpellDescriptionPopup(dword_507B00_spell_info_to_draw_in_popup);
            break;
        }
        case SCREEN_HOUSE: {
            if ((signed int)pY < 345 && (signed int)pX < 469)
                ShowPopupShopItem();
            if ((signed int)pX >= 469)
                ShowPopupShopSkills();
            break;
        }
        case SCREEN_PARTY_CREATION: {
            popup_window.sHint.clear();
            pStr = "";
            for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
                if (pButton->uButtonType == 1 && pButton->uButtonType != 3 &&
                    (signed int)pX > (signed int)pButton->uX &&
                    (signed int)pX < (signed int)pButton->uZ &&
                    (signed int)pY > (signed int)pButton->uY &&
                    (signed int)pY < (signed int)pButton->uW) {
                    switch (pButton->msg) {
                        case UIMSG_0:  // stats info
                            popup_window.sHint = localization->GetAttributeDescription(static_cast<CharacterAttributeType>(pButton->msg_param % 7));
                            pStr = localization->GetAttirubteName(static_cast<CharacterAttributeType>(pButton->msg_param % 7));
                            break;
                        case UIMSG_PlayerCreationClickPlus:  // Plus button info
                            pStr = localization->GetString(LSTR_ADD);
                            popup_window.sHint = localization->GetString(LSTR_SKILL_INCREASE_HINT);
                            break;
                        case UIMSG_PlayerCreationClickMinus:  // Minus button
                                                              // info
                            pStr = localization->GetString(LSTR_SUBTRACT);
                            popup_window.sHint = localization->GetString(LSTR_SKILL_DECREASE_HINT);
                            break;
                        case UIMSG_PlayerCreationSelectActiveSkill:  // Available
                                                                     // skill
                                                                     // button
                                                                     // info
                            pStr = localization->GetSkillName(
                                pParty->pCharacters
                                        [uPlayerCreationUI_SelectedCharacter]
                                    .GetSkillIdxByOrder(pButton->msg_param +
                                                        4));
                            popup_window
                                .sHint = localization->GetSkillDescription(
                                pParty->pCharacters
                                        [uPlayerCreationUI_SelectedCharacter]
                                    .GetSkillIdxByOrder(pButton->msg_param +
                                                        4));
                            break;
                        case UIMSG_PlayerCreationSelectClass:  // Available
                                                               // Class Info
                            popup_window.sHint = localization->GetClassDescription(static_cast<CharacterClass>(pButton->msg_param));
                            pStr = localization->GetClassName(static_cast<CharacterClass>(pButton->msg_param));
                            break;
                        case UIMSG_PlayerCreationClickOK:  // OK Info
                            popup_window.sHint = localization->GetString(
                                LSTR_ACCEPT_PARTY_HINT);
                            pStr = localization->GetString(LSTR_OK_BUTTON);
                            break;
                        case UIMSG_PlayerCreationClickReset:  // Clear info
                            popup_window.sHint = localization->GetString(
                                LSTR_RESET_HINT);
                            pStr = localization->GetString(LSTR_CLEAR_BUTTON);
                            break;
                        case UIMSG_PlayerCreation_SelectAttribute:  // Character
                                                                    // info
                            pStr = pParty->pCharacters[pButton->msg_param].name;
                            popup_window
                                .sHint = localization->GetClassDescription(
                                pParty->pCharacters[pButton->msg_param].classType);
                            break;
                        default:
                            break;
                    }
                    if (pButton->msg > UIMSG_44 &&
                        pButton->msg <=
                            UIMSG_PlayerCreationRemoveDownSkill) {  // Sellected
                                                                    // skills info
                        pY = 0;
                        if (pParty->pCharacters[pButton->msg_param].GetSkillIdxByOrder(pButton->msg - UIMSG_48) != CHARACTER_SKILL_INVALID) {
                            static std::string hint_reference;
                            hint_reference = CharacterUI_GetSkillDescText(
                                pButton->msg_param,
                                pParty->pCharacters[pButton->msg_param]
                                    .GetSkillIdxByOrder(pButton->msg -
                                                        UIMSG_48));

                            popup_window.sHint = hint_reference;
                            pStr = localization->GetSkillName(
                                pParty->pCharacters[pButton->msg_param]
                                    .GetSkillIdxByOrder(pButton->msg -
                                                        UIMSG_48));
                        }
                    }
                }
            }
            if (!popup_window.sHint.empty()) {
                std::string sHint = popup_window.sHint;
                popup_window.sHint.clear();
                popup_window.uFrameWidth = 384;
                popup_window.uFrameHeight = 256;
                popup_window.uFrameX = 128;
                popup_window.uFrameY = 40;
                popup_window.uFrameHeight =
                    assets->pFontSmallnum->CalcTextHeight(
                        sHint, popup_window.uFrameWidth, 24) +
                    2 * assets->pFontLucida->GetHeight() + 24;
                popup_window.uFrameZ =
                    popup_window.uFrameX + popup_window.uFrameWidth - 1;
                popup_window.uFrameW =
                    popup_window.uFrameY + popup_window.uFrameHeight - 1;
                popup_window.DrawMessageBox(0);
                popup_window.uFrameX += 12;
                popup_window.uFrameWidth -= 24;
                popup_window.uFrameY += 12;
                popup_window.uFrameHeight -= 12;
                popup_window.uFrameZ =
                    popup_window.uFrameX + popup_window.uFrameWidth - 1;
                popup_window.uFrameW =
                    popup_window.uFrameY + popup_window.uFrameHeight - 1;

                std::string str = fmt::format("{::}{}\f00000\n", colorTable.PaleCanary.tag(), pStr);
                popup_window.DrawTitleText(assets->pFontCreate.get(), 0, 0, colorTable.White, str, 3);
                popup_window.DrawText(assets->pFontSmallnum.get(), {1, assets->pFontLucida->GetHeight()}, colorTable.White, sHint);
            }
            break;
        }
        default:
            break;
    }
    holdingMouseRightButton = true;
}

//----- (00416196) --------------------------------------------------------
void Inventory_ItemPopupAndAlchemy() {
    if (rightClickItemActionPerformed) {
        // Forbid doing anything until right click has beed released
        return;
    }

    ItemGen *item = nullptr;

    static const std::array<int, 6> ringsX = {0x1EA, 0x21A, 0x248, 0x1EA, 0x21A, 0x248};
    static const std::array<int, 6> ringsY = {0x0CA, 0x0CA, 0x0CA, 0x0FA, 0x0FA, 0x0FA};

    int pX, pY;
    mouse->GetClickPos(&pX, &pY);

    int inventoryYCoord = (pY - 17) / 32;
    int inventoryXCoord = (pX - 14) / 32;
    int invMatrixIndex = inventoryXCoord + (14 * inventoryYCoord);  // INVETORYSLOTSWIDTH

    if (pX <= 13 || pX >= 462) {   // items on the player ragdoll
        static const int glovex = 586;
        static const int glovey = 88;

        static const int amuletx = 493;
        static const int amulety = 91;

        static const int slotSize = 32;

        if (!ringscreenactive()) { // rings not displayed
            int item_pid = (render->pActiveZBuffer[pX + pY * render->GetRenderDimensions().w] & 0xFFFF) - 1;
            // zbuffer still used for paperdolls

            if (item_pid != -1) {
                item = &pParty->activeCharacter().pInventoryItemList[item_pid];
            }
        } else {  // rings displayed
            ItemSlot pos = ITEM_SLOT_INVALID;

            if (mouse->uMouseX < 490 || mouse->uMouseX > 618) {
                return;
            }

            if (mouse->uMouseY < 88 || mouse->uMouseY > 282) {
                return;
            }

            if (mouse->uMouseX >= amuletx && mouse->uMouseX <= (amuletx + slotSize) &&
                mouse->uMouseY >= amulety && mouse->uMouseY <= (amulety + 2 * slotSize)) {
                pos = ITEM_SLOT_AMULET;
            }

            if (mouse->uMouseX >= glovex && mouse->uMouseX <= (glovex + slotSize) &&
                mouse->uMouseY >= glovey && mouse->uMouseY <= (glovey + 2 * slotSize)) {
                pos = ITEM_SLOT_GAUNTLETS;
            }

            for (int i = 0; i < 6; ++i) {
                if (mouse->uMouseX >= ringsX[i] && mouse->uMouseX <= (ringsX[i] + slotSize) &&
                    mouse->uMouseY >= ringsY[i] && mouse->uMouseY <= (ringsY[i] + slotSize)) {
                    pos = ringSlot(i);
                }
            }

            if (pos != ITEM_SLOT_INVALID) {
                item = pParty->activeCharacter().GetItem(pos);
            }
        }
    } else {
        item = pParty->activeCharacter().GetItemAtInventoryIndex(invMatrixIndex);
    }

    if (!item) {  // no item
        return;
    }

    // check character condition
    if (!pParty->activeCharacter().CanAct()) {
        static std::string hint_reference;
        hint_reference = localization->FormatString(LSTR_FMT_S_IS_IN_NO_CODITION_TO_S, pParty->activeCharacter().name,
                                                    localization->GetString(LSTR_IDENTIFY_ITEMS));

        GUIWindow message_window;
        message_window.sHint = hint_reference;
        message_window.uFrameWidth = 384;
        message_window.uFrameHeight = 180;
        if (pX <= 320) {
            message_window.uFrameX = pX + 30;
        } else {
            message_window.uFrameX = pX - 414;
        }
        message_window.uFrameY = 40;
        message_window.DrawMessageBox(0);
        return;
    }

    CombinedSkillValue alchemySkill = pParty->activeCharacter().getActualSkillValue(CHARACTER_SKILL_ALCHEMY);

    if (pParty->pPickedItem.uItemID == ITEM_POTION_BOTTLE) {
        GameUI_DrawItemInfo(item);
        return;
    }

    if (isPotion(pParty->pPickedItem.uItemID) && isPotion(item->uItemID)) { // potion mixing
        ItemId potionSrc1 = item->uItemID;
        ItemId potionSrc2 = pParty->pPickedItem.uItemID;

        ItemId potionID;
        if (pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST || item->uItemID == ITEM_POTION_CATALYST) {
            potionID = ITEM_POTION_CATALYST;
        } else {
            potionID = pItemTable->potionCombination[potionSrc2][potionSrc1];
        }

        if (potionID == ITEM_NULL) {
            // Combining same potions
            GameUI_DrawItemInfo(item);
            return;
        }

        int damage_level = 0;
        if (!isPotion(potionID)) {
            // In this case potionID represent damage level
            damage_level = std::to_underlying(potionID);
        } else {
            // potionID >= ITEM_POTION_CURE_WOUNDS && potionID <= ITEM_POTION_CURE_WEAKNESS does not require skill
            if (potionID >= ITEM_POTION_CURE_DISEASE &&
                    potionID <= ITEM_POTION_AWAKEN &&
                    alchemySkill.mastery() == CHARACTER_SKILL_MASTERY_NONE) {
                damage_level = 1;
            }
            if (potionID >= ITEM_POTION_HASTE &&
                    potionID <= ITEM_POTION_CURE_INSANITY &&
                    alchemySkill.mastery() <= CHARACTER_SKILL_MASTERY_NOVICE) {
                damage_level = 2;
            }
            if (potionID >= ITEM_POTION_MIGHT_BOOST &&
                    potionID <= ITEM_POTION_BODY_RESISTANCE &&
                    alchemySkill.mastery() <= CHARACTER_SKILL_MASTERY_EXPERT) {
                damage_level = 3;
            }
            if (potionID >= ITEM_POTION_STONE_TO_FLESH &&
                    alchemySkill.mastery() <= CHARACTER_SKILL_MASTERY_MASTER) {
                damage_level = 4;
            }
        }

        if (damage_level > 0) {
            pParty->activeCharacter().RemoveItemAtInventoryIndex(invMatrixIndex);

            if (damage_level == 1) {
                pParty->activeCharacter().receiveDamage(grng->random(11) + 10, DAMAGE_FIRE);
            } else if (damage_level == 2) {
                pParty->activeCharacter().receiveDamage(grng->random(71) + 30, DAMAGE_FIRE);
                pParty->activeCharacter().ItemsPotionDmgBreak(1);  // break 1
            } else if (damage_level == 3) {
                pParty->activeCharacter().receiveDamage(grng->random(201) + 50, DAMAGE_FIRE);
                pParty->activeCharacter().ItemsPotionDmgBreak(5);  // break 5
            } else if (damage_level >= 4) {
                pParty->activeCharacter().SetCondition(CONDITION_ERADICATED, 0);
                pParty->activeCharacter().ItemsPotionDmgBreak(0);  // break everything
            }

            pAudioPlayer->playUISound(SOUND_fireBall);
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);

            Vec3f pos = pParty->pos + Vec3f(0, 0, pParty->eyeLevel) + Vec3f::fromPolar(64, pParty->_viewYaw, pParty->_viewPitch);
            SpriteObject::dropItemAt(SPRITE_SPELL_FIRE_FIREBALL_IMPACT, pos, 0);
            if (pParty->activeCharacter().CanAct()) {
                pParty->activeCharacter().playReaction(SPEECH_POTION_EXPLODE);
            }
            engine->_statusBar->setEvent(LSTR_OOPS);
            mouse->RemoveHoldingItem();
            rightClickItemActionPerformed = true;
            return;
        } else {  // if ( damage_level == 0 )
            if (item->uItemID == ITEM_POTION_CATALYST && pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST) {
                // Both potions are catalyst: power is maximum of two
                item->potionPower = std::max(item->potionPower, pParty->pPickedItem.potionPower);
            } else if (item->uItemID == ITEM_POTION_CATALYST || pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST) {
                // One of the potion is catalyst: power of potion is replaced by power of catalyst
                if (item->uItemID == ITEM_POTION_CATALYST) {
                    item->uItemID = pParty->pPickedItem.uItemID;
                } else {
                    item->potionPower = pParty->pPickedItem.potionPower;
                }
            } else {
                item->uItemID = potionID;
                item->potionPower = (pParty->pPickedItem.potionPower + item->potionPower) / 2;
                // Can be zero even for valid potion combination when resulting potion is of lower grade than it's components
                // Example: "Cure Paralysis(white) + Cure Wounds(red) = Cure Wounds(red)"
                if (pItemTable->potionNotes[potionSrc1][potionSrc2] != 0) {
                    pParty->activeCharacter().SetVariable(VAR_AutoNotes, pItemTable->potionNotes[potionSrc1][potionSrc2]);
                }
            }
            if (!(pItemTable->pItems[item->uItemID].uItemID_Rep_St)) {
                item->uAttributes |= ITEM_IDENTIFIED;
            }
            pParty->activeCharacter().playReaction(SPEECH_POTION_SUCCESS);
            mouse->RemoveHoldingItem();
            rightClickItemActionPerformed = true;
            int bottleId = pParty->activeCharacter().AddItem(-1, ITEM_POTION_BOTTLE);
            if (bottleId) {
                pParty->activeCharacter().pInventoryItemList[bottleId - 1].uAttributes = ITEM_IDENTIFIED;
            } else {
                // Can't fit bottle in inventory - place it in hand
                ItemGen bottle;
                bottle.uItemID = ITEM_POTION_BOTTLE;
                bottle.uAttributes = ITEM_IDENTIFIED;
                pParty->setHoldingItem(&bottle);
            }
            return;
        }
    }

    if (pParty->pPickedItem.uItemID == ITEM_POTION_RECHARGE_ITEM) {
        if (item->isWand()) { // can recharge only wands
            int maxChargesDecreasePercent = 70 - pParty->pPickedItem.potionPower;
            if (maxChargesDecreasePercent < 0) {
                maxChargesDecreasePercent = 0;
            }

            float invMaxChargesDecrease = (100 - maxChargesDecreasePercent) * 0.01;
            int newCharges = item->uMaxCharges * invMaxChargesDecrease;

            // Disallow if wand will lose charges
            if (newCharges < item->uNumCharges) {
                engine->_statusBar->setEvent(LSTR_WAND_ALREADY_CHARGED);
                pAudioPlayer->playUISound(SOUND_spellfail0201);
            } else {
                item->uMaxCharges = item->uNumCharges = newCharges;
                // Effect and sound was not present previously
                item->uAttributes |= ITEM_AURA_EFFECT_GREEN;
                ItemEnchantmentTimer = Duration::fromRealtimeSeconds(2);
                pAudioPlayer->playSpellSound(SPELL_WATER_RECHARGE_ITEM, false, SOUND_MODE_UI);
            }

            mouse->RemoveHoldingItem();
            rightClickItemActionPerformed = true;
            return;
        }
        GameUI_DrawItemInfo(item);
        return;
    }

    if (pParty->pPickedItem.uItemID == ITEM_POTION_HARDEN_ITEM) {
        if (item->IsBroken() || isArtifact(item->uItemID)) {
            // Sound error and stop right click item actions until button is released
            pAudioPlayer->playUISound(SOUND_error);
            rightClickItemActionPerformed = true;
            return;
        }
        if (item->isWeapon() || item->isPassiveEquipment() || item->isWand()) {
            item->uAttributes |= ITEM_AURA_EFFECT_RED | ITEM_HARDENED;

            // Sound was missing previously
            pAudioPlayer->playSpellSound(SPELL_WATER_ENCHANT_ITEM, false, SOUND_MODE_UI);

            ItemEnchantmentTimer = Duration::fromRealtimeSeconds(2);
            mouse->RemoveHoldingItem();
            rightClickItemActionPerformed = true;
            return;
        }
        GameUI_DrawItemInfo(item);
        return;
    }

    if (isEnchantingPotion(pParty->pPickedItem.uItemID)) {
        if (item->IsBroken() || isArtifact(item->uItemID) || item->isPassiveEquipment() || item->isWand() || isAncientWeapon(item->uItemID)) {
            // Sound error and stop right click item actions until button is released
            pAudioPlayer->playUISound(SOUND_error);
            rightClickItemActionPerformed = true;
            return;
        }
        if (item->isWeapon()) {
            if (item->special_enchantment != ITEM_ENCHANTMENT_NULL || item->attributeEnchantment) {
                // Sound error and stop right click item actions until button is released
                pAudioPlayer->playUISound(SOUND_error);
                rightClickItemActionPerformed = true;
                return;
            }

            Duration effectTime = Duration::fromMinutes(30 * pParty->pPickedItem.potionPower);
            item->UpdateTempBonus(pParty->GetPlayingTime());
            item->special_enchantment = potionEnchantment(pParty->pPickedItem.uItemID);
            item->uExpireTime = pParty->GetPlayingTime() + effectTime;
            // Sound was missing previously
            item->uAttributes |= ITEM_TEMP_BONUS | ITEM_AURA_EFFECT_RED;
            pAudioPlayer->playSpellSound(SPELL_WATER_ENCHANT_ITEM, false, SOUND_MODE_UI);

            ItemEnchantmentTimer = Duration::fromRealtimeSeconds(2);
            mouse->RemoveHoldingItem();
            rightClickItemActionPerformed = true;
            return;
        }
        GameUI_DrawItemInfo(item);
        return;
    }

    if (isReagent(pParty->pPickedItem.uItemID) && item->uItemID == ITEM_POTION_BOTTLE) {
        item->potionPower = alchemySkill.level() + pParty->pPickedItem.GetDamageDice();
        switch (pParty->pPickedItem.uItemID) {
            case ITEM_REAGENT_WIDOWSWEEP_BERRIES:
            case ITEM_REAGENT_CRUSHED_ROSE_PETALS:
            case ITEM_REAGENT_VIAL_OF_TROLL_BLOOD:
            case ITEM_REAGENT_RUBY:
            case ITEM_REAGENT_DRAGONS_EYE:
                item->uItemID = ITEM_POTION_CURE_WOUNDS;
                break;

            case ITEM_REAGENT_PHIRNA_ROOT:
            case ITEM_REAGENT_METEORITE_FRAGMENT:
            case ITEM_REAGENT_HARPY_FEATHER:
            case ITEM_REAGENT_MOONSTONE:
            case ITEM_REAGENT_ELVISH_TOADSTOOL:
                item->uItemID = ITEM_POTION_MAGIC;
                break;

            case ITEM_REAGENT_POPPYSNAPS:
            case ITEM_REAGENT_FAE_DUST:
            case ITEM_REAGENT_SULFUR:
            case ITEM_REAGENT_GARNET:
            case ITEM_REAGENT_VIAL_OF_DEVIL_ICHOR:
                item->uItemID = ITEM_POTION_CURE_WEAKNESS;
                break;

            case ITEM_REAGENT_MUSHROOM:
            case ITEM_REAGENT_OBSIDIAN:
            case ITEM_REAGENT_VIAL_OF_OOZE_ENDOPLASM:
            case ITEM_REAGENT_MERCURY:
            case ITEM_REAGENT_PHILOSOPHERS_STONE:
                item->uItemID = ITEM_POTION_CATALYST;
                break;
            default:
                break;
        }

        pParty->activeCharacter().playReaction(SPEECH_POTION_SUCCESS);
        mouse->RemoveHoldingItem();
        rightClickItemActionPerformed = true;
        return;
    }

    GameUI_DrawItemInfo(item);
}

//----- (0045828B) --------------------------------------------------------
Color GetSpellColor(signed int a1) {
    if (a1 == 0) return colorTable.White;
    if (a1 < 12) return colorTable.DarkOrange;
    if (a1 < 23) return colorTable.Anakiwa;
    if (a1 < 34) return colorTable.AzureRadiance;
    if (a1 < 45) return colorTable.Gray;
    if (a1 < 56) return colorTable.Mercury;
    if (a1 < 67) return colorTable.PurplePink;
    if (a1 < 78) return colorTable.FlushOrange;
    if (a1 < 89) return colorTable.PaleCanary;
    if (a1 < 100)
        return colorTable.MoonRaker;
    else
        assert(false);

    logger->warning("No color returned - GetSpellColor!");
    return colorTable.White;
}

//----- (004B46F8) --------------------------------------------------------
uint64_t GetExperienceRequiredForLevel(int level) {
    int effectiveLevel = 0;
    for (int i = 0; i < level; ++i)
        effectiveLevel += i + 1;
    return (uint64_t)(1000 * effectiveLevel);
}
