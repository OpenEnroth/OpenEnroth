#include <cstdlib>
#include <array>
#include <algorithm>
#include <string>

#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Io/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/Books/MapBook.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

Texture *parchment = nullptr;
Image *messagebox_corner_x = nullptr;       // 5076AC
Image *messagebox_corner_y = nullptr;       // 5076B4
Image *messagebox_corner_z = nullptr;       // 5076A8
Image *messagebox_corner_w = nullptr;       // 5076B0
Image *messagebox_border_top = nullptr;     // 507698
Image *messagebox_border_bottom = nullptr;  // 5076A4
Image *messagebox_border_left = nullptr;    // 50769C
Image *messagebox_border_right = nullptr;   // 5076A0

/*  374 */
#pragma pack(push, 1)
struct stat_coord {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
};
#pragma pack(pop)

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


//----- (004179BC) --------------------------------------------------------
void CharacterUI_DrawTooltip(const char *title, std::string &content) {
    GUIWindow popup_window;  // [sp+Ch] [bp-5Ch]@1

    Pointi pt = mouse->GetCursorPos();

    popup_window.Init();
    popup_window.uFrameWidth = 384;
    popup_window.uFrameHeight = 256;
    popup_window.uFrameX = 128;
    popup_window.uFrameY = pt.y + 30;
    popup_window.uFrameHeight = pFontSmallnum->CalcTextHeight(content, popup_window.uFrameWidth, 24) + 2 * pFontLucida->GetHeight() + 24;
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
        "\f{:05}{}\f00000\n", ui_character_tooltip_header_default_color, title);
    popup_window.DrawTitleText(pFontCreate, 0, 0, 0, colored_title, 3);
    popup_window.DrawText(pFontSmallnum, {1, pFontLucida->GetHeight()}, colorTable.Black.c16(), content, 0, 0, 0);  // popup_window.uFrameY + popup_window.uFrameHeight
}

void CharacterUI_DrawTooltip(const char *title, const char *content) {
    std::string content_str = std::string(content);
    CharacterUI_DrawTooltip(title, content_str);
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

    unsigned int parchment_width = parchment->GetWidth();
    unsigned int parchment_height = parchment->GetHeight();

    uNumTiles = uWidth / parchment_width;
    if (uWidth % parchment_width) ++uNumTiles;
    coord_y = uY;
    for (uint j = 0; j <= uHeight / parchment_height; j++) {
        coord_x = uX - parchment_width;
        for (uint i = uNumTiles + 1; i; --i) {
            coord_x += parchment_width;
            render->DrawTextureNew(coord_x / renwidth, coord_y / renheight,
                                   parchment);
        }
        coord_y += parchment_height;
    }

    render->DrawTextureNew(uX / renwidth, uY / renheight, messagebox_corner_x);
    render->DrawTextureNew(
        uX / renwidth, (uY + uHeight - messagebox_corner_y->GetHeight()) / renheight,
        messagebox_corner_y);
    render->DrawTextureNew(
        (uX + uWidth - messagebox_corner_z->GetWidth()) / renwidth, uY / renheight,
        messagebox_corner_z);
    render->DrawTextureNew(
        (uX + uWidth - messagebox_corner_z->GetWidth()) / renwidth,
        (uY + uHeight - messagebox_corner_y->GetHeight()) / renheight,
        messagebox_corner_w);

    if (uWidth >
        messagebox_corner_x->GetWidth() + messagebox_corner_z->GetWidth()) {
        render->SetUIClipRect(uX + messagebox_corner_x->GetWidth(), uY,
                              uX + uWidth - messagebox_corner_z->GetWidth(),
                              uY + uHeight);

        // horizontal borders
        for (unsigned int x = uX + messagebox_corner_x->GetWidth();
             x < uX + uWidth - messagebox_corner_x->GetWidth();
             x += messagebox_border_top->GetWidth()) {
            render->DrawTextureNew(x / renwidth, uY / renheight,
                                        messagebox_border_top);
            render->DrawTextureNew(
                x / renwidth,
                (uY + uHeight - messagebox_border_bottom->GetHeight()) / renheight,
                messagebox_border_bottom);
        }
    }

    // vertical borders
    if (uHeight >
        messagebox_corner_x->GetHeight() + messagebox_corner_y->GetHeight()) {
        render->SetUIClipRect(uX, uY + messagebox_corner_x->GetHeight(),
                              uX + uWidth,
                              uY + uHeight - messagebox_corner_y->GetHeight());

        for (unsigned int y = uY + messagebox_corner_x->GetHeight();
             y < uY + uHeight - messagebox_corner_y->GetHeight();
             y += messagebox_border_right->GetHeight()) {
            render->DrawTextureNew(uX / renwidth, y / renheight,
                                        messagebox_border_left);
            render->DrawTextureNew(
                (uX + uWidth - messagebox_border_right->GetWidth() - 1) /
                    renwidth,
                y / renheight, messagebox_border_right);
        }
    }
    render->ResetUIClipRect();
}

//----- (0041D895) --------------------------------------------------------
void GameUI_DrawItemInfo(struct ItemGen *inspect_item) {
    unsigned int frameXpos;     // eax@3
    const char *v28;     // edi@69
    int v34;             // esi@81
    char out_text[300];  // [sp+8h] [bp-270h]@40
    SummonedItem v67;
    GUIWindow iteminfo_window;  // [sp+208h] [bp-70h]@2
    char *v84;
    int v85;
    char *Str;  // [sp+270h] [bp-8h]@65

    int r_mask = 0xF800;
    int g_mask = 0x7E0;
    int b_mask = 0x1F;

    if (inspect_item->uItemID == ITEM_NULL)
        return;

    auto inspect_item_image = assets->GetImage_ColorKey(inspect_item->GetIconName());

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
    int itemXspacing = 100 - inspect_item_image->GetWidth();
    int itemYspacing = 144 - inspect_item_image->GetHeight();
    if (itemXspacing > 0) itemXspacing = itemXspacing / 2;
    if (itemYspacing <= 0)
        itemYspacing = 0;
    else
        itemYspacing = itemYspacing / 2;

    // added so window is correct size with broken items
    iteminfo_window.uFrameHeight = inspect_item_image->GetHeight() + itemYspacing + 54;

    if (!pItemTable->pItems[inspect_item->uItemID].uItemID_Rep_St)
        inspect_item->SetIdentified();

    int GoldAmount = 0;
    if (inspect_item->isGold()) {
        GoldAmount = inspect_item->special_enchantment;
    }

    if (pParty->hasActiveCharacter()) {
        // try to identify
        if (!inspect_item->IsIdentified()) {
            if (pParty->getOptionallySharedSkillStrongestEffect(
                    PLAYER_SKILL_TYPE::PLAYER_SKILL_ITEM_ID,
                    pParty->getActiveCharacter(), inspect_item) == 1)
                inspect_item->SetIdentified();
            PlayerSpeech speech = SPEECH_IndentifyItemFail;
            if (!inspect_item->IsIdentified()) {
                GameUI_SetStatusBar(LSTR_IDENTIFY_FAILED);
            } else {
                speech = SPEECH_IndentifyItemStrong;
                if (inspect_item->GetValue() < 100 * (pPlayers[pParty->getActiveCharacter()]->uLevel + 5)) {
                    speech = SPEECH_IndentifyItemWeak;
                }
            }
            if (dword_4E455C) {
                pPlayers[pParty->getActiveCharacter()]->playReaction(speech);
                dword_4E455C = 0;
            }
        }
        inspect_item->UpdateTempBonus(pParty->GetPlayingTime());
        if (inspect_item->IsBroken()) {
            if (pParty->getOptionallySharedSkillStrongestEffect(
                    PLAYER_SKILL_TYPE::PLAYER_SKILL_REPAIR,
                    pParty->getActiveCharacter(), inspect_item) == 1)
                inspect_item->uAttributes =
                    inspect_item->uAttributes & ~ITEM_BROKEN | ITEM_IDENTIFIED;
            PlayerSpeech speech = SPEECH_RepairFail;
            if (!inspect_item->IsBroken())
                speech = SPEECH_RepairSuccess;
            else
                GameUI_SetStatusBar(LSTR_REPAIR_FAILED);
            if (dword_4E455C) {
                pPlayers[pParty->getActiveCharacter()]->playReaction(speech);
                dword_4E455C = 0;
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

        iteminfo_window.DrawTitleText(pFontArrus, 0, 0xCu, colorTable.PaleCanary.c16(), inspect_item->GetDisplayName().c_str(), 3);
        iteminfo_window.DrawTitleText(pFontArrus, 0x64u,
            ((signed int)iteminfo_window.uFrameHeight >> 1) - pFontArrus->CalcTextHeight(localization->GetString(LSTR_BROKEN_ITEM), iteminfo_window.uFrameWidth, 0) / 2,
                                      colorTable.TorchRed.c16(), localization->GetString(LSTR_BROKEN_ITEM), 3);
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
            pFontArrus, 0, 0xCu, colorTable.PaleCanary.c16(),
            pItemTable->pItems[inspect_item->uItemID].pUnidentifiedName, 3);
        iteminfo_window.DrawTitleText(
            pFontArrus, 0x64u,
            ((int)iteminfo_window.uFrameHeight >> 1) -
                pFontArrus->CalcTextHeight(localization->GetString(LSTR_NOT_IDENTIFIED),
                                           iteminfo_window.uFrameWidth, 0) / 2, colorTable.TorchRed.c16(), localization->GetString(LSTR_NOT_IDENTIFIED), 3);
        render->ResetUIClipRect();

        if (inspect_item_image) {
            inspect_item_image->Release();
            inspect_item_image = nullptr;
        }
        return;
    }

    std::string str = localization->FormatString(
        LSTR_FMT_TYPE_S,
        pItemTable->pItems[inspect_item->uItemID].pUnidentifiedName);

    strcpy(out_text, str.c_str());
    out_text[100] = 0;
    out_text[200] = 0;

    switch (inspect_item->GetItemEquipType()) {
        case EQUIP_SINGLE_HANDED:
        case EQUIP_TWO_HANDED: {
            sprintf(out_text + 100, "%s: +%d   %s: %dd%d",
                    localization->GetString(LSTR_ATTACK),
                    (int)inspect_item->GetDamageMod(),
                    localization->GetString(LSTR_DAMAGE),
                    (int)inspect_item->GetDamageDice(),
                    (int)inspect_item->GetDamageRoll());
            if (inspect_item->GetDamageMod()) {
                char mod[16];
                sprintf(mod, "+%d", (int)inspect_item->GetDamageMod());
                strcat(out_text + 100, mod);
            }
            break;
        }

        case EQUIP_BOW:
            sprintf(out_text + 100, "%s: +%d   %s: %dd%d",
                    localization->GetString(LSTR_SHOOT),
                    (int)inspect_item->GetDamageMod(),
                    localization->GetString(LSTR_DAMAGE),
                    (int)inspect_item->GetDamageDice(),
                    (int)inspect_item->GetDamageRoll());
            if (inspect_item->GetDamageMod()) {
                char mod[16];
                sprintf(mod, "+%d", (int)inspect_item->GetDamageMod());
                strcat(out_text + 100, mod);
            }
            break;

        case EQUIP_ARMOUR:
        case EQUIP_SHIELD:
        case EQUIP_HELMET:
        case EQUIP_BELT:
        case EQUIP_CLOAK:
        case EQUIP_GAUNTLETS:
        case EQUIP_BOOTS:
        case EQUIP_RING:
        case EQUIP_AMULET:
            if (inspect_item->GetDamageDice())
                sprintf(
                    out_text + 100, "%s: +%d", localization->GetString(LSTR_ARMOR),
                    inspect_item->GetDamageDice() + inspect_item->GetDamageMod()
                );
            break;

        default:
            break;
    }

    if (!GoldAmount) {
        // this is CORRECT! do not move to switch!
        if (inspect_item->isPotion()) {
            if (inspect_item->uEnchantmentType)
                sprintf(
                    out_text + 200, "%s: %d", localization->GetString(LSTR_POWER),
                    inspect_item->uEnchantmentType
                );
        } else if (inspect_item->isReagent()) {
            sprintf(
                out_text + 200, "%s: %d", localization->GetString(LSTR_POWER),
                inspect_item->GetDamageDice()
            );
        } else if (inspect_item->uEnchantmentType) {
            sprintf(
                out_text + 200, "%s: %s +%d", localization->GetString(LSTR_SPECIAL_2),
                pItemTable->pEnchantments[inspect_item->uEnchantmentType - 1].pBonusStat,
                inspect_item->m_enchantmentStrength);
        } else if (inspect_item->special_enchantment) {
            sprintf(
                out_text + 200, "%s: %s", localization->GetString(LSTR_SPECIAL_2),
                pItemTable->pSpecialEnchantments[inspect_item->special_enchantment].pBonusStatement);
        } else if (inspect_item->isWand()) {
            sprintf(
                out_text + 200, localization->GetString(LSTR_FMT_S_U_OUT_OF_U), localization->GetString(LSTR_CHARGES),
                inspect_item->uNumCharges, inspect_item->uMaxCharges
            );
        }
    }
    iteminfo_window.uFrameWidth -= 12;
    iteminfo_window.uFrameZ =
        iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
    iteminfo_window.uFrameW =
        iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
    int Str_int = (3 * (pFontArrus->GetHeight() + 8));
    v84 = &out_text[0];
    for (uint i = 1; i <= 3; i++) {
        if (*v84)
            Str_int += pFontComic->CalcTextHeight(v84, iteminfo_window.uFrameWidth,
                                              100) +
                   3;
        v84 += 100;
    }
    v28 = pItemTable->pItems[inspect_item->uItemID].pDescription;
    if (*v28)
        Str_int += pFontSmallnum->CalcTextHeight(
            pItemTable->pItems[inspect_item->uItemID].pDescription,
            iteminfo_window.uFrameWidth, 100);
    iteminfo_window.uFrameHeight = inspect_item_image->GetHeight() + itemYspacing + 54;
    if ((signed int)Str_int > (signed int)iteminfo_window.uFrameHeight)
        iteminfo_window.uFrameHeight = (unsigned int)Str_int;
    if (inspect_item->uAttributes & ITEM_TEMP_BONUS &&
        (inspect_item->special_enchantment || inspect_item->uEnchantmentType))
        iteminfo_window.uFrameHeight += pFontComic->GetHeight();
    v85 = 0;
    if (pFontArrus->GetHeight()) {
        iteminfo_window.uFrameWidth -= 24;
        if (pFontArrus->CalcTextHeight(inspect_item->GetIdentifiedName(),
                                       iteminfo_window.uFrameWidth, 0) /
            (signed int)pFontArrus->GetHeight())
            v85 = pFontArrus->GetHeight();
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
                                (iteminfo_window.uFrameY + (float)(iteminfo_window.uFrameHeight - inspect_item_image->GetHeight()) / 2.) / 480.0f,
                                inspect_item_image);

    v34 = (int)(v85 + 35);
    Str = out_text;
    for (uint i = 1; i <= 3; i++) {
        if (*Str) {
            iteminfo_window.DrawText(pFontComic, {100, v34}, colorTable.Black.c16(), Str, 0, 0, 0);
            v34 += pFontComic->CalcTextHeight(Str, iteminfo_window.uFrameWidth, 100, 0) + 3;
        }
        Str += 100;
    }
    v28 = pItemTable->pItems[inspect_item->uItemID].pDescription;
    if (*v28)
        iteminfo_window.DrawText(pFontSmallnum, {100, v34}, colorTable.Black.c16(), v28, 0, 0, 0);
    iteminfo_window.uFrameX += 12;
    iteminfo_window.uFrameWidth -= 24;
    iteminfo_window.DrawTitleText(pFontArrus, 0, 0xCu, colorTable.PaleCanary.c16(),
                                  inspect_item->GetIdentifiedName(), 3);
    iteminfo_window.uFrameWidth += 24;
    iteminfo_window.uFrameX -= 12;

    if (GoldAmount) {
        auto txt = fmt::format("{}: {}", localization->GetString(LSTR_VALUE), GoldAmount);
        iteminfo_window.DrawText(pFontComic, {100, iteminfo_window.uFrameHeight - pFontComic->GetHeight()}, colorTable.Black.c16(), txt, 0, 0, 0);
        render->ResetUIClipRect();
    } else {
        if ((inspect_item->uAttributes & ITEM_TEMP_BONUS) &&
            (inspect_item->special_enchantment || inspect_item->uEnchantmentType)) {
            v67.Initialize(inspect_item->uExpireTime - pParty->GetPlayingTime());

            std::string txt4 = "Duration:";
            bool formatting = false;

            int years = v67.field_18_expire_year - game_starting_year;
            formatting |= years != 0;
            if (formatting)
                txt4 += fmt::format(" {}:yr", years);

            formatting |= v67.field_14_exprie_month != 0;
            if (formatting)
                txt4 += fmt::format(" {}:mo", v67.field_14_exprie_month);

            formatting |= v67.field_C_expire_day != 0;
            if (formatting)
                txt4 += fmt::format(" {}:dy", v67.field_C_expire_day);

            formatting |= v67.field_8_expire_hour != 0;
            if (formatting)
                txt4 += fmt::format(" {}:hr", v67.field_8_expire_hour);

            formatting |= v67.field_4_expire_minute != 0;
            if (formatting)
                txt4 += fmt::format(" {}:mn", v67.field_4_expire_minute);

            iteminfo_window.DrawText(pFontComic, {100, iteminfo_window.uFrameHeight - 2 * pFontComic->GetHeight()}, colorTable.Black.c16(), txt4.data(), 0, 0, 0);
        }

        auto txt2 = fmt::format(
            "{}: {}", localization->GetString(LSTR_VALUE),
            inspect_item->GetValue()
        );
        iteminfo_window.DrawText(pFontComic, {100, iteminfo_window.uFrameHeight - pFontComic->GetHeight()}, colorTable.Black.c16(), txt2.data(), 0, 0, 0);

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

        iteminfo_window.DrawText(pFontComic,
            {pFontComic->GetLineWidth(txt2.data()) + 132, iteminfo_window.uFrameHeight - pFontComic->GetHeight()}, r_mask, txt3, 0, 0, 0);
        render->ResetUIClipRect();
    }
}

void MonsterPopup_Draw(unsigned int uActorID, GUIWindow *pWindow) {
    bool monster_full_informations = false;
    static Actor pMonsterInfoUI_Doll;
    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter()) {
        pParty->setActiveToFirstCanAct();
    }

    int Popup_Y_Offset =
        monster_popup_y_offsets[(pActors[uActorID].pMonsterInfo.uID - 1) / 3] -
        40;

    uint16_t v9 = 0;
    if (pActors[uActorID].pMonsterInfo.uID ==
        pMonsterInfoUI_Doll.pMonsterInfo.uID) {
        v9 = pMonsterInfoUI_Doll.uCurrentActionLength;
    } else {
        // copy actor info if different
        pMonsterInfoUI_Doll = pActors[uActorID];
        pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Bored;
        pMonsterInfoUI_Doll.uCurrentActionTime = 0;
        v9 = vrng->random(256) + 128;
        pMonsterInfoUI_Doll.uCurrentActionLength = v9;
    }

    if (pMonsterInfoUI_Doll.uCurrentActionTime > v9) {
        pMonsterInfoUI_Doll.uCurrentActionTime = 0;
        if (pMonsterInfoUI_Doll.uCurrentActionAnimation == ANIM_Bored ||
            pMonsterInfoUI_Doll.uCurrentActionAnimation == ANIM_AtkMelee) {
            pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Standing;
            pMonsterInfoUI_Doll.uCurrentActionLength = vrng->random(128) + 128;
        } else {
            // rand();
            pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Bored;
            if ((pMonsterInfoUI_Doll.pMonsterInfo.uID < 115 ||
                 pMonsterInfoUI_Doll.pMonsterInfo.uID > 186) &&
                (pMonsterInfoUI_Doll.pMonsterInfo.uID < 232 ||
                 pMonsterInfoUI_Doll.pMonsterInfo.uID > 249) && vrng->random(30) < 100)
                pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_AtkMelee;
            pMonsterInfoUI_Doll.uCurrentActionLength =
                8 *
                pSpriteFrameTable
                    ->pSpriteSFrames[pActors[uActorID].pSpriteIDs[pMonsterInfoUI_Doll.uCurrentActionAnimation]]
                    .uAnimLength;
        }
    }

    Recti doll_rect(pWindow->uFrameX + 13, pWindow->uFrameY + 52, 128, 128);

    {
        SpriteFrame *Portrait_Sprite = pSpriteFrameTable->GetFrame(
            pActors[uActorID]
                .pSpriteIDs[pMonsterInfoUI_Doll.uCurrentActionAnimation],
            pMonsterInfoUI_Doll.uCurrentActionTime);

        // Draw portrait border
        render->ResetUIClipRect();
        render->FillRectFast(doll_rect.x, doll_rect.y, 128, 128, colorTable.Black.c32());
        render->BeginLines2D();
        int x0 = doll_rect.x;
        int x1 = doll_rect.x + doll_rect.w;
        int y0 = doll_rect.y;
        int y1 = doll_rect.y + doll_rect.h;
        render->RasterLine2D(x0 - 1, y0 - 1, x1 + 1, y0 - 1, colorTable.Jonquil.c32());  // горизонтальная верхняя линия
        render->RasterLine2D(x0 - 1, y1 + 1, x0 - 1, y0 - 1, colorTable.Jonquil.c32());  // горизонтальная нижняя линия
        render->RasterLine2D(x1 + 1, y1 + 1, x0 - 1, y1 + 1, colorTable.Jonquil.c32());  // левая вертикальная линия
        render->RasterLine2D(x1 + 1, y0 - 1, x1 + 1, y1 + 1, colorTable.Jonquil.c32());  // правая вертикальная линия
        render->EndLines2D();

        // Draw portrait
        render->DrawMonsterPortrait(doll_rect, Portrait_Sprite, Popup_Y_Offset);
    }

    // Draw name and profession
    std::string str;
    if (pActors[uActorID].sNPC_ID) {
        str = NameAndTitle(GetNPCData(pActors[uActorID].sNPC_ID));
    } else {
        str = GetDisplayName(&pActors[uActorID]);
    }
    pWindow->DrawTitleText(pFontComic, 0, 0xCu, colorTable.PaleCanary.c16(), str, 3);

    // health bar
    Actor::DrawHealthBar(&pActors[uActorID], pWindow);

    bool normal_level = false;
    bool expert_level = false;
    bool master_level = false;
    bool grandmaster_level = false;
    bool for_effects = false;

    PLAYER_SKILL_LEVEL skill_points = 0;
    PLAYER_SKILL_MASTERY skill_mastery = PLAYER_SKILL_MASTERY_NONE;

    pMonsterInfoUI_Doll.uCurrentActionTime += pMiscTimer->uTimeElapsed;
    if (pPlayers[pParty->getActiveCharacter()]->GetActualSkillLevel(
            PLAYER_SKILL_MONSTER_ID)) {
        skill_points = pPlayers[pParty->getActiveCharacter()]->GetActualSkillLevel(PLAYER_SKILL_MONSTER_ID);
        skill_mastery = pPlayers[pParty->getActiveCharacter()]->GetActualSkillMastery(PLAYER_SKILL_MONSTER_ID);
        if (skill_mastery == PLAYER_SKILL_MASTERY_NOVICE) {
            if (skill_points + 10 >= pActors[uActorID].pMonsterInfo.uLevel)
                normal_level = 1;
        } else if (skill_mastery == PLAYER_SKILL_MASTERY_EXPERT) {
            if (2 * skill_points + 10 >=
                pActors[uActorID].pMonsterInfo.uLevel) {
                normal_level = 1;
                expert_level = 1;
            }
        } else if (skill_mastery == PLAYER_SKILL_MASTERY_MASTER) {
            if (3 * skill_points + 10 >=
                pActors[uActorID].pMonsterInfo.uLevel) {
                normal_level = 1;
                expert_level = 1;
                master_level = 1;
            }
        } else if (skill_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
            normal_level = 1;
            expert_level = 1;
            master_level = 1;
            grandmaster_level = 1;
        }
    }

    PlayerSpeech speech;
    if (pActors[uActorID].uAIState != Dead &&
        pActors[uActorID].uAIState != Dying &&
        !dword_507BF0_is_there_popup_onscreen &&
        pPlayers[pParty->getActiveCharacter()]->GetActualSkillLevel(
            PLAYER_SKILL_MONSTER_ID)) {
        if (normal_level || expert_level || master_level || grandmaster_level) {
            if (pActors[uActorID].pMonsterInfo.uLevel >=
                pPlayers[pParty->getActiveCharacter()]->uLevel - 5)
                speech = SPEECH_IDMonsterStrong;
            else
                speech = SPEECH_IDMonsterWeak;
        } else {
            speech = SPEECH_IDMonsterFail;
        }
        pPlayers[pParty->getActiveCharacter()]->playReaction(speech);
    }

    if ((signed int)(pParty->pPlayers[pParty->getActiveCharacter() - 1]
                         .GetActualSkillMastery(PLAYER_SKILL_MONSTER_ID)) >= 3)
        for_effects = 1;

    if (monster_full_informations == true) {
        normal_level = 1;       //
        expert_level = 1;       //
        master_level = 1;       //
        grandmaster_level = 1;  //
        for_effects = 1;
    }

    int pTextHeight = 0;
    const char *pText = nullptr;
    int pTextColorID = 0;
    pWindow->DrawText(pFontSmallnum, {12, 196}, colorTable.Jonquil.c16(), localization->GetString(LSTR_EFFECTS), 0, 0, 0);
    if (!for_effects) {
        pWindow->DrawText(pFontSmallnum, {28, pFontSmallnum->GetHeight() + 193}, colorTable.White.c16(), localization->GetString(LSTR_UNKNOWN_VALUE), 0, 0, 0);
    } else {
        pText = "";
        pTextHeight = pFontSmallnum->GetHeight() + 193;
        for (ACTOR_BUFF_INDEX i : pActors[uActorID].pActorBuffs.indices()) {
            if (pActors[uActorID].pActorBuffs[i].Active()) {
                switch (i) {
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
                        pText = "";
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
                    case ACTOR_BUFF_PAIN_HAMMERHANDS:
                        pTextColorID = 73;
                        pText = localization->GetString(LSTR_HAMMERHANDS);
                        break;
                    default:
                        pText = "";
                        break;
                }
                if (!iequals(pText, "")) {
                    pWindow->DrawText(pFontSmallnum, {28, pTextHeight}, GetSpellColor(pTextColorID), pText, 0, 0, 0);
                    pTextHeight =
                        pTextHeight + *(char *)((int64_t)pFontSmallnum + 5) - 3;
                }
            }
        }
        if (iequals(pText, ""))
            pWindow->DrawText(pFontSmallnum, {28, pTextHeight}, colorTable.Jonquil.c16(),
                              localization->GetString(LSTR_NONE), 0, 0, 0);
    }

    std::string txt2;
    if (normal_level) {
        auto str =
            fmt::format("{}\f{:05}\t100{}\n", localization->GetString(LSTR_HIT_POINTS), 0,
                         pActors[uActorID].pMonsterInfo.uHP);
        pWindow->DrawText(pFontSmallnum, {150, doll_rect.y}, colorTable.Jonquil.c16(), str, 0, 0, 0);
        pTextHeight = doll_rect.y + pFontSmallnum->GetHeight() - 3;
        txt2 = fmt::format("{}\f{:05}\t100{}\n", localization->GetString(LSTR_ARMOR_CLASS), 0,
                            pActors[uActorID].pMonsterInfo.uAC);
    } else {
        auto str = fmt::format(
            "{}\f{:05}\t100{}\n", localization->GetString(LSTR_HIT_POINTS), 0,
            localization->GetString(LSTR_UNKNOWN_VALUE));
        pWindow->DrawText(pFontSmallnum, {150, doll_rect.y}, colorTable.Jonquil.c16(), str, 0, 0, 0);
        pTextHeight = doll_rect.y + pFontSmallnum->GetHeight() - 3;
        txt2 = fmt::format(
            "{}\f{:05}\t100{}\n", localization->GetString(LSTR_ARMOR_CLASS), 0,
            localization->GetString(LSTR_UNKNOWN_VALUE));
    }
    pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), txt2, 0, 0, 0);
    pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 6 +
                  pFontSmallnum->GetHeight();

    const char *content[11] = {0};
    content[0] = localization->GetSpellSchoolName(0);
    content[1] = localization->GetSpellSchoolName(1);
    content[2] = localization->GetSpellSchoolName(2);
    content[3] = localization->GetSpellSchoolName(3);
    content[4] = localization->GetString(LSTR_PHYSICAL);
    content[5] = localization->GetString(LSTR_MAGIC);
    content[6] = localization->GetSpellSchoolName(5);
    content[7] = localization->GetSpellSchoolName(4);
    content[8] = localization->GetSpellSchoolName(6);
    content[9] = localization->GetSpellSchoolName(7);
    content[10] = localization->GetSpellSchoolName(8);

    std::string txt4;
    if (expert_level) {
        auto txt3 = fmt::format(
            "{}\f{:05}\t080{}\n", localization->GetString(LSTR_ATTACK), 0,
            content[pActors[uActorID].pMonsterInfo.uAttack1Type]);
        pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), txt3, 0, 0, 0);

        pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        if (pActors[uActorID].pMonsterInfo.uAttack1DamageBonus)
            txt4 = fmt::format(
                "{}\f{:05}\t080{}d{}+{}\n", localization->GetString(LSTR_DAMAGE), 0,
                pActors[uActorID].pMonsterInfo.uAttack1DamageDiceRolls,
                pActors[uActorID].pMonsterInfo.uAttack1DamageDiceSides,
                pActors[uActorID].pMonsterInfo.uAttack1DamageBonus);
        else
            txt4 = fmt::format(
                "{}\f{:05}\t080{}d{}\n", localization->GetString(LSTR_DAMAGE), 0,
                pActors[uActorID].pMonsterInfo.uAttack1DamageDiceRolls,
                pActors[uActorID].pMonsterInfo.uAttack1DamageDiceSides);
    } else {
        auto txt3 = fmt::format(
            "{}\f{:05}\t080{}\n", localization->GetString(LSTR_ATTACK), 0,
            localization->GetString(LSTR_UNKNOWN_VALUE)
        );
        pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), txt3, 0, 0, 0);
        pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        txt4 = fmt::format(
            "{}\f{:05}\t080{}\n", localization->GetString(LSTR_DAMAGE), 0,
            localization->GetString(LSTR_UNKNOWN_VALUE)
        );
    }
    pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), txt4, 0, 0, 0);

    pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 6 +
                  pFontSmallnum->GetHeight();
    if (!master_level) {
        auto txt5 = fmt::format(
            "{}\f{:05}\t080{}\n", localization->GetString(LSTR_SPELL), 0,
            localization->GetString(LSTR_UNKNOWN_VALUE)
        );
        pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), txt5, 0, 0, 0);
        pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
    } else {
        pText = localization->GetString(LSTR_SPELL);
        if (pActors[uActorID].pMonsterInfo.uSpell1ID != SPELL_NONE && pActors[uActorID].pMonsterInfo.uSpell2ID != SPELL_NONE)
            pText = localization->GetString(LSTR_SPELLS);
        if (pActors[uActorID].pMonsterInfo.uSpell1ID != SPELL_NONE) {
            auto txt6 = fmt::format(
                "{}\f{:05}\t070{}\n", pText, 0,
                pSpellStats->pInfos[pActors[uActorID].pMonsterInfo.uSpell1ID].pShortName
            );  // "%s\f%05u\t060%s\n"
            pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), txt6, 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
        if (pActors[uActorID].pMonsterInfo.uSpell2ID != SPELL_NONE) {
            auto txt6 = fmt::format(
                "\f{:05}\t070{}\n", 0,
                pSpellStats->pInfos[pActors[uActorID].pMonsterInfo.uSpell2ID]
                    .pShortName);  // "%s\f%05u\t060%s\n"
            pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), txt6, 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
        if (!pActors[uActorID].pMonsterInfo.uSpell1ID &&
            !pActors[uActorID].pMonsterInfo.uSpell2ID) {
            auto txt6 = fmt::format(
                "{}\f{:05}\t070{}\n", localization->GetString(LSTR_SPELL), 0,
                localization->GetString(LSTR_NONE));  // "%s\f%05u\t060%s\n"
            pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), txt6, 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
    }

    pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
    pWindow->DrawText(pFontSmallnum, {150, pTextHeight}, colorTable.Jonquil.c16(), localization->GetString(LSTR_RESISTANCES), 0, 0, 0);
    pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;

    const char *string_name[10] = {0};
    string_name[0] = localization->GetSpellSchoolName(0);  // Fire
    string_name[1] = localization->GetSpellSchoolName(1);  // Air
    string_name[2] = localization->GetSpellSchoolName(2);
    string_name[3] = localization->GetSpellSchoolName(3);
    string_name[4] = localization->GetSpellSchoolName(4);
    string_name[5] = localization->GetSpellSchoolName(5);
    string_name[6] = localization->GetSpellSchoolName(6);
    string_name[7] = localization->GetSpellSchoolName(7);
    string_name[8] = localization->GetSpellSchoolName(8);
    string_name[9] = localization->GetString(LSTR_PHYSICAL);

    unsigned char resistances[11] = {0};
    resistances[0] = pActors[uActorID].pMonsterInfo.uResFire;
    resistances[1] = pActors[uActorID].pMonsterInfo.uResAir;
    resistances[2] = pActors[uActorID].pMonsterInfo.uResWater;
    resistances[3] = pActors[uActorID].pMonsterInfo.uResEarth;
    resistances[4] = pActors[uActorID].pMonsterInfo.uResMind;
    resistances[5] = pActors[uActorID].pMonsterInfo.uResSpirit;
    resistances[6] = pActors[uActorID].pMonsterInfo.uResBody;
    resistances[7] = pActors[uActorID].pMonsterInfo.uResLight;
    resistances[8] = pActors[uActorID].pMonsterInfo.uResPhysical;
    resistances[9] = pActors[uActorID].pMonsterInfo.uResDark;

    if (grandmaster_level) {
        for (uint i = 0; i < 10; i++) {
            if (resistances[i] == 200) {
                pText = localization->GetString(LSTR_IMMUNE);
            } else {
                if (resistances[i])
                    pText = localization->GetString(LSTR_RESISTANT);
                else
                    pText = localization->GetString(LSTR_NONE);
            }

            pWindow->DrawText(pFontSmallnum, {170, pTextHeight}, colorTable.Jonquil.c16(), fmt::format("{}\f{:05}\t070{}\n", string_name[i], 0, pText), 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
    } else {
        for (uint i = 0; i < 10; ++i) {
            pWindow->DrawText(pFontSmallnum, {170, pTextHeight}, colorTable.Jonquil.c16(), fmt::format("{}\f{:05}\t070{}\n", string_name[i], 0, localization->GetString(LSTR_UNKNOWN_VALUE)), 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
    }

    // cast spell: Detect life
    if (pParty->pPartyBuffs[PARTY_BUFF_DETECT_LIFE].Active()) {
        std::string str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_HIT_POINTS), pActors[uActorID].sCurrentHP);
        pFontSmallnum->GetLineWidth(str);
        pWindow->DrawTitleText(pFontSmallnum, 0, pWindow->uFrameHeight - pFontSmallnum->GetHeight() - 12, 0, str, 3);
    }

    // TODO(captainurist): this isn't about verbose logging and should depend on some other parameter
    if (engine->config->debug.VerboseLogging.value()) {
        std::string str = fmt::format("AI State: {}", std::to_underlying(pActors[uActorID].uAIState));
        pFontSmallnum->GetLineWidth(str);
        pWindow->DrawTitleText(pFontSmallnum, 0, pWindow->uFrameHeight - pFontSmallnum->GetHeight() - 12, 0, str, 3);
    }
}

/**
  * @offset 0x00417BB5.
  *
  * @brief Generating message for skill description popup.
  *
  * @param uPlayerID                     Character identifier.
  * @param uPlayerSkillType              Skill type identifier.
  */
std::string CharacterUI_GetSkillDescText(unsigned int uPlayerID, PLAYER_SKILL_TYPE uPlayerSkillType) {
    size_t line_width = std::max({
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_NORMAL)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_EXPERT)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_MASTER)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_BONUS_2))
    });

    int base_skill = pParty->pPlayers[uPlayerID].pActiveSkills[uPlayerSkillType] & 0x3F;
    int actual_skill = pParty->pPlayers[uPlayerID].GetActualSkillLevel(uPlayerSkillType) & 0x3F;

    const char *desc = localization->GetSkillDescription(uPlayerSkillType);
    std::string Description = desc ? desc : "";
    if (localization->GetSkillDescriptionNormal(uPlayerSkillType)) {
        Description = fmt::format("{}\n\n", Description);

        for (PLAYER_SKILL_MASTERY mastery : SkillMasteries()) {
            Description += fmt::format(
                "\f{:05}{}\t{:03}:\t{:03}{}\t000\n",
                GetSkillColor(pParty->pPlayers[uPlayerID].classType, uPlayerSkillType, mastery),
                localization->MasteryName(mastery),
                line_width + 3,
                line_width + 10,
                localization->GetSkillDescription(uPlayerSkillType, mastery)
            );
        }
    }

    if (base_skill != actual_skill)
        Description += fmt::format("\f{:05}\n{}\t{:03}:\t{:03}+{}\n", colorTable.White.c16(), localization->GetString(LSTR_BONUS_2), line_width + 3, line_width + 10, actual_skill - base_skill);

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
                PLAYER_SKILL_TYPE skill = static_cast<PLAYER_SKILL_TYPE>(pButton->msg_param);
                std::string pSkillDescText = CharacterUI_GetSkillDescText(pParty->getActiveCharacter() - 1, skill);
                CharacterUI_DrawTooltip(localization->GetSkillName(skill), pSkillDescText);
            }
        }
    } else {
        CharacterUI_DrawTooltip(
            localization->GetString(LSTR_SKILL_POINTS),
            pSkillPointsAttributeDescription);
    }
}

//----- (00418083) --------------------------------------------------------
void CharacterUI_StatsTab_ShowHint() {
    int pStringNum;         // edi@1
    signed int pTextColor;  // eax@15
    const char *pHourWord;  // ecx@17
    const char *pDayWord;   // eax@20
    int v15;                // ebx@28
    int pHour;              // [sp+14h] [bp-1Ch]@15
    unsigned int pDay;      // [sp+24h] [bp-Ch]@15

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
                localization->GetAttirubteName(pStringNum),
                localization->GetAttributeDescription(pStringNum));
            break;
        case 7:  // Health Points
            if (pHealthPointsAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_HIT_POINTS),
                                        pHealthPointsAttributeDescription);
            break;
        case 8:  // Spell Points
            if (pSpellPointsAttributeDescription)
                CharacterUI_DrawTooltip(
                    localization->GetString(LSTR_SPELL_POINTS), pSpellPointsAttributeDescription
                );
            break;
        case 9:
            if (pArmourClassAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_ARMOR_CLASS),
                                        pArmourClassAttributeDescription);
            break;
        case 10:  // Player Condition
        {
            auto str = std::string(pPlayerConditionAttributeDescription) + "\n";

            for (Condition condition : conditionImportancyTable()) {
                if (pPlayers[pParty->getActiveCharacter()]->conditions.Has(condition)) {
                    str += " \n";
                    GameTime condition_time =
                        pParty->GetPlayingTime() - pPlayers[pParty->getActiveCharacter()]->conditions.Get(condition);
                    pHour = condition_time.GetHoursOfDay();
                    pDay = condition_time.GetDays();
                    pTextColor = GetConditionDrawColor(condition);
                    str += fmt::format("\f{:05}{}\f00000 - ", pTextColor, localization->GetCharacterConditionName(condition));
                    if (pHour && pHour <= 1)
                        pHourWord = localization->GetString(LSTR_HOUR);
                    else
                        pHourWord = localization->GetString(LSTR_HOURS);
                    if (!pDay ||
                        (pDayWord = localization->GetString(LSTR_DAY_CAPITALIZED), pDay > 1))
                        pDayWord = localization->GetString(LSTR_DAYS);
                    str += fmt::format(
                        "{} {}, {} {}", pDay, pDayWord, pHour, pHourWord
                    );
                }
            }

            if (!str.empty())
                CharacterUI_DrawTooltip(localization->GetString(LSTR_CONDITION), str);
            break;
        }

        case 11:  // Fast Spell
            if (pFastSpellAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_QUICK_SPELL),
                                        pFastSpellAttributeDescription);
            break;

        case 12:  // Player Age
            if (pPlayerAgeAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_AGE),
                                        pPlayerAgeAttributeDescription);
            break;

        case 13:  // Player Level
            if (pPlayerLevelAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_LEVEL),
                                        pPlayerLevelAttributeDescription);
            break;

        case 14:  // Experience
        {
            v15 = pPlayers[pParty->getActiveCharacter()]->uLevel;
            do {
                if (pPlayers[pParty->getActiveCharacter()]->uExperience < GetExperienceRequiredForLevel(v15))
                    break;
                ++v15;
            } while (v15 <= 10000);

            std::string str1;
            std::string str2;
            if (v15 > pPlayers[pParty->getActiveCharacter()]->uLevel)
                str1 = localization->FormatString(
                    LSTR_ELIGIBLE_TO_LEVELUP, v15);
            str2 = localization->FormatString(
                LSTR_XP_UNTIL_NEXT_LEVEL,
                (int)(GetExperienceRequiredForLevel(v15) - pPlayers[pParty->getActiveCharacter()]->uExperience),
                v15 + 1);
            str1 += "\n" + str2;

            str2 = std::string(pPlayerExperienceAttributeDescription) + "\n \n" + str1;

            CharacterUI_DrawTooltip(localization->GetString(LSTR_EXPERIENCE), str2);
            break;
        }

        case 15:  // Attack Bonus
            if (pAttackBonusAttributeDescription) {
                int meleerecov = pPlayers[pParty->getActiveCharacter()]->GetAttackRecoveryTime(false);
                // TODO(captainurist): fmt can throw
                std::string description = fmt::sprintf(localization->GetString(LSTR_FMT_RECOVERY_TIME_D), meleerecov);
                description = fmt::format("{}\n\n{}", pAttackBonusAttributeDescription, description.c_str());
                CharacterUI_DrawTooltip(localization->GetString(LSTR_ATTACK_BONUS), description);
            }
            break;

        case 16:  // Attack Damage
            if (pAttackDamageAttributeDescription)
                CharacterUI_DrawTooltip(
                    localization->GetString(LSTR_ATTACK_DAMAGE),
                    pAttackDamageAttributeDescription
                );
            break;

        case 17:  // Missle Bonus
            if (pMissleBonusAttributeDescription) {
                int missrecov = pPlayers[pParty->getActiveCharacter()]->GetAttackRecoveryTime(true);
                // TODO(captainurist): fmt can throw
                std::string description = fmt::sprintf(localization->GetString(LSTR_FMT_RECOVERY_TIME_D), missrecov);
                description = fmt::format("{}\n\n{}", pAttackBonusAttributeDescription, description);
                CharacterUI_DrawTooltip(localization->GetString(LSTR_SHOOT_BONUS), description);
            }
            break;

        case 18:  // Missle Damage
            if (pMissleDamageAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_SHOOT_DAMAGE),
                                        pMissleDamageAttributeDescription);
            break;

        case 19:  // Fire Resistance
            if (pFireResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_FIRE),
                                        pFireResistanceAttributeDescription);
            break;

        case 20:  // Air Resistance
            if (pAirResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_AIR),
                                        pAirResistanceAttributeDescription);
            break;

        case 21:  // Water Resistance
            if (pWaterResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_WATER),
                                        pWaterResistanceAttributeDescription);
            break;

        case 22:  // Earth Resistance
            if (pEarthResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_EARTH),
                                        pEarthResistanceAttributeDescription);
            break;

        case 23:  // Mind Resistance
            if (pMindResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_MIND),
                                        pMindResistanceAttributeDescription);
            break;

        case 24:  // Body Resistance
            if (pBodyResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_BODY),
                                        pBodyResistanceAttributeDescription);
            break;

        case 25:  // Skill Points
            if (pSkillPointsAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(LSTR_SKILL_POINTS),
                                        pSkillPointsAttributeDescription);
            break;

        case 26:  // Class description
        {
            CharacterUI_DrawTooltip(localization->GetClassName(
                                        pPlayers[pParty->getActiveCharacter()]->classType),
                                    localization->GetClassDescription(
                                        pPlayers[pParty->getActiveCharacter()]->classType));
        } break;

        default:
            break;
    }
}

//----- (00410B28) --------------------------------------------------------
void DrawSpellDescriptionPopup(int spell_index_in_book) {
    SpellInfo *spell;             // esi@1
    unsigned int v3;              // eax@2
    GUIWindow spell_info_window;  // [sp+Ch] [bp-68h]@4

    Pointi pt = mouse->GetCursorPos();
    SPELL_TYPE spell_id = static_cast<SPELL_TYPE>(spell_index_in_book + 11 * pPlayers[pParty->getActiveCharacter()]->lastOpenedSpellbookPage + 1);

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
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_NORMAL)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_EXPERT)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_MASTER)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND))
    });

    std::string str = fmt::format(
        "{}\n\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}",
        spell->pDescription,
        localization->GetString(LSTR_NORMAL), v5 + 3, v5 + 10, spell->pBasicSkillDesc,
        localization->GetString(LSTR_EXPERT), v5 + 3, v5 + 10, spell->pExpertSkillDesc,
        localization->GetString(LSTR_MASTER), v5 + 3, v5 + 10, spell->pMasterSkillDesc,
        localization->GetString(LSTR_GRAND), v5 + 3, v5 + 10, spell->pGrandmasterSkillDesc
    );
    spell_info_window.uFrameHeight += pFontSmallnum->CalcTextHeight(
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
        pFontArrus, 0x78u, 0xCu, colorTable.PaleCanary.c16(), spell->pName, 3);
    spell_info_window.DrawText(pFontSmallnum, {120, 44}, 0, str, 0, 0, 0);
    spell_info_window.uFrameWidth = 108;
    spell_info_window.uFrameZ = spell_info_window.uFrameX + 107;
    PLAYER_SKILL_TYPE skill = static_cast<PLAYER_SKILL_TYPE>(pPlayers[pParty->getActiveCharacter()]->lastOpenedSpellbookPage + 12);
    PLAYER_SKILL_MASTERY skill_mastery = pPlayers[pParty->getActiveCharacter()]->GetSkillMastery(skill);
    spell_info_window.DrawTitleText(pFontComic, 12, 75, 0, localization->GetSkillName(skill), 3);

    auto str2 = fmt::format(
        "{}\n{}", localization->GetString(LSTR_SP_COST),
        pSpellDatas[spell_id].mana_per_skill[std::to_underlying(skill_mastery) - 1]);
    spell_info_window.DrawTitleText(
        pFontComic, 12,
        spell_info_window.uFrameHeight - pFontComic->GetHeight() - 16, 0, str2,
        3);
    dword_507B00_spell_info_to_draw_in_popup = 0;
}

//----- (00416D62) --------------------------------------------------------
void UI_OnMouseRightClick(int mouse_x, int mouse_y) {
    int v5;                  // esi@62
    // GUIButton *pButton;      // esi@84
    const char *pStr;        // edi@85
    // const char *pHint;       // edx@113
    GUIWindow popup_window;  // [sp+4h] [bp-74h]@32

    if (current_screen_type == CURRENT_SCREEN::SCREEN_VIDEO || GetCurrentMenuID() == MENU_MAIN)
        return;


    unsigned int pX = mouse_x;
    unsigned int pY = mouse_y;

    // if ( render->bWindowMode )
    {
        Pointi pt = Pointi(pX, pY);
        if (pt.x < 1 || pt.y < 1 || pt.x > 638 || pt.y > 478) {
            back_to_game();
            return;
        }
    }
    if (pParty->pPickedItem.uItemID != ITEM_NULL) {
        // Use item on character portrait
        for (int i = 0; i < pParty->pPlayers.size(); ++i) {
            if ((signed int)pX > RightClickPortraitXmin[i] &&
                (signed int)pX < RightClickPortraitXmax[i] &&
                (signed int)pY > 375 && (signed int)pY < 466) {
                pPlayers[pParty->getActiveCharacter()]->useItem(i, true);
                return;
            }
        }
    }

    pEventTimer->Pause();
    switch (current_screen_type) {
        case CURRENT_SCREEN::SCREEN_CASTING: {
            Inventory_ItemPopupAndAlchemy();
            break;
        }
        case CURRENT_SCREEN::SCREEN_CHEST: {
            if (!pPlayers[pParty->getActiveCharacter()]->CanAct()) {
                static std::string hint_reference;
                hint_reference = localization->FormatString(
                    LSTR_FMT_S_IS_IN_NO_CODITION_TO_S,
                    pPlayers[pParty->getActiveCharacter()]->pName.c_str(),
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
                    int chestindex = vChests[pGUIWindow_CurrentMenu->wData.val].pInventoryIndices[invMatrixIndex];
                    if (chestindex < 0) {
                        invMatrixIndex = (-(chestindex + 1));
                        chestindex = vChests[pGUIWindow_CurrentMenu->wData.val].pInventoryIndices[invMatrixIndex];
                    }

                    if (chestindex) {
                        int itemindex = chestindex - 1;

                        GameUI_DrawItemInfo(&vChests[pGUIWindow_CurrentMenu->wData.val].igChestItems[itemindex]);
                    }
                }
            }
            break;
        }

        case CURRENT_SCREEN::SCREEN_GAME:  // In the main menu displays a pop-up window(В
                           // главном меню показывает всплывающее окно)
        {
            if (GetCurrentMenuID() > 0) break;
            if ((signed int)pY > (signed int)pViewport->uViewportBR_Y) {
                popup_window.wData.val = pX / 118;
                if ((signed int)pX / 118 < 4) {  // portaits zone
                    popup_window.sHint.clear();
                    popup_window.uFrameWidth = 400;
                    popup_window.uFrameHeight = 200;
                    popup_window.uFrameX = 38;
                    popup_window.uFrameY = 60;
                    pAudioPlayer->PauseSounds(-1);
                    GameUI_CharacterQuickRecord_Draw(
                        &popup_window, &pParty->pPlayers[popup_window.wData.val]);
                }
            } else if ((int)pX > pViewport->uViewportBR_X) {
                if (pY >= 130) {
                    if (pX >= 476 && pX <= 636 && pY >= 240 && pY <= 300) {  // buff_tooltip zone
                        popup_window.sHint.clear();
                        popup_window.uFrameWidth = 400;
                        popup_window.uFrameHeight = 200;
                        popup_window.uFrameX = 38;
                        popup_window.uFrameY = 60;
                        pAudioPlayer->PauseSounds(-1);
                        popup_window._41D73D_draw_buff_tooltip();
                    } else if ((int)pX < 485 || (int)pX > 548 ||
                               (int)pY < 156 ||
                               (int)pY > 229) {  // NPC zone
                        if (!((signed int)pX < 566 || (signed int)pX > 629 ||
                              (signed int)pY < 156 || (signed int)pY > 229)) {
                            pAudioPlayer->PauseSounds(-1);
                            GameUI_DrawNPCPopup((void *)1);  // NPC 2
                        }
                    } else {
                        pAudioPlayer->PauseSounds(-1);
                        GameUI_DrawNPCPopup(0);  // NPC 1
                    }
                } else {  // minimap zone
                    popup_window.sHint = GameUI_GetMinimapHintText();
                    popup_window.uFrameWidth = 256;
                    popup_window.uFrameX = 130;
                    popup_window.uFrameY = 140;
                    popup_window.uFrameHeight = 64;
                    pAudioPlayer->PauseSounds(-1);
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

                auto vis = EngineIocContainer::ResolveVis();
                v5 = vis->get_picked_object_zbuf_val().object_pid;
                /*else
                v5 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]];*/
                if (PID_TYPE(v5) == OBJECT_Actor) {
                    render->BeginScene2D();
                    popup_window.DrawMessageBox(1);
                    MonsterPopup_Draw(PID_ID(v5), &popup_window);
                }
                if (PID_TYPE(v5) == OBJECT_Item) {
                    if (!(pObjectList->pObjects[pSpriteObjects[PID_ID(v5)].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE)) {
                        GameUI_DrawItemInfo(&pSpriteObjects[PID_ID(v5)].containing_item);
                    }
                }
            }
            break;
        }
        case CURRENT_SCREEN::SCREEN_BOOKS: {
            if (!MapBookOpen ||
                (signed int)pX < (signed int)pViewport->uViewportTL_X ||
                (signed int)pX > (signed int)pViewport->uViewportBR_X ||
                (signed int)pY < (signed int)pViewport->uViewportTL_Y ||
                (signed int)pY > (signed int)pViewport->uViewportBR_Y ||
                ((popup_window.sHint = GetMapBookHintText(mouse_x, mouse_y)).empty()))
                break;
            popup_window.uFrameWidth =
                (pFontArrus->GetLineWidth(popup_window.sHint) + 32) + 0.5f;
            popup_window.uFrameX = pX + 5;
            popup_window.uFrameY = pY + 5;
            popup_window.uFrameHeight = 64;
            pAudioPlayer->PauseSounds(-1);
            popup_window.DrawMessageBox(0);
            break;
        }
        case CURRENT_SCREEN::SCREEN_CHARACTERS:
        case CURRENT_SCREEN::SCREEN_SHOP_INVENTORY:
        case CURRENT_SCREEN::SCREEN_CHEST_INVENTORY: {
            if ((signed int)pX > 467 && current_screen_type != CURRENT_SCREEN::SCREEN_SHOP_INVENTORY)
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
        case CURRENT_SCREEN::SCREEN_SPELL_BOOK: {
            if (dword_507B00_spell_info_to_draw_in_popup)
                DrawSpellDescriptionPopup(
                    dword_507B00_spell_info_to_draw_in_popup - 1);
            break;
        }
        case CURRENT_SCREEN::SCREEN_HOUSE: {
            if ((signed int)pY < 345 && (signed int)pX < 469)
                ShowPopupShopItem();
            if ((signed int)pX >= 469)
                ShowPopupShopSkills();
            break;
        }
        case CURRENT_SCREEN::SCREEN_PARTY_CREATION: {
            popup_window.sHint.clear();
            pStr = 0;
            for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
                if (pButton->uButtonType == 1 && pButton->uButtonType != 3 &&
                    (signed int)pX > (signed int)pButton->uX &&
                    (signed int)pX < (signed int)pButton->uZ &&
                    (signed int)pY > (signed int)pButton->uY &&
                    (signed int)pY < (signed int)pButton->uW) {
                    switch (pButton->msg) {
                        case UIMSG_0:  // stats info
                            popup_window.sHint = localization->GetAttributeDescription(
                                (int)pButton->msg_param % 7);
                            pStr = localization->GetAttirubteName(
                                (int)pButton->msg_param % 7);
                            break;
                        case UIMSG_PlayerCreationClickPlus:  // Plus button info
                            pStr = localization->GetString(LSTR_ADD);
                            popup_window.sHint = localization->GetString(
                                LSTR_SKILL_INCREASE_HINT);
                            break;
                        case UIMSG_PlayerCreationClickMinus:  // Minus button
                                                              // info
                            pStr = localization->GetString(LSTR_SUBTRACT);
                            popup_window.sHint = localization->GetString(
                                LSTR_SKILL_DECREASE_HINT);
                            break;
                        case UIMSG_PlayerCreationSelectActiveSkill:  // Available
                                                                     // skill
                                                                     // button
                                                                     // info
                            pStr = localization->GetSkillName(
                                pParty->pPlayers
                                        [uPlayerCreationUI_SelectedCharacter]
                                    .GetSkillIdxByOrder(pButton->msg_param +
                                                        4));
                            popup_window
                                .sHint = localization->GetSkillDescription(
                                pParty->pPlayers
                                        [uPlayerCreationUI_SelectedCharacter]
                                    .GetSkillIdxByOrder(pButton->msg_param +
                                                        4));
                            break;
                        case UIMSG_PlayerCreationSelectClass:  // Available
                                                               // Class Info
                            popup_window.sHint =
                                localization->GetClassDescription(
                                    pButton->msg_param);
                            pStr =
                                localization->GetClassName(pButton->msg_param);
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
                            pStr = pParty->pPlayers[pButton->msg_param].pName.c_str();
                            popup_window
                                .sHint = localization->GetClassDescription(
                                pParty->pPlayers[pButton->msg_param].classType);
                            break;
                        default:
                            break;
                    }
                    if (pButton->msg > UIMSG_44 &&
                        pButton->msg <=
                            UIMSG_PlayerCreationRemoveDownSkill) {  // Sellected
                                                                    // skills info
                        pY = 0;
                        if (pParty->pPlayers[pButton->msg_param].GetSkillIdxByOrder(pButton->msg - UIMSG_48) != PLAYER_SKILL_INVALID) {
                            static std::string hint_reference;
                            hint_reference = CharacterUI_GetSkillDescText(
                                pButton->msg_param,
                                pParty->pPlayers[pButton->msg_param]
                                    .GetSkillIdxByOrder(pButton->msg -
                                                        UIMSG_48));

                            popup_window.sHint = hint_reference;
                            pStr = localization->GetSkillName(
                                pParty->pPlayers[pButton->msg_param]
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
                    pFontSmallnum->CalcTextHeight(
                        sHint, popup_window.uFrameWidth, 24) +
                    2 * pFontLucida->GetHeight() + 24;
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

                std::string str = fmt::format("\f{:05}{}\f00000\n", colorTable.PaleCanary.c16(), pStr);
                popup_window.DrawTitleText(pFontCreate, 0, 0, 0, str.c_str(), 3);
                popup_window.DrawText(pFontSmallnum, {1, pFontLucida->GetHeight()}, colorTable.Black.c16(), sHint, 0, 0, 0);
            }
            break;
        }
        default:
            break;
    }
    dword_507BF0_is_there_popup_onscreen = 1;
}

int no_rightlick_in_inventory = false;  // 0050CDCC
//----- (00416196) --------------------------------------------------------
void Inventory_ItemPopupAndAlchemy() {  // needs cleaning
    ITEM_TYPE potionID;        // edx@27
    // unsigned int pOut_y;        // edx@57
    GUIWindow message_window;   // [sp+Ch] [bp-84h]@137
    unsigned int damage_level;  // [sp+8Ch] [bp-4h]@23

    if (no_rightlick_in_inventory) return;

    signed int inventoryXCoord;  // ecx@2
    int inventoryYCoord;         // eax@2

    int pY;  // [sp+3Ch] [bp-Ch]@2
    int pX;
    // Pointi cursor = pMouse->GetCursorPos();

    ItemGen *item = nullptr;

    int mousex = mouse->uMouseX;  // condense
    int mousey = mouse->uMouseY;  // condense

    static int RingsX[6] = {0x1EA, 0x21A, 0x248, 0x1EA, 0x21A, 0x248};
    static int RingsY[6] = {0x0CA, 0x0CA, 0x0CA, 0x0FA, 0x0FA, 0x0FA};

    static int glovex = 586;
    static int glovey = 88;

    static int amuletx = 493;
    static int amulety = 91;

    int slot = 32;
    ITEM_SLOT pos = ITEM_SLOT_INVALID;

    mouse->GetClickPos(&pX, &pY);
    inventoryYCoord = (pY - 17) / 32;
    inventoryXCoord = (pX - 14) / 32;
    int invMatrixIndex =
        inventoryXCoord + (14 * inventoryYCoord);  // INVETORYSLOTSWIDTH

    if (pX <= 13 || pX >= 462) {  // items out of inventory(вещи вне инвентаря)
                                  // this is for player ragdoll items??
        // popup checks if ringscreen up here

        if (!ringscreenactive()) {  // rings not displayd
            int item_pid = (render->pActiveZBuffer[pX + pY * render->GetRenderDimensions().w] & 0xFFFF) - 1;
            // zbuffer still used for paperdolls

            if (item_pid == -1) return;

            item = &pPlayers[pParty->getActiveCharacter()]->pInventoryItemList[item_pid];
            GameUI_DrawItemInfo(item);
            return;
        } else {  // rings displayed
            if (mousex < 490 || mousex > 618) return;

            if (mousey < 88 || mousey > 282) return;

            if (mousex >= amuletx && mousex <= (amuletx + slot) &&
                mousey >= amulety && mousey <= (amulety + 2 * slot)) {
                // amulet
                // pitem = pPlayers[pParty->getActiveCharacter()]->GetAmuletItem(); //9
                pos = ITEM_SLOT_AMULET;
            }

            if (mousex >= glovex && mousex <= (glovex + slot) &&
                mousey >= glovey && mousey <= (glovey + 2 * slot)) {
                // glove
                // pitem = pPlayers[pParty->getActiveCharacter()]->GetGloveItem(); //7
                pos = ITEM_SLOT_GAUTNLETS;
            }

            for (int i = 0; i < 6; ++i) {
                if (mousex >= RingsX[i] && mousex <= (RingsX[i] + slot) &&
                    mousey >= RingsY[i] && mousey <= (RingsY[i] + slot)) {
                    // ring
                    // pitem = pPlayers[pParty->getActiveCharacter()]->GetNthRingItem(i);
                    // //10+i
                    pos = RingSlot(i);
                }
            }

            if (pos != ITEM_SLOT_INVALID)
                item = pPlayers[pParty->getActiveCharacter()]->GetNthEquippedIndexItem(pos);

            if (!item) return;

            GameUI_DrawItemInfo(item);

            return;
        }
    }

    // limits check ?
    // if (inventoryYCoord >= 0 && inventoryYCoord < INVETORYSLOTSHEIGHT &&
    // inventoryXCoord >= 0 && inventoryXCoord < INVETORYSLOTSWIDTH) {

    item = pPlayers[pParty->getActiveCharacter()]->GetItemAtInventoryIndex(invMatrixIndex);

    if (!item) {  // no item
        return;
    }

    // if (item_pid == -1) //added here to avoid crash
    //    return;

    // check character condition(проверка состояния персонажа)
    if (!pPlayers[pParty->getActiveCharacter()]->CanAct()) {
        static std::string hint_reference;
        hint_reference = localization->FormatString(
            LSTR_FMT_S_IS_IN_NO_CODITION_TO_S,
            pPlayers[pParty->getActiveCharacter()]->pName.c_str(),
            localization->GetString(LSTR_IDENTIFY_ITEMS)
        );

        message_window.sHint = hint_reference;
        message_window.uFrameWidth = 384;
        message_window.uFrameHeight = 180;
        if (pX <= 320)
            message_window.uFrameX = pX + 30;
        else
            message_window.uFrameX = pX - 414;
        message_window.uFrameY = 40;
        message_window.DrawMessageBox(0);
        return;
    }

    PLAYER_SKILL_LEVEL alchemy_skill_points = pPlayers[pParty->getActiveCharacter()]->GetActualSkillLevel(PLAYER_SKILL_ALCHEMY);
    PLAYER_SKILL_MASTERY alchemy_skill_level = pPlayers[pParty->getActiveCharacter()]->GetActualSkillMastery(PLAYER_SKILL_ALCHEMY);

    if (pParty->pPickedItem.uItemID == ITEM_POTION_BOTTLE) {
        GameUI_DrawItemInfo(item);
        return;
    }

    if (pParty->pPickedItem.uItemID == ITEM_POTION_RECHARGE_ITEM) {
        if (item->uItemID < ITEM_POTION_BOTTLE ||
            item->uItemID > ITEM_POTION_REJUVENATION) {  // all potions
            if (!item->isWand()) {  // can recharge only wands
                pAudioPlayer->playUISound(SOUND_error);
                return;
            }

            int maxChargesDecreasePercent = 70 - pParty->pPickedItem.uEnchantmentType;
            if (maxChargesDecreasePercent < 0) {
                maxChargesDecreasePercent = 0;
            }
            float invMaxChargesDecrease = (100 - maxChargesDecreasePercent) * 0.01;
            item->uMaxCharges = item->uNumCharges = item->uMaxCharges * invMaxChargesDecrease;

            // Effect and sound was not present previously
            item->uAttributes |= ITEM_AURA_EFFECT_GREEN;
            ItemEnchantmentTimer = Timer::Second * 2;
            pAudioPlayer->playSpellSound(SPELL_WATER_RECHARGE_ITEM, 0, false);
            mouse->RemoveHoldingItem();
            no_rightlick_in_inventory = 1;
            return;
        }
        GameUI_DrawItemInfo(item);
        return;
    } else if (pParty->pPickedItem.uItemID == ITEM_POTION_HARDEN_ITEM) {
      if (item->uItemID < ITEM_POTION_BOTTLE ||
            item->uItemID > ITEM_POTION_REJUVENATION) {  // bottle and all potions
            if (item->IsBroken() ||  // cant harden broken items
                item->uItemID >= ITEM_ARTIFACT_PUCK ||  // cant harden artifacts
                (!item->isWeapon() && !item->isPassiveEquipment() && !item->isWand())) {
                mouse->RemoveHoldingItem();
                no_rightlick_in_inventory = true;
                return;
            }

            item->uAttributes |= ITEM_AURA_EFFECT_RED | ITEM_HARDENED;
            // Sound was missing previously
            pAudioPlayer->playSpellSound(SPELL_WATER_ENCHANT_ITEM, 0, false);

            ItemEnchantmentTimer = Timer::Second * 2;
            mouse->RemoveHoldingItem();
            no_rightlick_in_inventory = true;
            return;
        }
        GameUI_DrawItemInfo(item);
        return;
    } else if (pParty->pPickedItem.uItemID >= ITEM_POTION_FLAMING && pParty->pPickedItem.uItemID <= ITEM_POTION_SWIFT ||
             pParty->pPickedItem.uItemID == ITEM_POTION_SLAYING) {
        // different enchanting potions
        if (item->uItemID < ITEM_POTION_BOTTLE ||
            item->uItemID > ITEM_POTION_REJUVENATION) {  // all potions
            if (item->uItemID >= ITEM_BLASTER &&
                item->uItemID <= ITEM_BLASTER_RIFLE ||
                item->uItemID >= ITEM_ARTIFACT_PUCK || item->IsBroken() ||
                item->special_enchantment || item->uEnchantmentType ||
                !item->isWeapon()) {
                mouse->RemoveHoldingItem();
                no_rightlick_in_inventory = true;
                return;
            }

            GameTime effectTime = GameTime::FromMinutes(30 * pParty->pPickedItem.uEnchantmentType);
            item->UpdateTempBonus(pParty->GetPlayingTime());
            if (pParty->pPickedItem.uItemID == ITEM_POTION_SLAYING) {
                item->special_enchantment = ITEM_ENCHANTMENT_DRAGON_SLAYING;
            } else {
                static constinit IndexedArray<ITEM_ENCHANTMENT, ITEM_FIRST_ENCHANTING_POTION, ITEM_LAST_ENCHANTING_POTION> _4E2904_enchantment_by_potion_lut = {
                    {ITEM_POTION_FLAMING, ITEM_ENCHANTMENT_OF_FLAME},
                    {ITEM_POTION_FREEZING, ITEM_ENCHANTMENT_OF_FROST},
                    {ITEM_POTION_NOXIOUS, ITEM_ENCHANTMENT_OF_POISON},
                    {ITEM_POTION_SHOCKING, ITEM_ENCHANTMENT_OF_SPARKS},
                    {ITEM_POTION_SWIFT, ITEM_ENCHANTMENT_SWIFT}
                };
                item->special_enchantment = _4E2904_enchantment_by_potion_lut[pParty->pPickedItem.uItemID];
            }

            item->uExpireTime = GameTime(pParty->GetPlayingTime() + effectTime);
            // Sound was missing previously
            item->uAttributes |= ITEM_TEMP_BONUS | ITEM_AURA_EFFECT_RED;
            pAudioPlayer->playSpellSound(SPELL_WATER_ENCHANT_ITEM, 0, false);

            ItemEnchantmentTimer = Timer::Second * 2;
            mouse->RemoveHoldingItem();
            no_rightlick_in_inventory = true;
            return;
        }
        GameUI_DrawItemInfo(item);
        return;
    }

    // use reagents(применение реагентов)
    if (pParty->pPickedItem.uItemID >= ITEM_REAGENT_WIDOWSWEEP_BERRIES &&
        pParty->pPickedItem.uItemID <= ITEM_REAGENT_PHILOSOPHERS_STONE &&
        item->uItemID == ITEM_POTION_BOTTLE) {
        item->uEnchantmentType =
            alchemy_skill_points + pParty->pPickedItem.GetDamageDice();
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
        mouse->RemoveHoldingItem();
        no_rightlick_in_inventory = 1;
        if (dword_4E455C) {
            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_PotionSuccess);
            dword_4E455C = 0;
        }
        return;
    }

    // potions mixing
    if (IsPotion(pParty->pPickedItem.uItemID) && IsPotion(item->uItemID)) {
        ITEM_TYPE potionSrc1 = item->uItemID;
        ITEM_TYPE potionSrc2 = pParty->pPickedItem.uItemID;

        ITEM_TYPE potionID;
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

        // TODO(Nik-RE-dev): need to allow GetSkillMastery return PLAYER_SKILL_MASTERY_NONE
        if (!alchemy_skill_points) {
            alchemy_skill_level = PLAYER_SKILL_MASTERY_NONE;
        }

        damage_level = 0;
        if (!IsPotion(potionID)) {
            // In this case potionID represent damage level
            damage_level = std::to_underlying(potionID);
        } else {
            // potionID >= ITEM_POTION_CURE_WOUNDS && potionID <= ITEM_POTION_CURE_WEAKNESS does not require skill
            if (potionID >= ITEM_POTION_CURE_DISEASE &&
                    potionID <= ITEM_POTION_AWAKEN &&
                    alchemy_skill_level == PLAYER_SKILL_MASTERY_NONE) {
                damage_level = 1;
            }
            if (potionID >= ITEM_POTION_HASTE &&
                    potionID <= ITEM_POTION_CURE_INSANITY &&
                    alchemy_skill_level <= PLAYER_SKILL_MASTERY_NOVICE) {
                damage_level = 2;
            }
            if (potionID >= ITEM_POTION_MIGHT_BOOST &&
                    potionID <= ITEM_POTION_BODY_RESISTANCE &&
                    alchemy_skill_level <= PLAYER_SKILL_MASTERY_EXPERT) {
                damage_level = 3;
            }
            if (potionID >= ITEM_POTION_STONE_TO_FLESH &&
                    alchemy_skill_level <= PLAYER_SKILL_MASTERY_MASTER) {
                damage_level = 4;
            }
        }

        int item_pid = pPlayers[pParty->getActiveCharacter()]->GetItemListAtInventoryIndex(invMatrixIndex);
        // int pOut_x = item_pid + 1;
        // for (uint i = 0; i < 126; ++i)
        //{
        //  if (pPlayers[pParty->getActiveCharacter()]->pInventoryMatrix[i] == pOut_x)
        // {
        //    pOut_y = i;
        //   break;
        //}
        //}

        if (damage_level > 0) {
            pPlayers[pParty->getActiveCharacter()]->RemoveItemAtInventoryIndex(invMatrixIndex);  // pOut_y); ?? quickfix needs checking

            if (damage_level == 1) {
                pPlayers[pParty->getActiveCharacter()]->ReceiveDamage(grng->random(11) + 10, DMGT_FIRE);
            } else if (damage_level == 2) {
                pPlayers[pParty->getActiveCharacter()]->ReceiveDamage(grng->random(71) + 30, DMGT_FIRE);
                pPlayers[pParty->getActiveCharacter()]->ItemsPotionDmgBreak(1);  // break 1
            } else if (damage_level == 3) {
                pPlayers[pParty->getActiveCharacter()]->ReceiveDamage(grng->random(201) + 50, DMGT_FIRE);
                pPlayers[pParty->getActiveCharacter()]->ItemsPotionDmgBreak(5);  // break 5
            } else if (damage_level >= 4) {
                pPlayers[pParty->getActiveCharacter()]->SetCondition(Condition_Eradicated, 0);
                pPlayers[pParty->getActiveCharacter()]->ItemsPotionDmgBreak(0);  // break everything
            }

            pAudioPlayer->playUISound(SOUND_fireBall);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);

            int _viewPitch, _viewYaw, rot_z;
            Vec3i::rotate(64, pParty->_viewYaw, pParty->_viewPitch, pParty->vPosition + Vec3i(0, 0, pParty->sEyelevel), &_viewPitch, &_viewYaw, &rot_z);
            SpriteObject::dropItemAt(SPRITE_SPELL_FIRE_FIREBALL_IMPACT, {_viewPitch, _viewYaw, rot_z}, 0);
            if (dword_4E455C) {
                if (pPlayers[pParty->getActiveCharacter()]->CanAct()) {
                    pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_PotionExplode);
                }
                GameUI_SetStatusBar(LSTR_OOPS);
                dword_4E455C = 0;
            }
            mouse->RemoveHoldingItem();
            no_rightlick_in_inventory = 1;
            return;
        } else {  // if ( damage_level == 0 )
            if (item->uItemID == ITEM_POTION_CATALYST && pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST) {
                // Both potions are catalyst: power is maximum of two
                item->uEnchantmentType = std::max(item->uEnchantmentType, pParty->pPickedItem.uEnchantmentType);
            } else if (item->uItemID == ITEM_POTION_CATALYST || pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST) {
                // One of the potion is catalyst: power of potion is replaced by power of catalyst
                if (item->uItemID == ITEM_POTION_CATALYST) {
                    item->uItemID = pParty->pPickedItem.uItemID;
                } else {
                    item->uEnchantmentType = pParty->pPickedItem.uEnchantmentType;
                }
            } else {
                item->uItemID = potionID;
                item->uEnchantmentType = (pParty->pPickedItem.uEnchantmentType + item->uEnchantmentType) / 2;
                // Can be zero even for valid potion combination when resulting potion is of lower grade than it's components
                // Example: "Cure Paralysis(white) + Cure Wounds(red) = Cure Wounds(red)"
                if (pItemTable->potionNotes[potionSrc1][potionSrc2] != 0) {
                    pPlayers[pParty->getActiveCharacter()]->SetVariable(VAR_AutoNotes, pItemTable->potionNotes[potionSrc1][potionSrc2]);
                }
            }
            int bottle = pPlayers[pParty->getActiveCharacter()]->AddItem(-1, ITEM_POTION_BOTTLE);
            if (bottle) {
                pPlayers[pParty->getActiveCharacter()]->pOwnItems[bottle - 1].uAttributes = ITEM_IDENTIFIED;
            }
            if (!(pItemTable->pItems[item->uItemID].uItemID_Rep_St)) {
                item->uAttributes |= ITEM_IDENTIFIED;
            }
            if (!dword_4E455C) {
                mouse->RemoveHoldingItem();
                no_rightlick_in_inventory = 1;
                return;
            }
            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_PotionSuccess);
            dword_4E455C = 0;
            mouse->RemoveHoldingItem();
            no_rightlick_in_inventory = 1;
            return;
        }
    }

    GameUI_DrawItemInfo(item);
    return;
}

//----- (0045828B) --------------------------------------------------------
unsigned int GetSpellColor(signed int a1) {
    if (a1 == 0) return colorTable.Black.c16();
    if (a1 < 12) return colorTable.DarkOrange.c16();
    if (a1 < 23) return colorTable.Anakiwa.c16();
    if (a1 < 34) return colorTable.AzureRadiance.c16();
    if (a1 < 45) return colorTable.Gray.c16();
    if (a1 < 56) return colorTable.Mercury.c16();
    if (a1 < 67) return colorTable.PurplePink.c16();
    if (a1 < 78) return colorTable.FlushOrange.c16();
    if (a1 < 89) return colorTable.PaleCanary.c16();
    if (a1 < 100)
        return colorTable.MoonRaker.c16();
    else
        __debugbreak();

    logger->warning("No color returned - GetSpellColor!");
    return 0;
}

//----- (004B46F8) --------------------------------------------------------
uint64_t GetExperienceRequiredForLevel(int level) {
    int effectiveLevel = 0;
    for (int i = 0; i < level; ++i)
        effectiveLevel += i + 1;
    return (uint64_t)(1000 * effectiveLevel);
}
