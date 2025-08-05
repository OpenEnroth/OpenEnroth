#include <array>
#include <algorithm>
#include <string>
#include <utility>

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
#include "Engine/Tables/PortraitFrameTable.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Party.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Time/Timer.h"
#include "Engine/Conditions.h"
#include "Engine/Objects/ActorEnumFunctions.h"
#include "Engine/Spells/SpellEnumFunctions.h"

#include "Io/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/Books/MapBook.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIPopup.h"

#include "ItemGrid.h"
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

// OE addition - colors for monster special attack text.
static constexpr IndexedArray<Color, SPECIAL_ATTACK_FIRST, SPECIAL_ATTACK_LAST> monsterSpecialAttackColors = {
    {SPECIAL_ATTACK_CURSE,          colorTable.Cioccolato},
    {SPECIAL_ATTACK_WEAK,           colorTable.Mimosa},
    {SPECIAL_ATTACK_SLEEP,          colorTable.ScienceBlue},
    {SPECIAL_ATTACK_DRUNK,          colorTable.NavyBlue},
    {SPECIAL_ATTACK_INSANE,         colorTable.JazzberryJam},
    {SPECIAL_ATTACK_POISON_WEAK,    colorTable.Eucalyptus},
    {SPECIAL_ATTACK_POISON_MEDIUM,  colorTable.GreenTeal},
    {SPECIAL_ATTACK_POISON_SEVERE,  colorTable.VibrantGreen},
    {SPECIAL_ATTACK_DISEASE_WEAK,   colorTable.FlushOrange},
    {SPECIAL_ATTACK_DISEASE_MEDIUM, colorTable.DarkOrange},
    {SPECIAL_ATTACK_DISEASE_SEVERE, colorTable.OrangeyRed},
    {SPECIAL_ATTACK_PARALYZED,      colorTable.Primrose},
    {SPECIAL_ATTACK_UNCONSCIOUS,    colorTable.StarkWhite},
    {SPECIAL_ATTACK_DEAD,           colorTable.BloodRed},
    {SPECIAL_ATTACK_PETRIFIED,      colorTable.MediumGrey},
    {SPECIAL_ATTACK_ERADICATED,     colorTable.MoonRaker},
    {SPECIAL_ATTACK_BREAK_ANY,      colorTable.LaserLemon},
    {SPECIAL_ATTACK_BREAK_ARMOR,    colorTable.LaserLemon},
    {SPECIAL_ATTACK_BREAK_WEAPON,   colorTable.LaserLemon},
    {SPECIAL_ATTACK_STEAL,          colorTable.DirtyYellow},
    {SPECIAL_ATTACK_AGING,          colorTable.White},
    {SPECIAL_ATTACK_MANA_DRAIN,     colorTable.BoltBlue},
    {SPECIAL_ATTACK_FEAR,           colorTable.CornFlowerBlue}
};

// OE addition - colors for monster special ability text.
static constexpr IndexedArray<Color, MONSTER_SPECIAL_ABILITY_FIRST, MONSTER_SPECIAL_ABILITY_LAST> monsterSpecialAbilityColors = {
    {MONSTER_SPECIAL_ABILITY_MULTI_SHOT, colorTable.Mercury},
    {MONSTER_SPECIAL_ABILITY_SUMMON, colorTable.EasternBlue},
    {MONSTER_SPECIAL_ABILITY_EXPLODE, colorTable.Sunflower}
};

void Inventory_ItemPopupAndAlchemy();
Color GetSpellColor(SpellId spellId);
uint64_t GetExperienceRequiredForLevel(int level);

/**
 * @offset 0x4179BC
 */
static void CharacterUI_DrawTooltip(std::string_view title, std::string_view content) {
    Pointi pt = mouse->position();

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

    render->SetUIClipRect(Recti(uX, uY, uWidth, uHeight));

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
        render->SetUIClipRect(Recti(uX + messagebox_corner_x->width(), uY,
                              uWidth - messagebox_corner_z->width() - messagebox_corner_x->width(),
                              uHeight));

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
        render->SetUIClipRect(Recti(uX, uY + messagebox_corner_x->height(),
                              uWidth,
                              uHeight - messagebox_corner_y->height() - messagebox_corner_x->height()));

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
void GameUI_DrawItemInfo(Item *inspect_item) {
    unsigned int frameXpos;     // eax@3
    int v34;             // esi@81
    GUIWindow iteminfo_window;  // [sp+208h] [bp-70h]@2
    int v85;

    if (inspect_item->itemId == ITEM_NULL)
        return;

    auto inspect_item_image = assets->getImage_ColorKey(inspect_item->GetIconName());

    iteminfo_window.sHint.clear();
    iteminfo_window.uFrameWidth = 384;
    iteminfo_window.uFrameHeight = 180;
    iteminfo_window.uFrameY = 40;

    Pointi pt = mouse->position();
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

    if (!pItemTable->items[inspect_item->itemId].identifyAndRepairDifficulty)
        inspect_item->SetIdentified();

    int GoldAmount = 0;
    if (inspect_item->isGold()) {
        GoldAmount = inspect_item->goldAmount;
    }

    if (pParty->hasActiveCharacter()) {
        // try to identify
        if (!inspect_item->IsIdentified()) {
            if (pParty->activeCharacter().CanIdentify(*inspect_item) == 1)
                inspect_item->SetIdentified();
            SpeechId speech = SPEECH_ID_ITEM_FAIL;
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
            if (pParty->activeCharacter().CanRepair(*inspect_item) == 1)
                inspect_item->flags = inspect_item->flags & ~ITEM_BROKEN | ITEM_IDENTIFIED;
            SpeechId speech = SPEECH_REPAIR_FAIL;
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
        render->SetUIClipRect(Recti(
            iteminfo_window.uFrameX + 12, iteminfo_window.uFrameY + 12,
            iteminfo_window.uFrameWidth - 24, iteminfo_window.uFrameHeight - 24));
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
        render->SetUIClipRect(Recti(
            iteminfo_window.uFrameX + 12, iteminfo_window.uFrameY + 12,
            iteminfo_window.uFrameWidth - 24, iteminfo_window.uFrameHeight - 24));
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
            pItemTable->items[inspect_item->itemId].unidentifiedName, 3);
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
        LSTR_TYPE_S,
        pItemTable->items[inspect_item->itemId].unidentifiedName);

    switch (inspect_item->type()) {
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
            text[2] = fmt::format("{}: {}", localization->GetString(LSTR_POWER), inspect_item->GetReagentPower());
        } else if (inspect_item->standardEnchantment) {
            text[2] = fmt::format("{}: {} +{}",
                                  localization->GetString(LSTR_SPECIAL),
                                  pItemTable->standardEnchantments[*inspect_item->standardEnchantment].attributeName,
                                  inspect_item->standardEnchantmentStrength);
        } else if (inspect_item->specialEnchantment != ITEM_ENCHANTMENT_NULL) {
            text[2] = fmt::format("{}: {}",
                                  localization->GetString(LSTR_SPECIAL),
                                  pItemTable->specialEnchantments[inspect_item->specialEnchantment].description);
        } else if (inspect_item->isWand()) {
            text[2] = fmt::sprintf(localization->GetString(LSTR_FMT_S_U_OUT_OF_U),
                                   localization->GetString(LSTR_CHARGES),
                                   inspect_item->numCharges,
                                   inspect_item->maxCharges);
        }
    }
    iteminfo_window.uFrameWidth -= 12;
    iteminfo_window.uFrameZ = iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
    iteminfo_window.uFrameW = iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
    int Str_int = (3 * (assets->pFontArrus->GetHeight() + 8));
    for (const std::string &s : text)
        if (!s.empty())
            Str_int += assets->pFontComic->CalcTextHeight(s, iteminfo_window.uFrameWidth, 100) + 3;
    if (!pItemTable->items[inspect_item->itemId].description.empty())
        Str_int += assets->pFontSmallnum->CalcTextHeight(
            pItemTable->items[inspect_item->itemId].description,
            iteminfo_window.uFrameWidth, 100);
    iteminfo_window.uFrameHeight = inspect_item_image->height() + itemYspacing + 54;
    if ((signed int)Str_int > (signed int)iteminfo_window.uFrameHeight)
        iteminfo_window.uFrameHeight = (unsigned int)Str_int;
    if (inspect_item->flags & ITEM_TEMP_BONUS &&
        (inspect_item->specialEnchantment != ITEM_ENCHANTMENT_NULL || inspect_item->standardEnchantment))
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
    render->SetUIClipRect(Recti(
        iteminfo_window.uFrameX + 12, iteminfo_window.uFrameY + 12,
        iteminfo_window.uFrameWidth - 24, iteminfo_window.uFrameHeight - 24));
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
            v34 += assets->pFontComic->CalcTextHeight(s, iteminfo_window.uFrameWidth, 100) + 3;
        }
    }
    if (!pItemTable->items[inspect_item->itemId].description.empty())
        iteminfo_window.DrawText(assets->pFontSmallnum.get(), {100, v34}, colorTable.White, pItemTable->items[inspect_item->itemId].description);
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
        if ((inspect_item->flags & ITEM_TEMP_BONUS) &&
            (inspect_item->specialEnchantment != ITEM_ENCHANTMENT_NULL || inspect_item->standardEnchantment)) {
            LongCivilDuration d = (inspect_item->enchantmentExpirationTime - pParty->GetPlayingTime()).toLongCivilDuration();

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
        if (inspect_item->flags & ITEM_STOLEN) {
            txt3 = localization->GetString(LSTR_STOLEN);
        } else {
            if (!(inspect_item->flags & ITEM_HARDENED)) {
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

/**
 * Render the monster info popup
 * @param uActorID ID of the actor to show info for
 * @param pWindow The window to render into, or `null` to measure content only
 * @return Actual width/height needed to render
 */
std::pair<int, int> MonsterPopup_Draw(unsigned int uActorID, GUIWindow *pWindow) {
    /// Geometry hardcoded constants
    const int X_LEFT_COLUMN = 12                            // "Effects" label
        , X_EFFECT_LIST = X_LEFT_COLUMN + 16                // Effects list
        , X_RIGHT_COLUMN = 150                              // Most right-side labels
        , X_RIGHT_INDENTED = X_RIGHT_COLUMN + 20            // Resistance labels & extended spells/specials
        , X_RIGHT_DEFAULT_SPELLS = X_RIGHT_COLUMN + 70      // Spells when "extended" is off
        , X_RIGHT_DATA = X_RIGHT_COLUMN + 91                // Numbers
        , X_POS_DOLL = X_LEFT_COLUMN + 1                    // 1px frame
        , Y_POS_DOLL = 52
        , SIZE_DOLL = 128
        , Y_EFFECT_LIST = Y_POS_DOLL + SIZE_DOLL            // Lower edge doll frame - add an empty line!
        , RIGHT_BOTTOM_MARGIN = 16;                         // Added to measured bottom and right edge of rendered text

    static Actor pMonsterInfoUI_Doll;
    MonsterInfo &monsterInfo = pActors[uActorID].monsterInfo;

    /*------------------------------- Top and Doll -------------------------------*/
    if (pWindow) {
        Duration actionLen;
        if (monsterInfo.id == pMonsterInfoUI_Doll.monsterInfo.id) {
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

        Recti doll_rect(pWindow->uFrameX + X_POS_DOLL, pWindow->uFrameY + Y_POS_DOLL, SIZE_DOLL, SIZE_DOLL);

        {
            SpriteFrame *Portrait_Sprite = pSpriteFrameTable->GetFrame(
                    pActors[uActorID]
                            .spriteIds[pMonsterInfoUI_Doll.currentActionAnimation],
                    pMonsterInfoUI_Doll.currentActionTime);

            // Draw portrait border
            render->ResetUIClipRect();
            render->FillRectFast(doll_rect.x, doll_rect.y, SIZE_DOLL, SIZE_DOLL, colorTable.Black);

            Recti frameRect(doll_rect.topLeft() - Pointi(1, 1), doll_rect.bottomRight() + Pointi(1, 1));
            render->BeginLines2D();
            render->RasterLine2D(frameRect.topLeft(), frameRect.topRight(), colorTable.Jonquil);
            render->RasterLine2D(frameRect.topRight(), frameRect.bottomRight(), colorTable.Jonquil);
            render->RasterLine2D(frameRect.bottomRight(), frameRect.bottomLeft(), colorTable.Jonquil);
            render->RasterLine2D(frameRect.bottomLeft(), frameRect.topLeft(), colorTable.Jonquil);
            render->EndLines2D();

            // Draw portrait
            int Popup_Y_Offset = monster_popup_y_offsets[monsterTypeForMonsterId(monsterInfo.id)] - 40;
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
        pWindow->DrawTitleText(assets->pFontComic.get(), 0, 12, colorTable.PaleCanary, str, 3);

        // health bar
        Actor::DrawHealthBar(&pActors[uActorID], pWindow);
    }

    /*------------------------------- Determine Detail Level -------------------------------*/
    // Debug option for full info
    bool monster_full_informations = engine->config->debug.FullMonsterID.value();
    bool normal_level = monster_full_informations;
    bool expert_level = monster_full_informations;
    bool master_level = monster_full_informations;
    bool grandmaster_level = monster_full_informations;
    bool for_effects = monster_full_informations;
    bool showCurrentHp = monster_full_informations || pParty->pPartyBuffs[PARTY_BUFF_DETECT_LIFE].Active();

    if (pParty->hasActiveCharacter()) {
        int skill_points = 0;
        Mastery skill_mastery = MASTERY_NONE;
        CombinedSkillValue idMonsterSkill = pParty->activeCharacter().getActualSkillValue(SKILL_MONSTER_ID);

        if ((skill_points = idMonsterSkill.level()) > 0) {
            skill_mastery = idMonsterSkill.mastery();
            if (skill_mastery == MASTERY_NOVICE) {
                if (skill_points + 10 >= monsterInfo.level) {
                    normal_level = true;
                }
            } else if (skill_mastery == MASTERY_EXPERT) {
                if (2 * skill_points + 10 >= monsterInfo.level) {
                    normal_level = true;
                    expert_level = true;
                }
            } else if (skill_mastery == MASTERY_MASTER) {
                if (3 * skill_points + 10 >= monsterInfo.level) {
                    normal_level = true;
                    expert_level = true;
                    master_level = true;
                    for_effects = true;
                }
            } else if (skill_mastery == MASTERY_GRANDMASTER) {
                normal_level = true;
                expert_level = true;
                master_level = true;
                grandmaster_level = true;
                for_effects = true;
            }
        }

        // Only play reaction when right click on actor initially
        if (pActors[uActorID].aiState != Dead && pActors[uActorID].aiState != Dying &&
            !holdingMouseRightButton && skill_mastery != MASTERY_NONE) {
            SpeechId speech;
            if (normal_level || expert_level || master_level || grandmaster_level) {
                if (monsterInfo.level >= pParty->activeCharacter().uLevel - 5)
                    speech = SPEECH_ID_MONSTER_STRONG;
                else
                    speech = SPEECH_ID_MONSTER_WEAK;
            } else {
                speech = SPEECH_ID_MONSTER_FAIL;
            }
            pParty->activeCharacter().playReaction(speech);
        }
    }

    // Additionally show Attack2 if Attack1 is visible, and Special Attack (Break, Insanity...) if Spells are visible:
    bool extended = engine->config->settings.ExtendedMonsterInfo.value();

    /*------------------------------- Left side -------------------------------*/
    GUIFont *font = assets->pFontSmallnum.get();
    int lineAdvance = assets->pFontSmallnum->GetHeight() - 3;
    int pTextHeight = Y_EFFECT_LIST + lineAdvance;  // Start effects below portrait
    if (pWindow) pWindow->DrawText(font, {X_LEFT_COLUMN, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_EFFECTS));
    pTextHeight += lineAdvance;
    if (!for_effects) {
        if (pWindow) pWindow->DrawText(font, {X_EFFECT_LIST, pTextHeight}, colorTable.White, localization->GetString(LSTR_UNKNOWN_VALUE));
        pTextHeight += lineAdvance;
    } else {
        bool hasBuffs = false;
        for (ActorBuff buff : pActors[uActorID].buffs.indices()) {
            if (pActors[uActorID].buffs[buff].Active()) {
                hasBuffs = true;
                std::string text = localization->GetActorBuffName(buff);
                if (!text.empty()) {
                    if (pWindow)
                        pWindow->DrawText(font, {X_EFFECT_LIST, pTextHeight}, GetSpellColor(spellForActorBuff(buff)), text);
                    pTextHeight += lineAdvance;
                }
            }
        }
        if (!hasBuffs) {
            if (pWindow)
                pWindow->DrawText(font, {X_EFFECT_LIST, pTextHeight}, colorTable.White, localization->GetString(LSTR_NONE));
            pTextHeight += lineAdvance;
        }
    }

    /*------------------------------- Right side -------------------------------*/
    int leftTextHeight = pTextHeight;
    pTextHeight = Y_POS_DOLL;
    int maxWidth = 0; // The max width of the info at X_RIGHT_DATA
    auto measureWidth = [font, &maxWidth](std::string_view s) {
        int w = font->GetLineWidth(s);
        if (w > maxWidth) maxWidth = w;
    };

    std::string hpStr, acStr;
    if (normal_level) {
        hpStr = fmt::format("{}", monsterInfo.hp);
        acStr = fmt::format("{}", monsterInfo.ac);
    } else {
        hpStr = acStr = localization->GetString(LSTR_UNKNOWN_VALUE);
    }
    if (showCurrentHp && extended)
        hpStr = fmt::format("{} / {}", pActors[uActorID].currentHP, hpStr);
    if (pWindow) {
        pWindow->DrawText(font, {X_RIGHT_COLUMN, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_HIT_POINTS));
        pWindow->DrawText(font, {X_RIGHT_DATA, pTextHeight}, colorTable.White, hpStr);
    } else {
        measureWidth(hpStr);
    }
    pTextHeight += lineAdvance;
    if (pWindow) {
        pWindow->DrawText(font, {X_RIGHT_COLUMN, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_ARMOR_CLASS));
        pWindow->DrawText(font, {X_RIGHT_DATA, pTextHeight}, colorTable.White, acStr);
    } else {
        measureWidth(acStr);
    }
    pTextHeight += 2 * lineAdvance;


    // TODO(captainurist): Display ranged attack as "Ranged attack". This currently doesn't fit in the table, we used to
    //                     just do attackStr + " R" but that's cryptic. Redo properly with dynamic alignment.

    std::string attackStr, damageStr;
    if (expert_level) {
        attackStr = displayNameForDamageType(monsterInfo.attack1Type, localization);
        if (monsterInfo.attack1DamageBonus) {
            damageStr = fmt::format("{}d{}+{}", monsterInfo.attack1DamageDiceRolls, monsterInfo.attack1DamageDiceSides, monsterInfo.attack1DamageBonus);
        } else {
            damageStr = fmt::format("{}d{}", monsterInfo.attack1DamageDiceRolls, monsterInfo.attack1DamageDiceSides);
        }
    } else {
        attackStr = damageStr = localization->GetString(LSTR_UNKNOWN_VALUE);
    }
    if (pWindow) {
        pWindow->DrawText(font, {X_RIGHT_COLUMN, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_ATTACK));
        pWindow->DrawText(font, {X_RIGHT_DATA, pTextHeight}, colorTable.White, attackStr);
    } else {
        measureWidth(attackStr);
    }
    pTextHeight += lineAdvance;
    if (pWindow) {
        pWindow->DrawText(font, {X_RIGHT_COLUMN, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_DAMAGE));
        pWindow->DrawText(font, {X_RIGHT_DATA, pTextHeight}, colorTable.White, damageStr);
    } else {
        measureWidth(damageStr);
    }
    if (expert_level && extended && monsterInfo.attack2Chance > 0 && (monsterInfo.attack2DamageDiceRolls > 0 || monsterInfo.attack2DamageBonus > 0)) {
        attackStr = displayNameForDamageType(monsterInfo.attack2Type, localization);
        if (monsterInfo.attack2DamageBonus) {
            damageStr = fmt::format("{}d{}+{}", monsterInfo.attack2DamageDiceRolls, monsterInfo.attack2DamageDiceSides, monsterInfo.attack2DamageBonus);
        } else {
            damageStr = fmt::format("{}d{}", monsterInfo.attack2DamageDiceRolls, monsterInfo.attack2DamageDiceSides);
        }
        pTextHeight += lineAdvance;
        if (pWindow) {
            pWindow->DrawText(font, {X_RIGHT_COLUMN, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_ATTACK));
            pWindow->DrawText(font, {X_RIGHT_DATA, pTextHeight}, colorTable.White, attackStr);
        } else {
            measureWidth(attackStr);
        }
        pTextHeight += lineAdvance;
        if (pWindow) {
            pWindow->DrawText(font, {X_RIGHT_COLUMN, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_DAMAGE));
            pWindow->DrawText(font, {X_RIGHT_DATA, pTextHeight}, colorTable.White, damageStr);
        } else {
            measureWidth(damageStr);
        }
    }
    if (master_level && extended && monsterInfo.specialAttackType != SPECIAL_ATTACK_NONE) {
        pTextHeight += lineAdvance;
        if (pWindow) {
            pWindow->DrawText(font, {X_RIGHT_INDENTED, pTextHeight},
                              monsterSpecialAttackColors[monsterInfo.specialAttackType],
                              localization->GetSpecialAttackName(monsterInfo.specialAttackType));
        }
    }
    if (master_level && extended && monsterInfo.specialAbilityType != MONSTER_SPECIAL_ABILITY_NONE) {
        pTextHeight += lineAdvance;
        if (pWindow) {
            pWindow->DrawText(font, {X_RIGHT_INDENTED, pTextHeight},
                              monsterSpecialAbilityColors[monsterInfo.specialAbilityType],
                              localization->GetMonsterSpecialAbilityName(monsterInfo.specialAbilityType));
        }
    }
    pTextHeight += 2 * lineAdvance;

    std::string spellTitleStr = localization->GetString(LSTR_SPELL);
    std::string spell1Str, spell2Str, spell3Str; // 1 is in-line with spellTitleStr (only non-extended), 2 and 3 own lines
    int spellX = extended ? X_RIGHT_INDENTED : X_RIGHT_DEFAULT_SPELLS;
    if (master_level) {
        if (monsterInfo.spell1Id == SPELL_NONE && monsterInfo.spell2Id == SPELL_NONE) {
            spell1Str = localization->GetString(LSTR_NONE);
            spellX = X_RIGHT_DATA;
        }
        if (monsterInfo.spell1Id != SPELL_NONE && monsterInfo.spell2Id != SPELL_NONE) {
            spellTitleStr = localization->GetString(LSTR_SPELLS);
        }
        if (monsterInfo.spell1Id != SPELL_NONE) {
            if (extended)
                spell2Str = fmt::format("{} {}", pSpellStats->pInfos[monsterInfo.spell1Id].pShortName, localization->SkillValueShortString(monsterInfo.spell1SkillMastery));
            else
                spell1Str = pSpellStats->pInfos[monsterInfo.spell1Id].pShortName;
        }
        if (monsterInfo.spell2Id != SPELL_NONE) {
            if (extended)
                spell3Str = fmt::format("{} {}", pSpellStats->pInfos[monsterInfo.spell2Id].pShortName, localization->SkillValueShortString(monsterInfo.spell2SkillMastery));
            else
                spell2Str = pSpellStats->pInfos[monsterInfo.spell2Id].pShortName;
        }
    } else {
        spell1Str = localization->GetString(LSTR_UNKNOWN_VALUE);
        spellX = X_RIGHT_DATA;
    }
    if (pWindow) {
        pWindow->DrawText(font, {X_RIGHT_COLUMN, pTextHeight}, colorTable.Jonquil, spellTitleStr);
        if (!spell1Str.empty())
            pWindow->DrawText(font, {spellX, pTextHeight}, colorTable.White, spell1Str);
    }
    if (!spell2Str.empty()) {
        pTextHeight += lineAdvance;
        if (pWindow) pWindow->DrawText(font, {spellX, pTextHeight}, colorTable.White, spell2Str);
    }
    if (!spell3Str.empty()) {
        pTextHeight += lineAdvance;
        if (pWindow) pWindow->DrawText(font, {spellX, pTextHeight}, colorTable.White, spell3Str);
    }
    pTextHeight += 2 * lineAdvance;

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
        monsterInfo.resFire,
        monsterInfo.resAir,
        monsterInfo.resWater,
        monsterInfo.resEarth,
        monsterInfo.resSpirit,
        monsterInfo.resMind,
        monsterInfo.resBody,
        monsterInfo.resLight,
        monsterInfo.resDark,
        monsterInfo.resPhysical, // Physical & Dark were switched, was a bug?
    };

    if (pWindow) pWindow->DrawText(font, {X_RIGHT_COLUMN, pTextHeight}, colorTable.Jonquil, localization->GetString(LSTR_RESISTANCES));
    pTextHeight += lineAdvance;

    std::string resStr;
    for (int i = 0; i < 10; i++) {
        if (!grandmaster_level) {
            resStr = localization->GetString(LSTR_UNKNOWN_VALUE);
        } else if (resValues[i] == 200) {
            resStr = localization->GetString(LSTR_IMMUNE);
        } else {
            if (resValues[i]) {
                if (extended)
                    resStr = fmt::format("{}", resValues[i]);
                else
                    resStr = localization->GetString(LSTR_RESISTANT);
            } else {
                resStr = localization->GetString(LSTR_NONE);
            }
        }

        if (pWindow) {
            pWindow->DrawText(font, {X_RIGHT_INDENTED, pTextHeight}, colorTable.Jonquil, resTypes[i]);
            pWindow->DrawText(font, {X_RIGHT_DATA, pTextHeight}, colorTable.White, resStr);
        } else {
            measureWidth(resStr);
        }
        pTextHeight += lineAdvance;
    }

    /*------------------------------- Bottom -------------------------------*/
    if (leftTextHeight > pTextHeight) pTextHeight = leftTextHeight;
    pTextHeight += lineAdvance;

    // cast spell: Detect life
    if (showCurrentHp && !extended) {
        if (pWindow) {
            std::string str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_HIT_POINTS), pActors[uActorID].currentHP);
            pWindow->DrawTitleText(font, 0, pTextHeight, colorTable.White, str, 3);
        }
        pTextHeight += lineAdvance;
    }

    // Debug - show actor AI state with full information
    if (monster_full_informations) {
        if (pWindow) {
            std::string str = fmt::format("ActorId: {}   AI State: {}", uActorID, std::to_underlying(pActors[uActorID].aiState));
            pWindow->DrawTitleText(font, 0, pTextHeight, colorTable.White, str, 3);
        }
        pTextHeight += lineAdvance;
    }

    return std::make_pair(X_RIGHT_DATA + maxWidth + RIGHT_BOTTOM_MARGIN, pTextHeight + RIGHT_BOTTOM_MARGIN);
}

/**
  * @offset 0x00417BB5
  *
  * @brief Generating message for skill description popup.
  *
  * @param uPlayerID                     Character identifier.
  * @param uPlayerSkillType              Skill type identifier.
  */
std::string CharacterUI_GetSkillDescText(int uPlayerID, Skill uPlayerSkillType) {
    size_t line_width = std::max({
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_NORMAL)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_EXPERT)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_MASTER)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND_1)),
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_BONUS_2))
    });

    int base_skill = pParty->pCharacters[uPlayerID].getSkillValue(uPlayerSkillType).level();
    int actual_skill = pParty->pCharacters[uPlayerID].getActualSkillValue(uPlayerSkillType).level();

    std::string Description = localization->GetSkillDescription(uPlayerSkillType);
    if (!localization->GetSkillDescriptionNormal(uPlayerSkillType).empty()) {
        Description = fmt::format("{}\n\n", Description);

        for (Mastery mastery : allSkillMasteries()) {
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
    Pointi mousePos = mouse->position();
    int pX = mousePos.x;
    int pY = mousePos.y;

    if (pX < 24 || pX > 455 || pY < 18 || pY > 36) {
        for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
            if (pButton->msg == UIMSG_SkillUp && pX >= pButton->uX &&
                pX < pButton->uZ && pY >= pButton->uY && pY < pButton->uW) {
                Skill skill = static_cast<Skill>(pButton->msg_param);
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

    Pointi pt = mouse->position();
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
                localization->GetAttirubteName(static_cast<Attribute>(pStringNum)),
                localization->GetAttributeDescription(static_cast<Attribute>(pStringNum)));
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
                if (pParty->activeCharacter().conditions.has(condition)) {
                    str += " \n";
                    Duration condition_time = pParty->GetPlayingTime() - pParty->activeCharacter().conditions.get(condition);
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
                str = localization->FormatString(LSTR_YOU_ARE_ELIGIBLE_TO_TRAIN_TO_U, eligbleLevel);
            }
            int nextLevelExp = GetExperienceRequiredForLevel(eligbleLevel) - pParty->activeCharacter().experience;
            str += "\n" + localization->FormatString(LSTR_YOU_NEED_D_MORE_EXPERIENCE_TO_TRAIN_TO, nextLevelExp, eligbleLevel + 1);
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
            bool hasBow = !!pParty->activeCharacter().inventory.entry(ITEM_SLOT_BOW);
            InventoryConstEntry weapon = pParty->activeCharacter().inventory.entry(ITEM_SLOT_MAIN_HAND);
            bool hasBlaster = weapon && weapon->skill() == SKILL_BLASTER;
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

    Pointi pt = mouse->position();

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
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND_1))
    });

    std::string str = fmt::format(
        "{}\n\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}",
        spell->pDescription,
        localization->GetString(LSTR_NORMAL), v5 + 3, v5 + 10, spell->pBasicSkillDesc,
        localization->GetString(LSTR_EXPERT), v5 + 3, v5 + 10, spell->pExpertSkillDesc,
        localization->GetString(LSTR_MASTER), v5 + 3, v5 + 10, spell->pMasterSkillDesc,
        localization->GetString(LSTR_GRAND_1), v5 + 3, v5 + 10, spell->pGrandmasterSkillDesc
    );
    spell_info_window.uFrameHeight += assets->pFontSmallnum->CalcTextHeight(
        str, spell_info_window.uFrameWidth, 0
    );
    if ((signed int)spell_info_window.uFrameHeight < 150)
        spell_info_window.uFrameHeight = 150;
    spell_info_window.uFrameWidth = pViewport->viewportWidth;
    spell_info_window.DrawMessageBox(false);
    spell_info_window.uFrameWidth -= 12;
    spell_info_window.uFrameHeight -= 12;
    spell_info_window.uFrameZ = spell_info_window.uFrameX + spell_info_window.uFrameWidth - 1;
    spell_info_window.uFrameW = spell_info_window.uFrameHeight + spell_info_window.uFrameY - 1;
    spell_info_window.DrawTitleText(
        assets->pFontArrus.get(), 0x78u, 0xCu, colorTable.PaleCanary, spell->name, 3);
    spell_info_window.DrawText(assets->pFontSmallnum.get(), {120, 44}, colorTable.White, str);
    spell_info_window.uFrameWidth = 108;
    spell_info_window.uFrameZ = spell_info_window.uFrameX + 107;
    Skill skill = skillForMagicSchool(pParty->activeCharacter().lastOpenedSpellbookPage);
    Mastery skill_mastery = pParty->activeCharacter().getSkillValue(skill).mastery();
    spell_info_window.DrawTitleText(assets->pFontComic.get(), 12, 75, colorTable.White, localization->GetSkillName(skill), 3);

    if (skill_mastery == Mastery::MASTERY_NONE) {
        if (engine->config->debug.AllMagic.value()) {
            skill_mastery = Mastery::MASTERY_GRANDMASTER;
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
            if (i == PARTY_BUFF_PROTECTION_FROM_MAGIC && engine->config->gameplay.ShowProtectionMagicPower.value()) {
                yPos = stringCount * assets->pFontComic->GetHeight() + 40;
                popupWindow.DrawText(assets->pFontComic.get(), {32, yPos}, colorTable.White, "\r020" + localization->FormatString(LSTR_REMAINING_POWER, pParty->pPartyBuffs[i].power));
                stringCount++;
            }
        }
    }
}

/**
 * @offset 0x4B1523
 */
void showSpellbookInfo(ItemId spellbook) {
    assert(isSpellbook(spellbook));
    SpellId spell = spellForSpellbook(spellbook);

    Pointi cursorPos = EngineIocContainer::ResolveMouse()->position();
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
        assets->pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND_1))});

    std::string str = fmt::format("{}\n\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}",
                                  pSpellStats->pInfos[spell].pDescription,
                                  localization->GetString(LSTR_NORMAL), maxLineWidth + 3, maxLineWidth + 10, pSpellStats->pInfos[spell].pBasicSkillDesc,
                                  localization->GetString(LSTR_EXPERT), maxLineWidth + 3, maxLineWidth + 10, pSpellStats->pInfos[spell].pExpertSkillDesc,
                                  localization->GetString(LSTR_MASTER), maxLineWidth + 3, maxLineWidth + 10, pSpellStats->pInfos[spell].pMasterSkillDesc,
                                  localization->GetString(LSTR_GRAND_1), maxLineWidth + 3, maxLineWidth + 10, pSpellStats->pInfos[spell].pGrandmasterSkillDesc);

    popup.uFrameHeight += assets->pFontSmallnum->CalcTextHeight(str, popup.uFrameWidth, 0);
    if (popup.uFrameHeight < 150) {
        popup.uFrameHeight = 150;
    }
    popup.uFrameWidth = pViewport->viewportWidth;
    popup.DrawMessageBox(false);
    popup.uFrameWidth -= 12;
    popup.uFrameHeight -= 12;
    popup.uFrameZ = popup.uFrameX + popup.uFrameWidth - 1;
    popup.uFrameW = popup.uFrameHeight + popup.uFrameY - 1;
    popup.DrawTitleText(assets->pFontArrus.get(), 0x78u, 0xCu, colorTable.PaleCanary, pSpellStats->pInfos[spell].name, 3u);
    popup.DrawText(assets->pFontSmallnum.get(), {120, 44}, colorTable.White, str);
    popup.uFrameZ = popup.uFrameX + 107;
    popup.uFrameWidth = 108;
    popup.DrawTitleText(assets->pFontComic.get(), 0xCu, 0x4Bu, colorTable.White, localization->GetSkillName(skillForSpell(spell)), 3u);

    str = fmt::format("{}\n{}", localization->GetString(LSTR_SP_COST), pSpellDatas[spell].mana_per_skill[MASTERY_NOVICE]);
    popup.DrawTitleText(assets->pFontComic.get(), 0xCu, popup.uFrameHeight - assets->pFontComic->GetHeight() - 16, colorTable.White, str, 3);
}

//----- new function
void ShowPopupShopSkills() {
    Pointi mousePos = mouse->position();
    int pX = mousePos.x;
    int pY = mousePos.y;

    if (pDialogueWindow && pDialogueWindow->pNumPresenceButton != 0) {
        for (GUIButton *pButton : pDialogueWindow->vButtons) {
            if (pX >= pButton->uX && pX < pButton->uZ && pY >= pButton->uY && pY < pButton->uW) {
                if (IsSkillLearningDialogue((DialogueId)pButton->msg_param)) {
                    auto skill_id = GetLearningDialogueSkill((DialogueId)pButton->msg_param);
                    if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill_id] != MASTERY_NONE &&
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
    Item *item;  // ecx@13
    int invindex;
    int testpos;
    HouseType houseType = window_SpeakInHouse->buildingType();
    DialogueId dialogue = window_SpeakInHouse->getCurrentDialogue();

    if (houseType == HOUSE_TYPE_INVALID)
        return;

    if (dialogue < DIALOGUE_SHOP_BUY_STANDARD)
        return;

    Pointi pt = EngineIocContainer::ResolveMouse()->position();
    int testx;

    if (houseType <= HOUSE_TYPE_ALCHEMY_SHOP) {
        if (dialogue == DIALOGUE_SHOP_BUY_STANDARD || dialogue == DIALOGUE_SHOP_BUY_SPECIAL) {
            switch (houseType) {
                case HOUSE_TYPE_WEAPON_SHOP: {
                    testx = (pt.x - 30) / 70;
                    if (testx >= 0 && testx < 6) {
                        if (dialogue == DIALOGUE_SHOP_BUY_STANDARD)
                            item = &pParty->standartItemsInShops[window_SpeakInHouse->houseId()][testx];
                        else
                            item = &pParty->specialItemsInShops[window_SpeakInHouse->houseId()][testx];

                        if (item->itemId != ITEM_NULL) {
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

                case HOUSE_TYPE_ARMOR_SHOP:
                    testx = (pt.x - 40) / 105;
                    if (testx >= 0 && testx < 4) {
                        if (pt.y >= 126) {
                            testx += 4;
                        }

                        if (dialogue == DIALOGUE_SHOP_BUY_STANDARD)
                            item = &pParty->standartItemsInShops[window_SpeakInHouse->houseId()][testx];
                        else
                            item = &pParty->specialItemsInShops[window_SpeakInHouse->houseId()][testx];

                        if (item->itemId != ITEM_NULL) {
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

                case HOUSE_TYPE_ALCHEMY_SHOP:
                case HOUSE_TYPE_MAGIC_SHOP:
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

                        if (item->itemId != ITEM_NULL) {
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
            Pointi pos = mapToInventoryGrid(pt, Pointi(14, 17));
            if (pt.x <= 13 || pt.x >= 462 || !pParty->activeCharacter().inventory.entry(pos))
                return;

            GameUI_DrawItemInfo(pParty->activeCharacter().inventory.entry(pos).get());
            return;
        }
    }

    if (houseType <= HOUSE_TYPE_MIRRORED_PATH_GUILD && dialogue == DIALOGUE_GUILD_BUY_BOOKS) {
        int testx = (pt.x - 32) / 70;
        if (testx >= 0 && testx < 6) {
            if (pt.y >= 250) {
                testx += 6;
            }

            item = &pParty->spellBooksInGuilds[window_SpeakInHouse->houseId()][testx];

            if (item->itemId != ITEM_NULL) {
                int testpos;
                if (pt.y >= 250) {
                    testpos = 32 + 70 * testx - 420;
                } else {
                    testpos = 32 + 70 * testx;
                }

                if (pt.x >= testpos && pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                    if ((pt.y >= 90 && pt.y <= (90 + (shop_ui_items_in_store[testx]->height()))) || (pt.y >= 250 && pt.y <= (250 + (shop_ui_items_in_store[testx]->height())))) {
                        showSpellbookInfo(pParty->spellBooksInGuilds[window_SpeakInHouse->houseId()][testx].itemId);
                    }
                }
            }
        }
    }
}

//----- (0041D3B7) --------------------------------------------------------
void GameUI_CharacterQuickRecord_Draw(GUIWindow *window, int characterIndex) {
    GraphicsImage *v13;              // eax@6
    std::string spellName;   // eax@16
    int v36;                 // esi@22
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
        int faceTextureIndex = 1;
        if (player->portrait == PORTRAIT_TALK)
            faceTextureIndex = player->talkAnimation.currentFrameIndex();
        else
            faceTextureIndex = pPortraitFrameTable->animationFrameIndex(pPortraitFrameTable->animationId(player->portrait),
                                                                        pMiscTimer->time());
        player->portraitImageIndex = faceTextureIndex - 1;
        v13 = game_ui_player_faces[characterIndex][faceTextureIndex - 1];
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
        LSTR_ACTIVE_SPELLS_S,
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
void UI_OnMouseRightClick(Pointi mousePos) {
    std::string pStr;        // edi@85
    GUIWindow popup_window;  // [sp+4h] [bp-74h]@32

    if (current_screen_type == SCREEN_VIDEO || GetCurrentMenuID() == MENU_MAIN)
        return;


    unsigned int pX = mousePos.x;
    unsigned int pY = mousePos.y;

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

    if (pParty->pPickedItem.itemId != ITEM_NULL) {
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
                    LSTR_S_IS_IN_NO_CONDITION_TO_S,
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

                int inventoryYCoord = (pY - 34) / 32;  // TODO(captainurist): use pchestoffsets
                int inventoryXCoord = (pX - 42) / 32;

                InventoryEntry entry = vChests[pGUIWindow_CurrentChest->chestId()].inventory.entry({inventoryXCoord, inventoryYCoord});
                if (entry)
                    GameUI_DrawItemInfo(entry.get());
            }
            break;
        }

        case SCREEN_GAME:  // In the main menu displays a pop-up window(В
                           // главном меню показывает всплывающее окно)
        {
            if (GetCurrentMenuID() > MENU_MAIN) break;
            mouse->DoMouseLook();

            if ((signed int)pY > (signed int)pViewport->viewportBR_Y) {
                int characterIndex = pX / 118;
                if (characterIndex < 4) { // portaits zone
                    popup_window.sHint.clear();
                    popup_window.uFrameWidth = 400;
                    popup_window.uFrameHeight = 200;
                    popup_window.uFrameX = 38;
                    popup_window.uFrameY = 60;
                    GameUI_CharacterQuickRecord_Draw(&popup_window, characterIndex);
                }
            } else if ((int)pX > pViewport->viewportBR_X) {
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

                Pid pointedObject = engine->PickMouseInfoPopup().pid;
                /*else
                pointedObject = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]];*/
                if (pointedObject.type() == OBJECT_Actor) {
                    render->BeginScene2D();
                    auto [w, h] = MonsterPopup_Draw(pointedObject.id(), nullptr);
                    popup_window.uFrameWidth = w;
                    popup_window.uFrameHeight = h;
                    popup_window.DrawMessageBox(true);
                    MonsterPopup_Draw(pointedObject.id(), &popup_window);
                }
                if (pointedObject.type() == OBJECT_Sprite) {
                    if (!(pObjectList->pObjects[pSpriteObjects[pointedObject.id()].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE)) {
                        GameUI_DrawItemInfo(&pSpriteObjects[pointedObject.id()].containing_item);
                    }
                }
            }
            break;
        }
        case SCREEN_BOOKS: {
            if (pGUIWindow_CurrentMenu->eWindowType != WINDOW_MapsBook ||
                (signed int)pX < (signed int)pViewport->viewportTL_X ||
                (signed int)pX > (signed int)pViewport->viewportBR_X ||
                (signed int)pY < (signed int)pViewport->viewportTL_Y ||
                (signed int)pY > (signed int)pViewport->viewportBR_Y ||
                ((popup_window.sHint = GetMapBookHintText(mousePos.x, mousePos.y)).empty())) {
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
                            popup_window.sHint = localization->GetAttributeDescription(static_cast<Attribute>(pButton->msg_param % 7));
                            pStr = localization->GetAttirubteName(static_cast<Attribute>(pButton->msg_param % 7));
                            break;
                        case UIMSG_PlayerCreationClickPlus:  // Plus button info
                            pStr = localization->GetString(LSTR_ADD);
                            popup_window.sHint = localization->GetString(LSTR_ADDS_A_POINT_FROM_THE_HIGHLIGHTED_SKILL);
                            break;
                        case UIMSG_PlayerCreationClickMinus:  // Minus button
                                                              // info
                            pStr = localization->GetString(LSTR_SUBTRACT);
                            popup_window.sHint = localization->GetString(LSTR_SUBTRACTS_A_POINT_FROM_THE_HIGHLIGHTED);
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
                            popup_window.sHint = localization->GetClassDescription(static_cast<Class>(pButton->msg_param));
                            pStr = localization->GetClassName(static_cast<Class>(pButton->msg_param));
                            break;
                        case UIMSG_PlayerCreationClickOK:  // OK Info
                            popup_window.sHint = localization->GetString(
                                LSTR_CLICK_HERE_TO_ACCEPT_THIS_PARTY_AND);
                            pStr = localization->GetString(LSTR_OK_BUTTON);
                            break;
                        case UIMSG_PlayerCreationClickReset:  // Clear info
                            popup_window.sHint = localization->GetString(
                                LSTR_CLEARS_ALL_PARTY_STATS_AND_SKILLS);
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
                        if (pParty->pCharacters[pButton->msg_param].GetSkillIdxByOrder(pButton->msg - UIMSG_48) != SKILL_INVALID) {
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

    InventoryEntry entry;

    static const std::array<int, 6> ringsX = {0x1EA, 0x21A, 0x248, 0x1EA, 0x21A, 0x248};
    static const std::array<int, 6> ringsY = {0x0CA, 0x0CA, 0x0CA, 0x0FA, 0x0FA, 0x0FA};

    Pointi mousePos = mouse->position();
    int pX = mousePos.x;
    int pY = mousePos.y;

    Pointi gridPos = mapToInventoryGrid(mousePos, Pointi(14, 17));

    if (pX <= 13 || pX >= 462) {   // items on the player ragdoll
        static const int glovex = 586;
        static const int glovey = 88;

        static const int amuletx = 493;
        static const int amulety = 91;

        static const int slotSize = 32;

        if (!ringscreenactive()) { // rings not displayed
            int item_pid = render->QueryHitMap({pX, pY}, -1);
            entry = pParty->activeCharacter().inventory.entry(item_pid);
        } else {  // rings displayed
            ItemSlot pos = ITEM_SLOT_INVALID;

            if (mouse->position().x < 490 || mouse->position().x > 618) {
                return;
            }

            if (mouse->position().y < 88 || mouse->position().y > 282) {
                return;
            }

            if (mouse->position().x >= amuletx && mouse->position().x <= (amuletx + slotSize) &&
                mouse->position().y >= amulety && mouse->position().y <= (amulety + 2 * slotSize)) {
                pos = ITEM_SLOT_AMULET;
            }

            if (mouse->position().x >= glovex && mouse->position().x <= (glovex + slotSize) &&
                mouse->position().y >= glovey && mouse->position().y <= (glovey + 2 * slotSize)) {
                pos = ITEM_SLOT_GAUNTLETS;
            }

            for (int i = 0; i < 6; ++i) {
                if (mouse->position().x >= ringsX[i] && mouse->position().x <= (ringsX[i] + slotSize) &&
                    mouse->position().y >= ringsY[i] && mouse->position().y <= (ringsY[i] + slotSize)) {
                    pos = ringSlot(i);
                }
            }

            if (pos != ITEM_SLOT_INVALID) {
                entry = pParty->activeCharacter().inventory.entry(pos);
            }
        }
    } else {
        entry = pParty->activeCharacter().inventory.entry(gridPos);
    }

    if (!entry) {  // no item
        return;
    }

    // check character condition
    if (!pParty->activeCharacter().CanAct()) {
        static std::string hint_reference;
        hint_reference = localization->FormatString(LSTR_S_IS_IN_NO_CONDITION_TO_S, pParty->activeCharacter().name,
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

    CombinedSkillValue alchemySkill = pParty->activeCharacter().getActualSkillValue(SKILL_ALCHEMY);

    if (pParty->pPickedItem.itemId == ITEM_POTION_BOTTLE) {
        GameUI_DrawItemInfo(entry.get());
        return;
    }

    if (isPotion(pParty->pPickedItem.itemId) && isPotion(entry->itemId)) { // potion mixing
        ItemId potionSrc1 = entry->itemId;
        ItemId potionSrc2 = pParty->pPickedItem.itemId;

        ItemId potionID;
        if (pParty->pPickedItem.itemId == ITEM_POTION_CATALYST || entry->itemId == ITEM_POTION_CATALYST) {
            potionID = ITEM_POTION_CATALYST;
        } else {
            potionID = pItemTable->potionCombination[potionSrc2][potionSrc1];
        }

        if (potionID == ITEM_NULL) {
            // Combining same potions
            GameUI_DrawItemInfo(entry.get());
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
                    alchemySkill.mastery() == MASTERY_NONE) {
                damage_level = 1;
            }
            if (potionID >= ITEM_POTION_HASTE &&
                    potionID <= ITEM_POTION_CURE_INSANITY &&
                    alchemySkill.mastery() <= MASTERY_NOVICE) {
                damage_level = 2;
            }
            if (potionID >= ITEM_POTION_MIGHT_BOOST &&
                    potionID <= ITEM_POTION_BODY_RESISTANCE &&
                    alchemySkill.mastery() <= MASTERY_EXPERT) {
                damage_level = 3;
            }
            if (potionID >= ITEM_POTION_STONE_TO_FLESH &&
                    alchemySkill.mastery() <= MASTERY_MASTER) {
                damage_level = 4;
            }
        }

        if (damage_level > 0) {
            pParty->activeCharacter().inventory.take(entry);

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
            engine->_statusBar->setEvent(LSTR_OOOPS);
            pParty->takeHoldingItem();
            rightClickItemActionPerformed = true;
            return;
        } else {  // if ( damage_level == 0 )
            if (entry->itemId == ITEM_POTION_CATALYST && pParty->pPickedItem.itemId == ITEM_POTION_CATALYST) {
                // Both potions are catalyst: power is maximum of two
                entry->potionPower = std::max(entry->potionPower, pParty->pPickedItem.potionPower);
            } else if (entry->itemId == ITEM_POTION_CATALYST || pParty->pPickedItem.itemId == ITEM_POTION_CATALYST) {
                // One of the potion is catalyst: power of potion is replaced by power of catalyst
                if (entry->itemId == ITEM_POTION_CATALYST) {
                    entry->itemId = pParty->pPickedItem.itemId;
                } else {
                    entry->potionPower = pParty->pPickedItem.potionPower;
                }
            } else {
                entry->itemId = potionID;
                entry->potionPower = (pParty->pPickedItem.potionPower + entry->potionPower) / 2;
                // Can be zero even for valid potion combination when resulting potion is of lower grade than it's components
                // Example: "Cure Paralysis(white) + Cure Wounds(red) = Cure Wounds(red)"
                if (pItemTable->potionNotes[potionSrc1][potionSrc2] != 0) {
                    pParty->activeCharacter().SetVariable(VAR_AutoNotes, pItemTable->potionNotes[potionSrc1][potionSrc2]);
                }
            }
            if (!(pItemTable->items[entry->itemId].identifyAndRepairDifficulty)) {
                entry->flags |= ITEM_IDENTIFIED;
            }
            pParty->activeCharacter().playReaction(SPEECH_POTION_SUCCESS);
            pParty->takeHoldingItem();
            rightClickItemActionPerformed = true;
            InventoryEntry bottle = pParty->activeCharacter().inventory.tryAdd(Item(ITEM_POTION_BOTTLE));
            if (bottle) {
                bottle->flags = ITEM_IDENTIFIED;
            } else {
                // Can't fit bottle in inventory - place it in hand
                Item bottle;
                bottle.itemId = ITEM_POTION_BOTTLE;
                bottle.flags = ITEM_IDENTIFIED;
                pParty->setHoldingItem(bottle);
            }
            return;
        }
    }

    if (pParty->pPickedItem.itemId == ITEM_POTION_RECHARGE_ITEM) {
        if (entry->isWand()) { // can recharge only wands
            int maxChargesDecreasePercent = 70 - pParty->pPickedItem.potionPower;
            if (maxChargesDecreasePercent < 0) {
                maxChargesDecreasePercent = 0;
            }

            float invMaxChargesDecrease = (100 - maxChargesDecreasePercent) * 0.01;
            int newCharges = entry->maxCharges * invMaxChargesDecrease;

            // Disallow if wand will lose charges
            if (newCharges <= entry->numCharges) {
                engine->_statusBar->setEvent(LSTR_WAND_ALREADY_CHARGED);
                pAudioPlayer->playUISound(SOUND_spellfail0201);
            } else {
                entry->maxCharges = entry->numCharges = newCharges;
                // Effect and sound was not present previously
                entry->flags |= ITEM_AURA_EFFECT_GREEN;
                ItemEnchantmentTimer = Duration::fromRealtimeSeconds(2);
                pAudioPlayer->playSpellSound(SPELL_WATER_RECHARGE_ITEM, false, SOUND_MODE_UI);
            }

            pParty->takeHoldingItem();
            rightClickItemActionPerformed = true;
            return;
        }
        GameUI_DrawItemInfo(entry.get());
        return;
    }

    if (pParty->pPickedItem.itemId == ITEM_POTION_HARDEN_ITEM) {
        if (entry->IsBroken() || isArtifact(entry->itemId)) {
            // Sound error and stop right click item actions until button is released
            pAudioPlayer->playUISound(SOUND_error);
            rightClickItemActionPerformed = true;
            return;
        }
        if (entry->isWeapon() || entry->isPassiveEquipment() || entry->isWand()) {
            entry->flags |= ITEM_AURA_EFFECT_RED | ITEM_HARDENED;

            // Sound was missing previously
            pAudioPlayer->playSpellSound(SPELL_WATER_ENCHANT_ITEM, false, SOUND_MODE_UI);

            ItemEnchantmentTimer = Duration::fromRealtimeSeconds(2);
            pParty->takeHoldingItem();
            rightClickItemActionPerformed = true;
            return;
        }
        GameUI_DrawItemInfo(entry.get());
        return;
    }

    if (isEnchantingPotion(pParty->pPickedItem.itemId)) {
        if (entry->IsBroken() || isArtifact(entry->itemId) || entry->isPassiveEquipment() || entry->isWand() || isAncientWeapon(entry->itemId)) {
            // Sound error and stop right click item actions until button is released
            pAudioPlayer->playUISound(SOUND_error);
            rightClickItemActionPerformed = true;
            return;
        }
        if (entry->isWeapon()) {
            if (entry->specialEnchantment != ITEM_ENCHANTMENT_NULL || entry->standardEnchantment) {
                // Sound error and stop right click item actions until button is released
                pAudioPlayer->playUISound(SOUND_error);
                rightClickItemActionPerformed = true;
                return;
            }

            Duration effectTime = Duration::fromMinutes(30 * pParty->pPickedItem.potionPower);
            entry->UpdateTempBonus(pParty->GetPlayingTime());
            entry->specialEnchantment = potionEnchantment(pParty->pPickedItem.itemId);
            entry->enchantmentExpirationTime = pParty->GetPlayingTime() + effectTime;
            // Sound was missing previously
            entry->flags |= ITEM_TEMP_BONUS | ITEM_AURA_EFFECT_RED;
            pAudioPlayer->playSpellSound(SPELL_WATER_ENCHANT_ITEM, false, SOUND_MODE_UI);

            ItemEnchantmentTimer = Duration::fromRealtimeSeconds(2);
            pParty->takeHoldingItem();
            rightClickItemActionPerformed = true;
            return;
        }
        GameUI_DrawItemInfo(entry.get());
        return;
    }

    if (isReagent(pParty->pPickedItem.itemId) && entry->itemId == ITEM_POTION_BOTTLE) {
        entry->potionPower = alchemySkill.level() + pParty->pPickedItem.GetReagentPower();
        switch (pParty->pPickedItem.itemId) {
            case ITEM_REAGENT_WIDOWSWEEP_BERRIES:
            case ITEM_REAGENT_CRUSHED_ROSE_PETALS:
            case ITEM_REAGENT_VIAL_OF_TROLL_BLOOD:
            case ITEM_REAGENT_RUBY:
            case ITEM_REAGENT_DRAGONS_EYE:
                entry->itemId = ITEM_POTION_CURE_WOUNDS;
                break;

            case ITEM_REAGENT_PHIRNA_ROOT:
            case ITEM_REAGENT_METEORITE_FRAGMENT:
            case ITEM_REAGENT_HARPY_FEATHER:
            case ITEM_REAGENT_MOONSTONE:
            case ITEM_REAGENT_ELVISH_TOADSTOOL:
                entry->itemId = ITEM_POTION_MAGIC;
                break;

            case ITEM_REAGENT_POPPYSNAPS:
            case ITEM_REAGENT_FAE_DUST:
            case ITEM_REAGENT_SULFUR:
            case ITEM_REAGENT_GARNET:
            case ITEM_REAGENT_VIAL_OF_DEVIL_ICHOR:
                entry->itemId = ITEM_POTION_CURE_WEAKNESS;
                break;

            case ITEM_REAGENT_MUSHROOM:
            case ITEM_REAGENT_OBSIDIAN:
            case ITEM_REAGENT_VIAL_OF_OOZE_ENDOPLASM:
            case ITEM_REAGENT_MERCURY:
            case ITEM_REAGENT_PHILOSOPHERS_STONE:
                entry->itemId = ITEM_POTION_CATALYST;
                break;
            default:
                break;
        }

        pParty->activeCharacter().playReaction(SPEECH_POTION_SUCCESS);
        pParty->takeHoldingItem();
        rightClickItemActionPerformed = true;
        return;
    }

    GameUI_DrawItemInfo(entry.get());
}

//----- (0045828B) --------------------------------------------------------
Color GetSpellColor(SpellId spellId) {
    if (spellId == SPELL_NONE)
        return colorTable.White;

    switch (magicSchoolForSpell(spellId)) {
    case MAGIC_SCHOOL_FIRE: return colorTable.DarkOrange;
    case MAGIC_SCHOOL_AIR: return colorTable.Anakiwa;
    case MAGIC_SCHOOL_WATER: return colorTable.AzureRadiance;
    case MAGIC_SCHOOL_EARTH: return colorTable.Gray;
    case MAGIC_SCHOOL_SPIRIT: return colorTable.Mercury;
    case MAGIC_SCHOOL_MIND: return colorTable.PurplePink;
    case MAGIC_SCHOOL_BODY: return colorTable.FlushOrange;
    case MAGIC_SCHOOL_LIGHT: return colorTable.PaleCanary;
    case MAGIC_SCHOOL_DARK: return colorTable.MoonRaker;
    default:
        assert(false);
        return colorTable.White;
    }
}

//----- (004B46F8) --------------------------------------------------------
uint64_t GetExperienceRequiredForLevel(int level) {
    int effectiveLevel = 0;
    for (int i = 0; i < level; ++i)
        effectiveLevel += i + 1;
    return (uint64_t)(1000 * effectiveLevel);
}
