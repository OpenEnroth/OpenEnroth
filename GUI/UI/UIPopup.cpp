#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <crtdbg.h>
#include <stdlib.h>

#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"

#include "IO/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/Books/MapBook.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"

#include "GUI/UI/UICharacter.h"

#include "Media/Audio/AudioPlayer.h"

using EngineIoc = Engine_::IocContainer;

Mouse *pMouse = EngineIoc::ResolveMouse();

Texture *parchment = nullptr;
Image *messagebox_corner_x = nullptr;       // 5076AC
Image *messagebox_corner_y = nullptr;       // 5076B4
Image *messagebox_corner_z = nullptr;       // 5076A8
Image *messagebox_corner_w = nullptr;       // 5076B0
Image *messagebox_border_top = nullptr;     // 507698
Image *messagebox_border_bottom = nullptr;  // 5076A4
Image *messagebox_border_left = nullptr;    // 50769C
Image *messagebox_border_right = nullptr;   // 5076A0

//----- (004179BC) --------------------------------------------------------
void CharacterUI_DrawTooltip(const char *title, String &content) {
    GUIWindow popup_window;  // [sp+Ch] [bp-5Ch]@1

    Point pt = pMouse->GetCursorPos();

    memset(&popup_window, 0, 0x54u);
    popup_window.uFrameWidth = 384;
    popup_window.uFrameHeight = 256;
    popup_window.uFrameX = 128;
    popup_window.uFrameY = pt.y + 30;
    popup_window.uFrameHeight =
        pFontSmallnum->CalcTextHeight(content, popup_window.uFrameWidth, 24) +
        2 * pFontLucida->GetHeight() + 24;
    popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
    popup_window.uFrameW = popup_window.uFrameY + popup_window.uFrameHeight - 1;
    popup_window.DrawMessageBox(0);

    popup_window.uFrameX += 12;
    popup_window.uFrameWidth -= 24;
    popup_window.uFrameY += 12;
    popup_window.uFrameHeight -= 12;
    popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
    popup_window.uFrameW = popup_window.uFrameY + popup_window.uFrameHeight - 1;

    auto colored_title = StringPrintf(
        "\f%05d%s\f00000\n", ui_character_tooltip_header_default_color, title);
    popup_window.DrawTitleText(pFontCreate, 0, 0, 0, colored_title, 3);
    popup_window.DrawText(
        pFontSmallnum, 1, pFontLucida->GetHeight(), 0, content, 0, 0,
        0);  // popup_window.uFrameY + popup_window.uFrameHeight
}

void CharacterUI_DrawTooltip(const char *title, const char *content) {
    CharacterUI_DrawTooltip(title, String(content));
}

//----- (004151D9) --------------------------------------------------------
void DrawPopupWindow(unsigned int uX, unsigned int uY, unsigned int uWidth,
                     unsigned int uHeight) {
    unsigned int uNumTiles;  // [sp+2Ch] [bp-Ch]@6
    int coord_x;             // [sp+2Ch] [bp-Ch]@3
    int coord_y;             // [sp+34h] [bp-4h]@5

    if (!parchment) return;

    render->SetUIClipRect(uX, uY, uX + uWidth, uY + uHeight);

    unsigned int parchment_width = parchment->GetWidth();
    unsigned int parchment_height = parchment->GetHeight();

    uNumTiles = uWidth / parchment_width;
    if (uWidth % parchment_width) ++uNumTiles;
    coord_y = uY;
    for (uint j = 0; j <= uHeight / parchment_height; j++) {
        coord_x = uX - parchment_width;
        for (uint i = uNumTiles + 1; i; --i) {
            coord_x += parchment_width;
            render->DrawTextureNew(coord_x / 640.0f, coord_y / 480.0f,
                                   parchment);
        }
        coord_y += parchment_height;
    }

    render->DrawTextureAlphaNew(uX / 640.0f, uY / 480.0f, messagebox_corner_x);
    render->DrawTextureAlphaNew(
        uX / 640.0f, (uY + uHeight - messagebox_corner_y->GetHeight()) / 480.0f,
        messagebox_corner_y);
    render->DrawTextureAlphaNew(
        (uX + uWidth - messagebox_corner_z->GetWidth()) / 640.0f, uY / 480.0f,
        messagebox_corner_z);
    render->DrawTextureAlphaNew(
        (uX + uWidth - messagebox_corner_z->GetWidth()) / 640.0f,
        (uY + uHeight - messagebox_corner_y->GetHeight()) / 480.0f,
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
            render->DrawTextureAlphaNew(x / 640.0f, uY / 480.0f,
                                        messagebox_border_top);
            render->DrawTextureAlphaNew(
                x / 640.0f,
                (uY + uHeight - messagebox_border_bottom->GetHeight()) / 480.0f,
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
             y += messagebox_border_top->GetHeight()) {
            render->DrawTextureAlphaNew(uX / 640.0f, y / 480.0f,
                                        messagebox_border_left);
            render->DrawTextureAlphaNew(
                (uX + uWidth - messagebox_border_right->GetWidth() - 1) /
                    640.0f,
                y / 480.0f, messagebox_border_right);
        }
    }
    render->ResetUIClipRect();
}

//----- (0041D895) --------------------------------------------------------
void GameUI_DrawItemInfo(struct ItemGen *inspect_item) {
    unsigned int v2;     // eax@3
    const char *v28;     // edi@69
    int v34;             // esi@81
    char out_text[300];  // [sp+8h] [bp-270h]@40
    SummonedItem v67;
    int v77;                    // [sp+200h] [bp-78h]@12
    int v78;                    // [sp+204h] [bp-74h]@5
    GUIWindow iteminfo_window;  // [sp+208h] [bp-70h]@2
    int v81;                    // [sp+264h] [bp-14h]@5
    PlayerSpeech v83;           // [sp+26Ch] [bp-Ch]@18
    char *v84;
    int v85;
    char *Str;  // [sp+270h] [bp-8h]@65

    int r_mask = 0xF800;
    int g_mask = 0x7E0;
    int b_mask = 0x1F;

    if (!inspect_item->uItemID) return;

    auto inspect_item_image =
        assets->GetImage_ColorKey(inspect_item->GetIconName(), 0x7FF);

    iteminfo_window.sHint.clear();
    iteminfo_window.uFrameWidth = 384;
    iteminfo_window.uFrameHeight = 180;
    iteminfo_window.uFrameY = 40;

    Point pt = pMouse->GetCursorPos();
    if (pt.x <= 320)
        v2 = pt.x + 30;
    else
        v2 = pt.x - iteminfo_window.uFrameWidth - 30;
    iteminfo_window.uFrameX = v2;
    v78 = 100 - inspect_item_image->GetWidth();
    v81 = 144 - inspect_item_image->GetHeight();
    if (v78 > 0) v78 = v78 / 2;
    if (v81 <= 0)
        v81 = 0;
    else
        v81 = v81 / 2;
    if (!pItemsTable->pItems[inspect_item->uItemID].uItemID_Rep_St)
        inspect_item->SetIdentified();
    v77 = 0;
    if (inspect_item->GetItemEquipType() == EQUIP_GOLD)
        v77 = inspect_item->special_enchantment;
    if (uActiveCharacter) {
        // try to identify
        if (!inspect_item->IsIdentified()) {
            if (pPlayers[uActiveCharacter]->CanIdentify(inspect_item) == 1)
                inspect_item->SetIdentified();
            v83 = SPEECH_9;
            if (!inspect_item->IsIdentified()) {
                GameUI_StatusBar_OnEvent(
                    localization->GetString(446));  // Identify Failed
            } else {
                v83 = SPEECH_8;
                if (inspect_item->GetValue() <
                    100 * (pPlayers[uActiveCharacter]->uLevel + 5))
                    v83 = SPEECH_7;
            }
            if (dword_4E455C) {
                pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)(int)v83,
                                                      0);
                dword_4E455C = 0;
            }
        }
        inspect_item->UpdateTempBonus(pParty->GetPlayingTime());
        if (inspect_item->IsBroken()) {
            if (pPlayers[uActiveCharacter]->CanRepair(inspect_item) == 1)
                inspect_item->uAttributes =
                    inspect_item->uAttributes & 0xFFFFFFFD | 1;
            v83 = SPEECH_11;
            if (!inspect_item->IsBroken())
                v83 = SPEECH_10;
            else
                GameUI_StatusBar_OnEvent(
                    localization->GetString(448));  // Repair Failed
            if (dword_4E455C) {
                pPlayers[uActiveCharacter]->PlaySound(v83, 0);
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
            (iteminfo_window.uFrameX + v78) / 640.0f,
            (v81 + iteminfo_window.uFrameY + 30) / 480.0f, inspect_item_image);

        iteminfo_window.DrawTitleText(
            pFontArrus, 0, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu),
            inspect_item->GetDisplayName().c_str(), 3);
        iteminfo_window.DrawTitleText(
            pFontArrus, 0x64u,
            ((signed int)iteminfo_window.uFrameHeight >> 1) -
                pFontArrus->CalcTextHeight(localization->GetString(32),
                                           iteminfo_window.uFrameWidth, 0) /
                    2,
            Color16(0xFFu, 0x19u, 0x19u), localization->GetString(32),
            3);  // "Broken Item"
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
        render->DrawTextureAlphaNew(
            (iteminfo_window.uFrameX + v78) / 640.0f,
            (v81 + iteminfo_window.uFrameY + 30) / 480.0f, inspect_item_image);
        iteminfo_window.DrawTitleText(
            pFontArrus, 0, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu),
            pItemsTable->pItems[inspect_item->uItemID].pUnidentifiedName, 3);
        iteminfo_window.DrawTitleText(
            pFontArrus, 0x64u,
            ((int)iteminfo_window.uFrameHeight >> 1) -
                pFontArrus->CalcTextHeight(localization->GetString(232),
                                           iteminfo_window.uFrameWidth, 0) /
                    2,
            Color16(0xFFu, 0x19u, 0x19u), localization->GetString(232),
            3);  // Not Identified
        render->ResetUIClipRect();

        if (inspect_item_image) {
            inspect_item_image->Release();
            inspect_item_image = nullptr;
        }
        return;
    }

    String str = localization->FormatString(
        463, pItemsTable->pItems[inspect_item->uItemID]
                 .pUnidentifiedName);  // "Type: %s"

    strcpy(out_text, str.c_str());
    out_text[100] = 0;
    out_text[200] = 0;

    switch (inspect_item->GetItemEquipType()) {
        case EQUIP_SINGLE_HANDED:
        case EQUIP_TWO_HANDED: {
            sprintf(out_text + 100, "%s: +%d   %s: %dd%d",
                    localization->GetString(18),  // Attack
                    (int)inspect_item->GetDamageMod(),
                    localization->GetString(53),
                    (int)inspect_item->GetDamageDice(),
                    (int)inspect_item->GetDamageRoll());  // "Damage"
            if (inspect_item->GetDamageMod()) {
                char mod[16];
                sprintf(mod, "+%d", (int)inspect_item->GetDamageMod());
                strcat(out_text + 100, mod);
            }
            break;
        }

        case EQUIP_BOW:
            sprintf(out_text + 100, "%s: +%d   %s: %dd%d",
                    localization->GetString(203),  // "Shoot"
                    (int)inspect_item->GetDamageMod(),
                    localization->GetString(53),  // "Damage"
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
            if (inspect_item->GetDamageDice())  // "Armor"
                sprintf(out_text + 100, "%s: +%d", localization->GetString(11),
                        inspect_item->GetDamageDice() +
                            inspect_item->GetDamageMod());
            break;
    }

    if (!v77) {
        if (inspect_item->GetItemEquipType() ==
            EQUIP_POTION) {  // this is CORRECT! do not move to switch!
            if (inspect_item->uEnchantmentType)
                sprintf(out_text + 200, "%s: %d", localization->GetString(449),
                        inspect_item->uEnchantmentType);  // "Power"
        } else if (inspect_item->GetItemEquipType() == EQUIP_REAGENT) {
            sprintf(out_text + 200, "%s: %d", localization->GetString(449),
                inspect_item->GetDamageDice());  // "Power"
        } else if (inspect_item->uEnchantmentType) {
            sprintf(
                out_text + 200, "%s: %s +%d", localization->GetString(210),
                pItemsTable->pEnchantments[inspect_item->uEnchantmentType - 1]
                .pBonusStat,
                inspect_item->m_enchantmentStrength);  // "Special"
        } else if (inspect_item->special_enchantment) {
            sprintf(
                out_text + 200, "%s: %s", localization->GetString(210),
                pItemsTable
                ->pSpecialEnchantments[inspect_item->special_enchantment -
                1]
                .pBonusStatement);
        } else if (inspect_item->uNumCharges) {
            sprintf(out_text + 200, "%s: %lu", localization->GetString(464),
                inspect_item->uNumCharges);  // "Charges"
        }
    }
    iteminfo_window.uFrameWidth -= 12;
    iteminfo_window.uFrameZ =
        iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
    iteminfo_window.uFrameW =
        iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
    Str = (char *)(3 * (pFontArrus->GetHeight() + 8));
    v84 = &out_text[0];
    for (uint i = 1; i <= 3; i++) {
        if (*v84)
            Str += pFontComic->CalcTextHeight(v84, iteminfo_window.uFrameWidth,
                                              100) +
                   3;
        v84 += 100;
    }
    v28 = pItemsTable->pItems[inspect_item->uItemID].pDescription;
    if (*v28)
        Str += pFontSmallnum->CalcTextHeight(
            pItemsTable->pItems[inspect_item->uItemID].pDescription,
            iteminfo_window.uFrameWidth, 100);
    iteminfo_window.uFrameHeight =
        assets->GetImage_ColorKey(inspect_item->GetIconName(), 0x7FF)
            ->GetHeight() +
        v81 + 54;
    if ((signed int)Str > (signed int)iteminfo_window.uFrameHeight)
        iteminfo_window.uFrameHeight = (unsigned int)Str;
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
    render->DrawTextureAlphaNew((iteminfo_window.uFrameX + v78) / 640.0f,
                                (iteminfo_window.uFrameY +
                                 (signed int)(iteminfo_window.uFrameHeight -
                                              inspect_item_image->GetHeight()) /
                                     2) /
                                    480.0f,
                                inspect_item_image);

    v34 = (int)(v85 + 35);
    Str = out_text;
    for (uint i = 1; i <= 3; i++) {
        if (*Str) {
            iteminfo_window.DrawText(pFontComic, 100, v34, 0, Str, 0, 0, 0);
            v34 += pFontComic->CalcTextHeight(Str, iteminfo_window.uFrameWidth,
                                              100, 0) +
                   3;
        }
        Str += 100;
    }
    v28 = pItemsTable->pItems[inspect_item->uItemID].pDescription;
    if (*v28)
        iteminfo_window.DrawText(pFontSmallnum, 100, v34, 0, v28, 0, 0, 0);
    iteminfo_window.uFrameX += 12;
    iteminfo_window.uFrameWidth -= 24;
    iteminfo_window.DrawTitleText(pFontArrus, 0, 0xCu,
                                  Color16(0xFFu, 0xFFu, 0x9Bu),
                                  inspect_item->GetIdentifiedName(), 3);
    iteminfo_window.uFrameWidth += 24;
    iteminfo_window.uFrameX -= 12;
    if (v77) {
        auto txt = StringPrintf("%s: %lu", localization->GetString(465),
                                v77);  // Value
        iteminfo_window.DrawText(
            pFontComic, 100,
            iteminfo_window.uFrameHeight - pFontComic->GetHeight(), 0, txt, 0,
            0, 0);
        render->ResetUIClipRect();
    } else {
        if ((inspect_item->uAttributes & ITEM_TEMP_BONUS) &&
            (inspect_item->special_enchantment ||
             inspect_item->uEnchantmentType)) {
            v67.Initialize(inspect_item->expirte_time -
                           pParty->GetPlayingTime());

            String txt4 = "Duration:";
            Str = (char *)(v67.field_18_expire_year - game_starting_year);
            if (v67.field_18_expire_year != 1168) {
                txt4 += StringPrintf(
                    " %d:yr", v67.field_18_expire_year - game_starting_year);
            }

            if ((((v67.field_14_exprie_month || Str) &&
                      ((txt4 +=
                        StringPrintf(" %d:mo", v67.field_14_exprie_month),
                        v67.field_14_exprie_month) ||
                       Str) ||
                  v67.field_C_expire_day) &&
                     ((txt4 += StringPrintf(" %d:dy", v67.field_C_expire_day),
                       v67.field_14_exprie_month) ||
                      Str || v67.field_C_expire_day) ||
                 v67.field_8_expire_hour) &&
                    ((txt4 += StringPrintf(" %d:hr", v67.field_8_expire_hour),
                      v67.field_14_exprie_month) ||
                     Str || v67.field_C_expire_day ||
                     v67.field_8_expire_hour) ||
                v67.field_4_expire_minute) {
                txt4 += StringPrintf(" %d:mn", v67.field_4_expire_minute);
            }
            iteminfo_window.DrawText(
                pFontComic, 100,
                iteminfo_window.uFrameHeight - 2 * pFontComic->GetHeight(), 0,
                txt4.data(), 0, 0, 0);
        }

        auto txt2 = StringPrintf("%s: %lu", localization->GetString(465),
                                 inspect_item->GetValue());
        iteminfo_window.DrawText(
            pFontComic, 100,
            iteminfo_window.uFrameHeight - pFontComic->GetHeight(), 0,
            txt2.data(), 0, 0, 0);

        String txt3;
        if (inspect_item->uAttributes & ITEM_STOLEN) {
            txt3 = localization->GetString(187);  // "Stolen"
        } else {
            if (!(inspect_item->uAttributes & ITEM_HARDENED)) {
                render->ResetUIClipRect();
                return;
            }
            txt3 = localization->GetString(651);  // "Hardened"
        }

        int tempatt = (inspect_item->uAttributes & 0xFFFF0000) | r_mask;

        iteminfo_window.DrawText(
            pFontComic, pFontComic->GetLineWidth(txt2.data()) + 132,
            iteminfo_window.uFrameHeight - pFontComic->GetHeight(), tempatt,
            txt3, 0, 0, 0);
        render->ResetUIClipRect();
    }
}

void MonsterPopup_Draw(unsigned int uActorID, GUIWindow *pWindow) {
    bool monster_full_informations = false;
    static Actor pMonsterInfoUI_Doll;
    if (!uActiveCharacter) {
        uActiveCharacter = 1;
    }

    int Popup_Y_Offset =
        monster_popup_y_offsets[(pActors[uActorID].pMonsterInfo.uID - 1) / 3] -
        40;

    uint16_t v9 = 0;
    if (pActors[uActorID].pMonsterInfo.uID ==
        pMonsterInfoUI_Doll.pMonsterInfo.uID) {
        v9 = pMonsterInfoUI_Doll.uCurrentActionLength;
    } else {
        memcpy(&pMonsterInfoUI_Doll, &pActors[uActorID],
               sizeof(pMonsterInfoUI_Doll));
        pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Bored;
        pMonsterInfoUI_Doll.uCurrentActionTime = 0;
        v9 = rand() % 256 + 128;
        pMonsterInfoUI_Doll.uCurrentActionLength = v9;
    }

    if (pMonsterInfoUI_Doll.uCurrentActionTime > v9) {
        pMonsterInfoUI_Doll.uCurrentActionTime = 0;
        if (pMonsterInfoUI_Doll.uCurrentActionAnimation == ANIM_Bored ||
            pMonsterInfoUI_Doll.uCurrentActionAnimation == ANIM_AtkMelee) {
            pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Standing;
            pMonsterInfoUI_Doll.uCurrentActionLength = rand() % 128 + 128;
        } else {
            // rand();
            pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Bored;
            if ((pMonsterInfoUI_Doll.pMonsterInfo.uID < 115 ||
                 pMonsterInfoUI_Doll.pMonsterInfo.uID > 186) &&
                (pMonsterInfoUI_Doll.pMonsterInfo.uID < 232 ||
                 pMonsterInfoUI_Doll.pMonsterInfo.uID > 249) &&
                rand() % 30 < 100)
                pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_AtkMelee;
            pMonsterInfoUI_Doll.uCurrentActionLength =
                8 *
                pSpriteFrameTable
                    ->pSpriteSFrames[pActors[uActorID].pSpriteIDs
                                         [(signed __int16)pMonsterInfoUI_Doll
                                              .uCurrentActionAnimation]]
                    .uAnimLength;
        }
    }

    Rect doll_rect = {pWindow->uFrameX + 13, pWindow->uFrameY + 52,
                      (pWindow->uFrameX + 13) + 128,
                      (pWindow->uFrameY + 52) + 128};
    {
        SpriteFrame *Portrait_Sprite = pSpriteFrameTable->GetFrame(
            pActors[uActorID]
                .pSpriteIDs[pMonsterInfoUI_Doll.uCurrentActionAnimation],
            pMonsterInfoUI_Doll.uCurrentActionTime);

        // Draw portrait border
        render->ResetUIClipRect();
        render->FillRectFast(doll_rect.x, doll_rect.y, 128, 128, Color16(0, 0, 0));
        render->RasterLine2D(
            doll_rect.x - 1, doll_rect.y - 1, doll_rect.z + 1, doll_rect.y - 1,
            Color16(0xE1u, 255, 0x9Bu));  // горизонтальная верхняя линия
        render->RasterLine2D(
            doll_rect.x - 1, doll_rect.w + 1, doll_rect.x - 1, doll_rect.y - 1,
            Color16(0xE1u, 255, 0x9Bu));  // горизонтальная нижняя линия
        render->RasterLine2D(
            doll_rect.z + 1, doll_rect.w + 1, doll_rect.x - 1, doll_rect.w + 1,
            Color16(0xE1u, 255, 0x9Bu));  // левая вертикальная линия
        render->RasterLine2D(
            doll_rect.z + 1, doll_rect.y - 1, doll_rect.z + 1, doll_rect.w + 1,
            Color16(0xE1u, 255, 0x9Bu));  // правая вертикальная линия

        // Draw portrait
        render->DrawMonsterPortrait(doll_rect, Portrait_Sprite, Popup_Y_Offset);
    }

    // Draw name and profession
    String str;
    if (pActors[uActorID].sNPC_ID) {
        if (GetNPCData(pActors[uActorID].sNPC_ID)->uProfession)
            str = localization->FormatString(
                429, GetNPCData(pActors[uActorID].sNPC_ID)->pName,
                localization->GetNpcProfessionName(
                    GetNPCData(pActors[uActorID].sNPC_ID)
                        ->uProfession));  // "%s the %s"   /   ^Pi[%s] %s
        else
            str = GetNPCData(pActors[uActorID].sNPC_ID)->pName;
    } else {
        if (pActors[uActorID].dword_000334_unique_name)
            str =
                pMonsterStats
                    ->pPlaceStrings[pActors[uActorID].dword_000334_unique_name];
        else
            str =
                pMonsterStats->pInfos[pActors[uActorID].pMonsterInfo.uID].pName;
    }
    pWindow->DrawTitleText(pFontComic, 0, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu),
                           str, 3);

    // health bar
    Actor::DrawHealthBar(&pActors[uActorID], pWindow);

    bool normal_level = false;
    bool expert_level = false;
    bool master_level = false;
    bool grandmaster_level = false;
    bool for_effects = false;

    int skill_points = 0;
    unsigned int skill_level = 0;

    pMonsterInfoUI_Doll.uCurrentActionTime += pMiscTimer->uTimeElapsed;
    if (pPlayers[uActiveCharacter]->GetActualSkillLevel(
            PLAYER_SKILL_MONSTER_ID)) {
        skill_points =
            (unsigned __int8)pPlayers[uActiveCharacter]->GetActualSkillLevel(
                PLAYER_SKILL_MONSTER_ID);
        skill_level = pPlayers[uActiveCharacter]->GetActualSkillMastery(
                          PLAYER_SKILL_MONSTER_ID) -
                      1;
        if (skill_level == 0) {  // (normal)
            if (skill_points + 10 >= pActors[uActorID].pMonsterInfo.uLevel)
                normal_level = 1;
        } else if (skill_level == 1) {  // (expert)
            if (2 * skill_points + 10 >=
                pActors[uActorID].pMonsterInfo.uLevel) {
                normal_level = 1;
                expert_level = 1;
            }
        } else if (skill_level == 2) {  // (master)
            if (3 * skill_points + 10 >=
                pActors[uActorID].pMonsterInfo.uLevel) {
                normal_level = 1;
                expert_level = 1;
                master_level = 1;
            }
        } else if (skill_level == 3) {  // grandmaster
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
        pPlayers[uActiveCharacter]->GetActualSkillLevel(
            PLAYER_SKILL_MONSTER_ID)) {
        if (normal_level | expert_level | master_level | grandmaster_level) {
            if (pActors[uActorID].pMonsterInfo.uLevel >=
                pPlayers[uActiveCharacter]->uLevel - 5)
                speech = SPEECH_IDENTIFY_MONSTER_STRONGER;
            else
                speech = SPEECH_IDENTIFY_MONSTER_WEAKER;
        } else {
            speech = SPEECH_IDENTIFY_MONSTER_106;
        }
        pPlayers[uActiveCharacter]->PlaySound(speech, 0);
    }

    if ((signed int)(pParty->pPlayers[uActiveCharacter - 1]
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
    pWindow->DrawText(pFontSmallnum, 12, 196, Color16(0xE1u, 255, 0x9Bu),
                      localization->GetString(631), 0, 0, 0);  // Effects
    if (!for_effects && false) {
        pWindow->DrawText(pFontSmallnum, 28, pFontSmallnum->GetHeight() + 193,
                          Color16(0xE1u, 255, 0x9Bu),
                          localization->GetString(630), 0, 0, 0);  //?
    } else {
        pText = "";
        pTextHeight = pFontSmallnum->GetHeight() + 193;
        for (uint i = 1; i <= 21; ++i) {
            if (pActors[uActorID].pActorBuffs[i].Active()) {
                switch (i) {
                    case ACTOR_BUFF_CHARM:
                        pTextColorID = 60;
                        pText = localization->GetString(591);  // Charmed
                        break;
                    case ACTOR_BUFF_SUMMONED:
                        pTextColorID = 82;
                        pText = localization->GetString(649);  // Summoned
                        break;
                    case ACTOR_BUFF_SHRINK:
                        pTextColorID = 92;
                        pText = localization->GetString(592);  // Shrunk
                        break;
                    case ACTOR_BUFF_AFRAID:
                        pTextColorID = 63;
                        pText = localization->GetString(4);  // Afraid
                        break;
                    case ACTOR_BUFF_STONED:
                        pText = localization->GetString(220);  // Stoned
                        pTextColorID = 81;
                        break;
                    case ACTOR_BUFF_PARALYZED:
                        pText = localization->GetString(162);  // Paralyzed
                        pTextColorID = 81;
                        break;
                    case ACTOR_BUFF_SLOWED:
                        pText = localization->GetString(593);  // Slowed
                        pTextColorID = 35;
                        break;
                    case ACTOR_BUFF_BERSERK:
                        pText = localization->GetString(608);  // Berserk
                        pTextColorID = 62;
                        break;
                    case ACTOR_BUFF_SOMETHING_THAT_HALVES_AC:
                    case ACTOR_BUFF_MASS_DISTORTION:
                        pText = "";
                        pTextColorID = 0;
                        continue;
                    case ACTOR_BUFF_FATE:
                        pTextColorID = 47;
                        pText = localization->GetString(221);  // Fate
                        break;
                    case ACTOR_BUFF_ENSLAVED:
                        pTextColorID = 66;
                        pText = localization->GetString(607);  // Enslaved
                        break;
                    case ACTOR_BUFF_DAY_OF_PROTECTION:
                        pTextColorID = 85;
                        pText =
                            localization->GetString(610);  // Day of Protection
                        break;
                    case ACTOR_BUFF_HOUR_OF_POWER:
                        pTextColorID = 86;
                        pText = localization->GetString(609);  // Hour of Power
                        break;
                    case ACTOR_BUFF_SHIELD:
                        pTextColorID = 17;
                        pText = localization->GetString(279);  // Shield
                        break;
                    case ACTOR_BUFF_STONESKIN:
                        pTextColorID = 38;
                        pText = localization->GetString(442);  // Stoneskin
                        break;
                    case ACTOR_BUFF_BLESS:
                        pTextColorID = 46;
                        pText = localization->GetString(443);  // Bless
                        break;
                    case ACTOR_BUFF_HEROISM:
                        pTextColorID = 51;
                        pText = localization->GetString(440);  // Heroism
                        break;
                    case ACTOR_BUFF_HASTE:
                        pTextColorID = 5;
                        pText = localization->GetString(441);  // Haste
                        break;
                    case ACTOR_BUFF_PAIN_REFLECTION:
                        pTextColorID = 95;
                        pText =
                            localization->GetString(229);  // Pain Reflection
                        break;
                    case ACTOR_BUFF_PAIN_HAMMERHANDS:
                        pTextColorID = 73;
                        pText = localization->GetString(228);  // Hammerhands
                        break;
                    default:
                        pText = "";
                        break;
                }
                if (_stricmp(pText, "")) {
                    pWindow->DrawText(pFontSmallnum, 28, pTextHeight,
                                      GetSpellColor(pTextColorID), pText, 0, 0,
                                      0);
                    pTextHeight =
                        pTextHeight + *(char *)((int)pFontSmallnum + 5) - 3;
                }
            }
        }
        if (!_stricmp(pText, ""))
            pWindow->DrawText(pFontSmallnum, 28, pTextHeight,
                              Color16(0xE1u, 255, 0x9Bu),
                              localization->GetString(153), 0, 0, 0);  // Нет
    }

    String txt2;
    if (normal_level) {
        auto str =
            StringPrintf("%s\f%05u\t100%d\n", localization->GetString(108), 0,
                         pActors[uActorID].pMonsterInfo.uHP);
        pWindow->DrawText(pFontSmallnum, 150, (int)doll_rect.y,
                          Color16(0xE1u, 255, 0x9Bu), str, 0, 0, 0);
        pTextHeight = doll_rect.y + pFontSmallnum->GetHeight() - 3;
        txt2 = StringPrintf("%s\f%05u\t100%d\n", localization->GetString(12), 0,
                            pActors[uActorID].pMonsterInfo.uAC);  // Armor Class
    } else {
        auto str = StringPrintf(
            "%s\f%05u\t100%s\n", localization->GetString(108), 0,
            localization->GetString(
                630));  //?   - [630] actually displays a question mark
        pWindow->DrawText(pFontSmallnum, 150, (int)doll_rect.y,
                          Color16(0xE1u, 255, 0x9Bu), str, 0, 0, 0);
        pTextHeight = doll_rect.y + pFontSmallnum->GetHeight() - 3;
        txt2 = StringPrintf(
            "%s\f%05u\t100%s\n", localization->GetString(12), 0,
            localization->GetString(
                630));  //?   - [630] actually displays a question mark
    }
    pWindow->DrawText(pFontSmallnum, 150, pTextHeight,
                      Color16(0xE1u, 255, 0x9Bu), txt2, 0, 0, 0);
    pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 6 +
                  pFontSmallnum->GetHeight();

    const char *content[11] = {0};
    content[0] = localization->GetSpellSchoolName(0);
    content[1] = localization->GetSpellSchoolName(1);
    content[2] = localization->GetSpellSchoolName(2);
    content[3] = localization->GetSpellSchoolName(3);
    content[4] = localization->GetString(624);  // Physical
    content[5] = localization->GetString(138);  // Magic
    content[6] = localization->GetSpellSchoolName(5);
    content[7] = localization->GetSpellSchoolName(4);
    content[8] = localization->GetSpellSchoolName(6);
    content[9] = localization->GetSpellSchoolName(7);
    content[10] = localization->GetSpellSchoolName(8);

    String txt4;
    if (expert_level) {
        auto txt3 = StringPrintf(
            "%s\f%05u\t080%s\n", localization->GetString(18), 0,
            content[pActors[uActorID].pMonsterInfo.uAttack1Type]);  // Attack
        pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight,
                          Color16(0xE1u, 255, 0x9Bu), txt3, 0, 0, 0);

        pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        if (pActors[uActorID].pMonsterInfo.uAttack1DamageBonus)
            txt4 = StringPrintf(
                "%s\f%05u\t080%dd%d+%d\n", localization->GetString(53), 0,
                pActors[uActorID].pMonsterInfo.uAttack1DamageDiceRolls,
                pActors[uActorID].pMonsterInfo.uAttack1DamageDiceSides,
                pActors[uActorID].pMonsterInfo.uAttack1DamageBonus);
        else
            txt4 = StringPrintf(
                "%s\f%05u\t080%dd%d\n", localization->GetString(53), 0,
                pActors[uActorID].pMonsterInfo.uAttack1DamageDiceRolls,
                pActors[uActorID].pMonsterInfo.uAttack1DamageDiceSides);
    } else {
        auto txt3 =
            StringPrintf("%s\f%05u\t080%s\n", localization->GetString(18), 0,
                         localization->GetString(630));
        pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight,
                          Color16(0xE1u, 255, 0x9Bu), txt3, 0, 0, 0);
        pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        txt4 = StringPrintf("%s\f%05u\t080%s\n", localization->GetString(53), 0,
                            localization->GetString(630));
    }
    pWindow->DrawText(pFontSmallnum, 150, pTextHeight,
                      Color16(0xE1u, 255, 0x9Bu), txt4, 0, 0, 0);

    pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 6 +
                  pFontSmallnum->GetHeight();
    if (!master_level) {
        auto txt5 =
            StringPrintf("%s\f%05u\t080%s\n", localization->GetString(628), 0,
                         localization->GetString(630));  // "Spell" "?"
        pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight,
                          Color16(0xE1u, 255, 0x9Bu), txt5, 0, 0, 0);
        pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
    } else {
        pText = localization->GetString(628);  // Spell
        if (pActors[uActorID].pMonsterInfo.uSpell1ID &&
            pActors[uActorID].pMonsterInfo.uSpell2ID)
            pText = localization->GetString(629);  // Spells
        if (pActors[uActorID].pMonsterInfo.uSpell1ID) {
            auto txt6 = StringPrintf(
                "%s\f%05u\t070%s\n", pText, 0,
                pSpellStats->pInfos[pActors[uActorID].pMonsterInfo.uSpell1ID]
                    .pShortName);  // "%s\f%05u\t060%s\n"
            pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight,
                              Color16(0xE1u, 255, 0x9Bu), txt6, 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
        if (pActors[uActorID].pMonsterInfo.uSpell2ID) {
            auto txt6 = StringPrintf(
                "\f%05u\t070%s\n", 0,
                pSpellStats->pInfos[pActors[uActorID].pMonsterInfo.uSpell2ID]
                    .pShortName);  // "%s\f%05u\t060%s\n"
            pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight,
                              Color16(0xE1u, 255, 0x9Bu), txt6, 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
        if (!pActors[uActorID].pMonsterInfo.uSpell1ID &&
            !pActors[uActorID].pMonsterInfo.uSpell2ID) {
            auto txt6 = StringPrintf(
                "%s\f%05u\t070%s\n", localization->GetString(628), 0,
                localization->GetString(153));  // "%s\f%05u\t060%s\n"
            pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight,
                              Color16(0xE1u, 255, 0x9Bu), txt6, 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
    }

    pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
    pWindow->DrawText(pFontSmallnum, 150, pTextHeight,
                      Color16(0xE1u, 255, 0x9Bu), localization->GetString(626),
                      0, 0, 0);  // Immune
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
    string_name[9] = localization->GetString(624);  // Physical

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
                pText = localization->GetString(625);  // Immune
            } else {
                if (resistances[i])
                    pText = localization->GetString(627);  // Resistant
                else
                    pText = localization->GetString(153);  // None
            }

            auto txt7 =
                StringPrintf("%s\f%05u\t070%s\n", string_name[i], 0, pText);
            pWindow->DrawText(pFontSmallnum, 170, pTextHeight,
                              Color16(0xE1u, 255, 0x9Bu), txt7, 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
    } else {
        for (uint i = 0; i < 10; ++i) {
            auto txt8 = StringPrintf("%s\f%05u\t070%s\n", string_name[i], 0,
                                     localization->GetString(630));  // "?"
            pWindow->DrawText(pFontSmallnum, 170, pTextHeight,
                              Color16(0xE1u, 255, 0x9Bu), txt8, 0, 0, 0);
            pTextHeight = pTextHeight + pFontSmallnum->GetHeight() - 3;
        }
    }

    // cast spell: Detect life
    if (pParty->pPartyBuffs[PARTY_BUFF_DETECT_LIFE].Active()) {
        auto txt9 =
            StringPrintf("%s: %d", localization->GetString(650),
                         pActors[uActorID].sCurrentHP);  // Current Hit Points
        pFontSmallnum->GetLineWidth(txt9);
        pWindow->DrawTitleText(
            pFontSmallnum, 0,
            pWindow->uFrameHeight - pFontSmallnum->GetHeight() - 12, 0, txt9,
            3);
    }
}

//----- (00417BB5) --------------------------------------------------------
String CharacterUI_GetSkillDescText(unsigned int uPlayerID,
                                    PLAYER_SKILL_TYPE uPlayerSkillType) {
    char a2[1200];     // [sp+Ch] [bp-538h]@7
    char Source[120];  // [sp+4BCh] [bp-88h]@7
    int v35;           // [sp+53Ch] [bp-8h]@1

    v35 = pFontSmallnum->GetLineWidth(localization->GetString(431));  // Normal
    if (pFontSmallnum->GetLineWidth(localization->GetString(433)) >
        (signed int)v35)
        v35 = pFontSmallnum->GetLineWidth(
            localization->GetString(433));  // Expert
    if (pFontSmallnum->GetLineWidth(localization->GetString(432)) >
        (signed int)v35)
        v35 = pFontSmallnum->GetLineWidth(
            localization->GetString(432));  // Master
    if (pFontSmallnum->GetLineWidth(localization->GetString(96)) >
        (signed int)v35)
        v35 =
            pFontSmallnum->GetLineWidth(localization->GetString(96));  // Grand

    char static_sub_417BB5_out_string[1200];
    a2[0] = 0;
    Source[0] = 0;
    strcpy(a2, "%s\n\n");
    sprintf(Source, "\f%05d",
            GetSkillColor(pParty->pPlayers[uPlayerID].classType,
                          uPlayerSkillType, 1));
    strcat(a2, Source);
    strcat(a2, "%s\t%03d:\t%03d%s\t000\n");
    sprintf(Source, "\f%05d",
            GetSkillColor(pParty->pPlayers[uPlayerID].classType,
                          uPlayerSkillType, 2));
    strcat(a2, Source);
    strcat(a2, "%s\t%03d:\t%03d%s\t000\n");
    sprintf(Source, "\f%05d",
            GetSkillColor(pParty->pPlayers[uPlayerID].classType,
                          uPlayerSkillType, 3));
    strcat(a2, Source);
    strcat(a2, "%s\t%03d:\t%03d%s\t000\n");
    sprintf(Source, "\f%05d",
            GetSkillColor(pParty->pPlayers[uPlayerID].classType,
                          uPlayerSkillType, 4));
    strcat(a2, Source);
    strcat(a2, "%s\t%03d:\t%03d%s\t000\n");
    if ((pParty->pPlayers[uPlayerID].pActiveSkills[uPlayerSkillType] & 0x3F) ==
        (pParty->pPlayers[uPlayerID].GetActualSkillLevel(uPlayerSkillType) &
         0x3F)) {
        sprintf(static_sub_417BB5_out_string, a2,
                localization->GetSkillDescription(uPlayerSkillType),
                localization->GetString(431), v35 + 3, v35 + 15,
                localization->GetSkillDescriptionNormal(
                    uPlayerSkillType),  // changed from 5 to 15 to add space
                                        // after ':'
                localization->GetString(433), v35 + 3, v35 + 15,
                localization->GetSkillDescriptionExpert(uPlayerSkillType),
                localization->GetString(432), v35 + 3, v35 + 15,
                localization->GetSkillDescriptionMaster(uPlayerSkillType),
                localization->GetString(96), v35 + 3, v35 + 15,
                localization->GetSkillDescriptionGrand(uPlayerSkillType));
    } else {
        sprintf(Source, "\f%05d", Color16(0xFFu, 0xFFu, 0xFFu));
        strcat(a2, Source);
        strcat(a2, "%s: +%d");
        sprintf(
            static_sub_417BB5_out_string, a2,
            localization->GetSkillDescription(uPlayerSkillType),
            localization->GetString(431), v35 + 3, v35 + 5,
            localization->GetSkillDescriptionNormal(uPlayerSkillType),
            localization->GetString(433), v35 + 3, v35 + 5,
            localization->GetSkillDescriptionExpert(uPlayerSkillType),
            localization->GetString(432), v35 + 3, v35 + 5,
            localization->GetSkillDescriptionMaster(uPlayerSkillType),
            localization->GetString(96), v35 + 3, v35 + 5,
            localization->GetSkillDescriptionGrand(uPlayerSkillType),
            localization->GetString(623),  // Bonus
            (pParty->pPlayers[uPlayerID].GetActualSkillLevel(uPlayerSkillType) &
             0x3F) -
                (pParty->pPlayers[uPlayerID].pActiveSkills[uPlayerSkillType] &
                 0x3F));
    }
    return String(static_sub_417BB5_out_string);
}

void CharacterUI_SkillsTab_ShowHint() {
    unsigned int pX = 0;
    unsigned int pY = 0;
    pMouse->GetClickPos(&pX, &pY);

    if (pX < 24 || pX > 455 || pY < 18 || pY > 36) {
        for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
            if (pButton->msg == UIMSG_SkillUp && pX >= pButton->uX &&
                pX <= pButton->uZ && pY >= pButton->uY && pY <= pButton->uW) {
                CharacterUI_DrawTooltip(
                    localization->GetSkillName(pButton->msg_param),
                    CharacterUI_GetSkillDescText(
                        uActiveCharacter - 1,
                        (PLAYER_SKILL_TYPE)pButton->msg_param));
            }
        }
    } else {
        CharacterUI_DrawTooltip(
            localization->GetString(207),
            pSkillPointsAttributeDescription);  // Skill points   /   Очки
                                                // навыков
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

    Point pt = pMouse->GetCursorPos();
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
                CharacterUI_DrawTooltip(localization->GetString(108),
                                        pHealthPointsAttributeDescription);
            break;
        case 8:  // Spell Points
            if (pSpellPointsAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(212),
                                        pSpellPointsAttributeDescription);
            break;
        case 9:  // Armor Class
            if (pArmourClassAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(12),
                                        pArmourClassAttributeDescription);
            break;
        case 10:  // Player Condition
        {
            auto str = String(pPlayerConditionAttributeDescription) + "\n";

            extern std::array<unsigned int, 18> pConditionImportancyTable;
            for (uint i = 0; i < 18; ++i) {
                if (pPlayers[uActiveCharacter]
                        ->conditions_times[pConditionImportancyTable[i]]
                        .Valid()) {
                    str += " \n";
                    auto condition_time =
                        pParty->GetPlayingTime() -
                        pPlayers[uActiveCharacter]
                            ->conditions_times[pConditionImportancyTable[i]];
                    pHour = condition_time.GetHoursOfDay();
                    pDay = condition_time.GetDays();
                    pTextColor =
                        GetConditionDrawColor(pConditionImportancyTable[i]);
                    str += StringPrintf(format_4E2DE8, pTextColor,
                                        localization->GetCharacterConditionName(
                                            pConditionImportancyTable[i]));
                    if (pHour && pHour <= 1)
                        pHourWord = localization->GetString(109);
                    else
                        pHourWord = localization->GetString(110);
                    if (!pDay ||
                        (pDayWord = localization->GetString(56), pDay > 1))
                        pDayWord = localization->GetString(57);
                    str += StringPrintf("%lu %s, %lu %s", pDay, pDayWord, pHour,
                                        pHourWord);
                }
            }

            if (!str.empty())
                CharacterUI_DrawTooltip(localization->GetString(47), str);
            break;
        }

        case 11:  // Fast Spell
            if (pFastSpellAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(172),
                                        pFastSpellAttributeDescription);
            break;

        case 12:  // Player Age
            if (pPlayerAgeAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(5),
                                        pPlayerAgeAttributeDescription);
            break;

        case 13:  // Player Level
            if (pPlayerLevelAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(131),
                                        pPlayerLevelAttributeDescription);
            break;

        case 14:  // Experience
        {
            v15 = pPlayers[uActiveCharacter]->uLevel;
            do {
                if ((signed __int64)pPlayers[uActiveCharacter]->uExperience <
                    (unsigned int)GetExperienceRequiredForLevel(v15))
                    break;
                ++v15;
            } while (v15 <= 10000);

            String str1;
            String str2;
            if (v15 > pPlayers[uActiveCharacter]->uLevel)
                str1 = StringPrintf(localization->GetString(147), v15);
            str2 = localization->FormatString(
                538,
                (GetExperienceRequiredForLevel(v15) -
                    (int)pPlayers[uActiveCharacter]->uExperience),
                char(v15 + 1));
            str1 += "\n" + str2;

            str2 =
                String(pPlayerExperienceAttributeDescription) + "\n \n" + str1;

            CharacterUI_DrawTooltip(localization->GetString(83), str2);
            break;
        }

        case 15:  // Attack Bonus
            if (pAttackBonusAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(587),
                                        pAttackBonusAttributeDescription);
            break;

        case 16:  // Attack Damage
            if (pAttackDamageAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(588),
                                        pAttackDamageAttributeDescription);
            break;

        case 17:  // Missle Bonus
            if (pMissleBonusAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(589),
                                        pMissleBonusAttributeDescription);
            break;

        case 18:  // Missle Damage
            if (pMissleDamageAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(590),
                                        pMissleDamageAttributeDescription);
            break;

        case 19:  // Fire Resistance
            if (pFireResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(87),
                                        pFireResistanceAttributeDescription);
            break;

        case 20:  // Air Resistance
            if (pAirResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(6),
                                        pAirResistanceAttributeDescription);
            break;

        case 21:  // Water Resistance
            if (pWaterResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(240),
                                        pWaterResistanceAttributeDescription);
            break;

        case 22:  // Earth Resistance
            if (pEarthResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(70),
                                        pEarthResistanceAttributeDescription);
            break;

        case 23:  // Mind Resistance
            if (pMindResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(142),
                                        pMindResistanceAttributeDescription);
            break;

        case 24:  // Body Resistance
            if (pBodyResistanceAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(29),
                                        pBodyResistanceAttributeDescription);
            break;

        case 25:  // Skill Points
            if (pSkillPointsAttributeDescription)
                CharacterUI_DrawTooltip(localization->GetString(207),
                                        pSkillPointsAttributeDescription);
            break;

        case 26:  // Class description
        {
            CharacterUI_DrawTooltip(localization->GetClassName(
                                        pPlayers[uActiveCharacter]->classType),
                                    localization->GetClassDescription(
                                        pPlayers[uActiveCharacter]->classType));
        } break;

        default:
            break;
    }
}

//----- (00410B28) --------------------------------------------------------
void DrawSpellDescriptionPopup(int spell_index) {
    SpellInfo *spell;             // esi@1
    unsigned int v3;              // eax@2
    long v5;                      // ecx@4
    GUIWindow spell_info_window;  // [sp+Ch] [bp-68h]@4

    Point pt = pMouse->GetCursorPos();

    spell =
        &pSpellStats
             ->pInfos[spell_index +
                      11 * pPlayers[uActiveCharacter]->lastOpenedSpellbookPage +
                      1];
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
    v5 = pFontSmallnum->GetLineWidth(localization->GetString(431));  // Normal
    if (pFontSmallnum->GetLineWidth(localization->GetString(432)) >
        v5)  // Master
        v5 = pFontSmallnum->GetLineWidth(
            localization->GetString(432));  // Master
    if (pFontSmallnum->GetLineWidth(localization->GetString(433)) >
        v5)  // Expert
        v5 = pFontSmallnum->GetLineWidth(
            localization->GetString(433));  // Expert
    if (pFontSmallnum->GetLineWidth(localization->GetString(96)) > v5)  // Grand
        v5 = pFontSmallnum->GetLineWidth(localization->GetString(96));

    String str = StringPrintf(
        "%s\n\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%"
        "s\t000\n%s\t%03d:\t%03d%s",
        spell->pDescription, localization->GetString(431), v5 + 3, v5 + 10,
        spell->pBasicSkillDesc,  // Normal
        localization->GetString(433), v5 + 3, v5 + 10,
        spell->pExpertSkillDesc,  // Expert
        localization->GetString(432), v5 + 3, v5 + 10,
        spell->pMasterSkillDesc,  // Master
        localization->GetString(96), v5 + 3, v5 + 10,
        spell->pGrandmasterSkillDesc);  // Grand
    spell_info_window.uFrameHeight +=
        pFontSmallnum->CalcTextHeight(str, spell_info_window.uFrameWidth, 0);
    if ((signed int)spell_info_window.uFrameHeight < 150)
        spell_info_window.uFrameHeight = 150;
    spell_info_window.uFrameWidth = game_viewport_width;
    spell_info_window.DrawMessageBox(0);
    spell_info_window.uFrameWidth -= 12;
    spell_info_window.uFrameHeight -= 12;
    spell_info_window.uFrameZ =
        spell_info_window.uFrameX + spell_info_window.uFrameWidth - 1;
    spell_info_window.uFrameW =
        spell_info_window.uFrameHeight + spell_info_window.uFrameY - 1;
    spell_info_window.DrawTitleText(
        pFontArrus, 0x78u, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu), spell->pName, 3);
    spell_info_window.DrawText(pFontSmallnum, 120, 44, 0, str, 0, 0, 0);
    spell_info_window.uFrameWidth = 108;
    spell_info_window.uFrameZ = spell_info_window.uFrameX + 107;
    int skill_level = SkillToMastery(
        pPlayers[uActiveCharacter]->pActiveSkills
            [pPlayers[uActiveCharacter]->lastOpenedSpellbookPage + 12]);
    spell_info_window.DrawTitleText(
        pFontComic, 12, 75, 0,
        localization->GetSkillName(
            pPlayers[uActiveCharacter]->lastOpenedSpellbookPage + 12),
        3);

    auto str2 = StringPrintf(
        "%s\n%d", localization->GetString(522),  // SP Cost
        pSpellDatas[spell_index +
                    11 * pPlayers[uActiveCharacter]->lastOpenedSpellbookPage +
                    1]
            .mana_per_skill[skill_level - 1]);
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

    if (current_screen_type == SCREEN_VIDEO || GetCurrentMenuID() == MENU_MAIN)
        return;


    unsigned int pX = mouse_x;
    unsigned int pY = mouse_y;

    // if ( render->bWindowMode )
    {
        Point pt = Point(pX, pY);
        if (pt.x < 1 || pt.y < 1 || pt.x > 638 || pt.y > 478) {
            back_to_game();
            return;
        }
    }
    if (pParty->pPickedItem.uItemID) {  // нажатие на портрет перса правой кнопкой
                                        // мыши с раствором
        for (uint i = 0; i < 4; ++i) {
            if ((signed int)pX > RightClickPortraitXmin[i] &&
                (signed int)pX < RightClickPortraitXmax[i] &&
                (signed int)pY > 375 && (signed int)pY < 466) {
                pPlayers[uActiveCharacter]->UseItem_DrinkPotion_etc(i + 1, 1);
                return;
            }
        }
    }

    pEventTimer->Pause();
    switch (current_screen_type) {
        case SCREEN_CASTING: {
            Inventory_ItemPopupAndAlchemy();
            break;
        }
        case SCREEN_CHEST: {
            if (!pPlayers[uActiveCharacter]->CanAct()) {
                static String hint_reference;
                hint_reference = localization->FormatString(
                    427, pPlayers[uActiveCharacter]->pName,
                    localization->GetString(
                        541));  // %s не в состоянии %s Опознать предметы

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
                    int chestindex = vChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[invMatrixIndex];
                    if (chestindex < 0) {
                        invMatrixIndex = (-(chestindex + 1));
                        chestindex = vChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[invMatrixIndex];
                    }

                    if (chestindex) {
                        int itemindex = chestindex - 1;

                        GameUI_DrawItemInfo(&vChests[pChestWindow->par1C].igChestItems[itemindex]);
                    }
                }
            }
            break;
        }

        case SCREEN_GAME:  // In the main menu displays a pop-up window(В
                           // главном меню показывает всплывающее окно)
        {
            if (GetCurrentMenuID() > 0) break;
            if ((signed int)pY > (signed int)pViewport->uViewportBR_Y) {
                popup_window.ptr_1C = (void *)((signed int)pX / 118);
                if ((signed int)pX / 118 < 4) {  // portaits zone
                    popup_window.sHint.clear();
                    popup_window.uFrameWidth = 400;
                    popup_window.uFrameHeight = 200;
                    popup_window.uFrameX = 38;
                    popup_window.uFrameY = 60;
                    pAudioPlayer->StopChannels(-1, -1);
                    GameUI_CharacterQuickRecord_Draw(
                        &popup_window, pPlayers[(int)popup_window.ptr_1C + 1]);
                }
            } else if ((int)pX > pViewport->uViewportBR_X) {
                if (pY >= 130) {
                    if (pX >= 476 && pX <= 636 && pY >= 240 && pY <= 300) {  // buff_tooltip zone
                        popup_window.sHint.clear();
                        popup_window.uFrameWidth = 400;
                        popup_window.uFrameHeight = 200;
                        popup_window.uFrameX = 38;
                        popup_window.uFrameY = 60;
                        pAudioPlayer->StopChannels(-1, -1);
                        popup_window._41D73D_draw_buff_tooltip();
                    } else if ((int)pX < 485 || (int)pX > 548 ||
                               (int)pY < 156 ||
                               (int)pY > 229) {  // NPC zone
                        if (!((signed int)pX < 566 || (signed int)pX > 629 ||
                              (signed int)pY < 156 || (signed int)pY > 229)) {
                            pAudioPlayer->StopChannels(-1, -1);
                            GameUI_DrawNPCPopup((void *)1);  // NPC 2
                        }
                    } else {
                        pAudioPlayer->StopChannels(-1, -1);
                        GameUI_DrawNPCPopup(0);  // NPC 1
                    }
                } else {  // minimap zone
                    popup_window.sHint = GameUI_GetMinimapHintText();
                    popup_window.uFrameWidth = 256;
                    popup_window.uFrameX = 130;
                    popup_window.uFrameY = 140;
                    popup_window.uFrameHeight = 64;
                    pAudioPlayer->StopChannels(-1, -1);
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

                auto vis = EngineIoc::ResolveVis();
                v5 = vis->get_picked_object_zbuf_val();
                /*else
                v5 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]];*/
                if (PID_TYPE(v5) == OBJECT_Actor) {
                    render->BeginScene();
                    popup_window.DrawMessageBox(1);
                    MonsterPopup_Draw(PID_ID(v5), &popup_window);
                    render->EndScene();
                }
                if (PID_TYPE(v5) == OBJECT_Item) {
                    if (!(pObjectList
                              ->pObjects[pSpriteObjects[PID_ID((unsigned __int16)v5)].uObjectDescID].uFlags & 0x10)) {
                        GameUI_DrawItemInfo(
                            &pSpriteObjects[PID_ID((unsigned __int16)v5)].containing_item);
                    }
                }
            }
            break;
        }
        case SCREEN_BOOKS: {
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
            pAudioPlayer->StopChannels(-1, -1);
            popup_window.DrawMessageBox(0);
            break;
        }
        case SCREEN_CHARACTERS:
        case SCREEN_E:
        case SCREEN_CHEST_INVENTORY: {
            if ((signed int)pX > 467 && current_screen_type != SCREEN_E)
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
            if (dword_507B00_spell_info_to_draw_in_popup)
                DrawSpellDescriptionPopup(
                    dword_507B00_spell_info_to_draw_in_popup - 1);
            break;
        }
        case SCREEN_HOUSE: {
            if ((signed int)pY < 345 && (signed int)pX < 469)
                ShowPopupShopItem();
            break;
        }
        case SCREEN_PARTY_CREATION: {
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
                            popup_window.sHint =
                                localization->GetAttributeDescription(
                                    (int)pButton->msg_param % 7);
                            pStr = localization->GetAttirubteName(
                                (int)pButton->msg_param % 7);
                            break;
                        case UIMSG_PlayerCreationClickPlus:  // Plus button info
                            pStr = localization->GetString(670);  //Добавить
                            popup_window.sHint = localization->GetString(
                                671);  //"Добавляет очко к выделенному навыку,
                                       //забирая его из накопителя очков"
                            break;
                        case UIMSG_PlayerCreationClickMinus:  // Minus button
                                                              // info
                            pStr = localization->GetString(668);  //Вычесть
                            popup_window.sHint = localization->GetString(
                                669);  //"Вычитает очко из выделенного навыка,
                                       //возвращая его в накопитель очков"
                            break;
                        case UIMSG_PlayerCreationSelectActiveSkill:  // Available
                                                                     // skill
                                                                     // button
                                                                     // info
                            pStr = localization->GetSkillName(
                                pParty
                                    ->pPlayers
                                        [uPlayerCreationUI_SelectedCharacter]
                                    .GetSkillIdxByOrder(pButton->msg_param +
                                                        4));
                            popup_window
                                .sHint = localization->GetSkillDescription(
                                pParty
                                    ->pPlayers
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
                                664);  //Щелкните здесь для утверждения состава
                                       //отряда и продолжения игры.
                            pStr = localization->GetString(665);  //Кнопка ОК
                            break;
                        case UIMSG_PlayerCreationClickReset:  // Clear info
                            popup_window.sHint = localization->GetString(
                                666);  //Сбрасывает все параметры и навыки
                                       //отряда.
                            pStr =
                                localization->GetString(667);  //Кнопка Очистить
                            break;
                        case UIMSG_PlayerCreation_SelectAttribute:  // Character
                                                                    // info
                            pStr = pParty->pPlayers[pButton->msg_param].pName;
                            popup_window
                                .sHint = localization->GetClassDescription(
                                pParty->pPlayers[pButton->msg_param].classType);
                            break;
                    }
                    if (pButton->msg > UIMSG_44 &&
                        pButton->msg <=
                            UIMSG_PlayerCreationRemoveDownSkill) {  // Sellected
                                                                    // skills info
                        pY = 0;
                        if ((int)pParty->pPlayers[pButton->msg_param]
                                .GetSkillIdxByOrder(pButton->msg - UIMSG_48) <
                            37) {
                            static String hint_reference;
                            hint_reference = CharacterUI_GetSkillDescText(
                                pButton->msg_param,
                                (PLAYER_SKILL_TYPE)pParty
                                    ->pPlayers[pButton->msg_param]
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
                String sHint = popup_window.sHint;
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

                auto str = StringPrintf("\f%05d%s\f00000\n",
                                        Color16(0xFF, 0xFF, 0x9B), pStr);
                popup_window.DrawTitleText(pFontCreate, 0, 0, 0, str.c_str(),
                                           3);
                popup_window.DrawText(pFontSmallnum, 1,
                                      pFontLucida->GetHeight(), 0, sHint, 0, 0,
                                      0);
            }
            break;
        }
        default:
            break;
    }
    dword_507BF0_is_there_popup_onscreen = 1;
    viewparams->bRedrawGameUI = 1;
}

int no_rightlick_in_inventory = false;  // 0050CDCC
//----- (00416196) --------------------------------------------------------
void Inventory_ItemPopupAndAlchemy() {  // needs cleaning
    int potion1_id;             // edx@25
    unsigned int potion2_id;    // edi@25
    signed int potionID;        // edx@27
    // unsigned int pOut_y;        // edx@57
    double v31;                 // st7@112
    Vec3_int_ v39;              // [sp-18h] [bp-A8h]@83
    GUIWindow message_window;   // [sp+Ch] [bp-84h]@137
    unsigned int damage_level;  // [sp+8Ch] [bp-4h]@23

    if (no_rightlick_in_inventory) return;

    signed int inventoryXCoord;  // ecx@2
    int inventoryYCoord;         // eax@2

    unsigned int pY;  // [sp+3Ch] [bp-Ch]@2
    unsigned int pX;
    // Point cursor = pMouse->GetCursorPos();

    ItemGen *item = nullptr;

    int mousex = pMouse->uMouseClickX;  // condense
    int mousey = pMouse->uMouseClickY;  // condense

    static int RingsX[6] = {0x1EA, 0x21A, 0x248, 0x1EA, 0x21A, 0x248};
    static int RingsY[6] = {0x0CA, 0x0CA, 0x0CA, 0x0FA, 0x0FA, 0x0FA};

    static int glovex = 586;
    static int glovey = 88;

    static int amuletx = 493;
    static int amulety = 91;

    int slot = 32;
    int pos = -1;

    pMouse->GetClickPos(&pX, &pY);
    inventoryYCoord = (pY - 17) / 32;
    inventoryXCoord = (pX - 14) / 32;
    int invMatrixIndex =
        inventoryXCoord + (14 * inventoryYCoord);  // INVETORYSLOTSWIDTH

    if (pX <= 13 || pX >= 462) {  // items out of inventory(вещи вне инвентаря)
                                  // this is for player ragdoll items??
        // popup checks if ringscreen up here

        if (!ringscreenactive()) {  // rings not displayd
            int item_pid =
                (render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]] &
                 0xFFFF) -
                1;
            // zbuffer still used for paperdolls

            if (item_pid == -1) return;

            item = &pPlayers[uActiveCharacter]->pInventoryItemList[item_pid];
            GameUI_DrawItemInfo(item);
            return;
        } else {  // rings displayed
            if (mousex < 490 || mousex > 618) return;

            if (mousey < 88 || mousey > 282) return;

            if (mousex >= amuletx && mousex <= (amuletx + slot) &&
                mousey >= amulety && mousey <= (amulety + 2 * slot)) {
                // amulet
                // pitem = pPlayers[uActiveCharacter]->GetAmuletItem(); //9
                pos = 9;
            }

            if (mousex >= glovex && mousex <= (glovex + slot) &&
                mousey >= glovey && mousey <= (glovey + 2 * slot)) {
                // glove
                // pitem = pPlayers[uActiveCharacter]->GetGloveItem(); //7
                pos = 7;
            }

            for (int i = 0; i < 6; ++i) {
                if (mousex >= RingsX[i] && mousex <= (RingsX[i] + slot) &&
                    mousey >= RingsY[i] && mousey <= (RingsY[i] + slot)) {
                    // ring
                    // pitem = pPlayers[uActiveCharacter]->GetNthRingItem(i);
                    // //10+i
                    pos = 10 + i;
                }
            }

            if (pos != -1)
                item = pPlayers[uActiveCharacter]->GetNthEquippedIndexItem(pos);

            if (!item) return;

            GameUI_DrawItemInfo(item);

            return;
        }
    }

    // limits check ?
    // if (inventoryYCoord >= 0 && inventoryYCoord < INVETORYSLOTSHEIGHT &&
    // inventoryXCoord >= 0 && inventoryXCoord < INVETORYSLOTSWIDTH) {

    item = pPlayers[uActiveCharacter]->GetItemAtInventoryIndex(invMatrixIndex);

    if (!item) {  // no item
        return;
    }

    // if (item_pid == -1) //added here to avoid crash
    //    return;

    // check character condition(проверка состояния персонажа)
    if (!pPlayers[uActiveCharacter]->CanAct()) {
        static String hint_reference;
        hint_reference = localization->FormatString(
            427, pPlayers[uActiveCharacter]->pName,
            localization->GetString(
                541));  // %s не в состоянии %s Опознать предметы

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

    int alchemy_skill_points =
        pPlayers[uActiveCharacter]->GetActualSkillLevel(PLAYER_SKILL_ALCHEMY);
    int alchemy_skill_level =
        pPlayers[uActiveCharacter]->GetActualSkillMastery(PLAYER_SKILL_ALCHEMY);

    // for potion bottle(простая бутылка)
    if (pParty->pPickedItem.uItemID == ITEM_POTION_BOTTLE) {
        GameUI_DrawItemInfo(item);
        return;
    }

    // for recharge potion(зелье перезарядка)
    if (pParty->pPickedItem.uItemID == ITEM_POTION_RECHARGE_ITEM) {
        if (item->uItemID < ITEM_POTION_BOTTLE ||
            item->uItemID > ITEM_POTION_REJUVENATION) {  // all potions
            if (item->GetItemEquipType() !=
                EQUIP_WAND) {  // can recharge only wands
                pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                return;
            }

            v31 = (70.0 - (double)pParty->pPickedItem.uEnchantmentType) * 0.01;
            if (v31 < 0.0) v31 = 0.0;
            item->uMaxCharges =
                (signed __int64)((double)item->uMaxCharges -
                                 v31 * (double)item->uMaxCharges);
            item->uNumCharges =
                (signed __int64)((double)item->uMaxCharges -
                                 v31 * (double)item->uMaxCharges);

            pMouse->RemoveHoldingItem();
            no_rightlick_in_inventory = 1;
            return;
        }
        GameUI_DrawItemInfo(item);
        return;
    } else if (pParty->pPickedItem.uItemID == ITEM_POTION_HARDEN_ITEM) {
      // for harden potion(зелье закалка)
      if (item->uItemID < ITEM_POTION_BOTTLE ||
            item->uItemID > ITEM_POTION_REJUVENATION) {  // bottle and all potions
            if (item->IsBroken() ||  // cant harden broken items
                item->uItemID >= ITEM_ARTIFACT_PUCK ||  // cant harden artifacts
                item->GetItemEquipType() < EQUIP_SINGLE_HANDED ||
                item->GetItemEquipType() > EQUIP_WAND) {
                pMouse->RemoveHoldingItem();
                no_rightlick_in_inventory = true;
                return;
            }

            item->uAttributes |= ITEM_AURA_EFFECT_RED | ITEM_HARDENED;

            _50C9A8_item_enchantment_timer = 256;
            pMouse->RemoveHoldingItem();
            no_rightlick_in_inventory = true;
            return;
        }
        GameUI_DrawItemInfo(item);
        return;
    } else if (pParty->pPickedItem.uItemID >= ITEM_POTION_FLAMING_POTION &&
      // several potions(несколько зелий)
      pParty->pPickedItem.uItemID <= ITEM_POTION_SWIFT_POTION ||
             pParty->pPickedItem.uItemID == ITEM_POTION_SLAYING_POTION) {
        if (item->uItemID < ITEM_POTION_BOTTLE ||
            item->uItemID > ITEM_POTION_REJUVENATION) {  // all potions
            if (item->uItemID >= ITEM_BLASTER &&
                    item->uItemID <= ITEM_LASER_RIFLE ||
                item->uItemID >= ITEM_ARTIFACT_PUCK || item->IsBroken() ||
                item->special_enchantment || item->uEnchantmentType ||
                item->GetItemEquipType() >=
                    EQUIP_ARMOUR) {  // only melee weapons and bows
                pMouse->RemoveHoldingItem();
                no_rightlick_in_inventory = true;
                return;
            }

            item->UpdateTempBonus(pParty->GetPlayingTime());
            if (pParty->pPickedItem.uItemID == ITEM_POTION_SLAYING_POTION) {
                item->special_enchantment = ITEM_ENCHANTMENT_40;  // of Slaying
                v31 = (double)(1800 * pParty->pPickedItem.uEnchantmentType);
            } else {
                static ITEM_ENCHANTMENT _4E2904_enchantment_by_potion_lut[] = {
                    (ITEM_ENCHANTMENT)164,      (ITEM_ENCHANTMENT)93,
                    (ITEM_ENCHANTMENT)22,       (ITEM_ENCHANTMENT)164,
                    (ITEM_ENCHANTMENT)93,       (ITEM_ENCHANTMENT)22,
                    ITEM_ENCHANTMENT_OF_FLAME,  ITEM_ENCHANTMENT_OF_FROST,
                    ITEM_ENCHANTMENT_OF_POISON, ITEM_ENCHANTMENT_OF_SPARKS,
                    (ITEM_ENCHANTMENT)59};
                item->special_enchantment = _4E2904_enchantment_by_potion_lut
                    [pParty->pPickedItem.uItemID - 240];
                v31 = (double)(1800 * pParty->pPickedItem.uEnchantmentType);
            }

            item->expirte_time =
                GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(v31));
            item->uAttributes = alchemy_skill_level | 0x18;

            _50C9A8_item_enchantment_timer = 256;
            pMouse->RemoveHoldingItem();
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
            case ITEM_TROLL_BLOOD:
            case ITEM_TROLL_RUBY:
            case ITEM_DRAGON_EYE:
                item->uItemID = ITEM_POTION_CURE_WOUNDS;
                break;

            case ITEM_PHIMA_ROOT:
            case ITEM_METEORITE_FRAGMENT:
            case ITEM_HARPY_FEATHER:
            case ITEM_MOONSTONE:
            case ITEM_ELVISH_TOADSTOOL:
                item->uItemID = ITEM_POTION_MAGIC_POTION;
                break;

            case ITEM_POPPYSNAPS:
            case ITEM_FAE_DUST:
            case ITEM_SULFUR:
            case ITEM_GARNET:
            case ITEM_DEVIL_ICHOR:
                item->uItemID = ITEM_POTION_CURE_WEAKNESS;
                break;

            case ITEM_MUSHROOM:
            case ITEM_OBSIDIAN:
            case ITEM_OOZE_ENDOPLASM_VIAL:
            case ITEM_MERCURY:
            case ITEM_REAGENT_PHILOSOPHERS_STONE:
                item->uItemID = ITEM_POTION_CATALYST;
                break;
            default:
                break;
        }
        pMouse->RemoveHoldingItem();
        no_rightlick_in_inventory = 1;
        if (dword_4E455C) {
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_DO_POTION_FINE, 0);
            dword_4E455C = 0;
        }
        return;
    }

    // potions mixing(смешивание двух зелий)
    if (pParty->pPickedItem.uItemID >= ITEM_POTION_CATALYST &&
        pParty->pPickedItem.uItemID <= ITEM_POTION_REJUVENATION &&
        item->uItemID >= ITEM_POTION_CATALYST &&
        item->uItemID <= ITEM_POTION_REJUVENATION) {
        potion1_id = item->uItemID - ITEM_POTION_CURE_WOUNDS;
        potion2_id = pParty->pPickedItem.uItemID - ITEM_POTION_CURE_WOUNDS;

        if (pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST ||
            item->uItemID == ITEM_POTION_CATALYST)
            potionID = 5;
        else
            potionID = pItemsTable->potion_data[potion2_id][potion1_id];

        damage_level = 0;
        if (alchemy_skill_points) {
            if (potionID < ITEM_POTION_CURE_DISEASE ||
                potionID > ITEM_POTION_AWAKEN) {  // < 225 >227
                if (potionID >= ITEM_POTION_HASTE &&
                    potionID <= ITEM_POTION_CURE_INSANITY &&
                    alchemy_skill_level == 1)  // 228 >= potionID <= 239
                    damage_level = 2;
                if (potionID >= ITEM_POTION_MIGHT_BOOST &&
                    potionID <= ITEM_POTION_BODY_RESISTANE &&
                    alchemy_skill_level <= 2)  // 240 >= potionID <= 261
                    damage_level = 3;
                if (potionID >= ITEM_POTION_STONE_TO_FLESH &&
                    alchemy_skill_level <= 3)  // 262 < potionID
                    damage_level = 4;
            }
        } else {  // no skill(нет навыка)
            if (potionID >= ITEM_POTION_CURE_DISEASE &&
                potionID <= ITEM_POTION_AWAKEN)  // 225 <= v16 <= 227
                damage_level = 1;
            if (potionID >= ITEM_POTION_HASTE &&
                potionID <= ITEM_POTION_CURE_INSANITY)  // 228 <= v16 <= 239
                damage_level = 2;
            if (potionID >= ITEM_POTION_MIGHT_BOOST &&
                potionID <= ITEM_POTION_BODY_RESISTANE)  // 240 <= v16 <= 261
                damage_level = 3;
            if (potionID >= ITEM_POTION_STONE_TO_FLESH)  // 262 <= v16
                damage_level = 4;
        }

        int item_pid = pPlayers[uActiveCharacter]->GetItemListAtInventoryIndex(
            invMatrixIndex);
        // int pOut_x = item_pid + 1;
        // for (uint i = 0; i < 126; ++i)
        //{
        //  if (pPlayers[uActiveCharacter]->pInventoryMatrix[i] == pOut_x)
        // {
        //    pOut_y = i;
        //   break;
        //}
        //}
        if (!potionID) {
            GameUI_DrawItemInfo(item);
            return;
        }

        if (damage_level > 0) {
            pPlayers[uActiveCharacter]->RemoveItemAtInventoryIndex(
                invMatrixIndex);  // pOut_y); ?? quickfix needs checking

            if (damage_level == 1) {
                pPlayers[uActiveCharacter]->ReceiveDamage(rand() % 11 + 10,
                                                          DMGT_FIRE);
            } else if (damage_level == 2) {
                pPlayers[uActiveCharacter]->ReceiveDamage(rand() % 71 + 30,
                                                          DMGT_FIRE);
                pPlayers[uActiveCharacter]->ItemsPotionDmgBreak(1);  // break 1
            } else if (damage_level == 3) {
                pPlayers[uActiveCharacter]->ReceiveDamage(rand() % 201 + 50,
                                                          DMGT_FIRE);
                pPlayers[uActiveCharacter]->ItemsPotionDmgBreak(5);  // break 5
            } else if (damage_level >= 4) {
                pPlayers[uActiveCharacter]->SetCondition(Condition_Eradicated,
                                                         0);
                pPlayers[uActiveCharacter]->ItemsPotionDmgBreak(
                    0);  // break everything
            }

            pAudioPlayer->PlaySound(SOUND_fireBall, 0, 0, -1, 0, 0);
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
            v39.z = pParty->vPosition.z + pParty->sEyelevel;
            v39.x = pParty->vPosition.x;
            v39.y = pParty->vPosition.y;

            int rot_x, rot_y, rot_z;
            Vec3_int_::Rotate(64, pParty->sRotationY, pParty->sRotationX, v39,
                              &rot_x, &rot_y, &rot_z);
            SpriteObject::sub_42F7EB_DropItemAt(
                SPRITE_SPELL_FIRE_FIREBALL_IMPACT, rot_x, rot_y, rot_z, 0, 1, 0,
                0, 0);
            if (dword_4E455C) {
                if (pPlayers[uActiveCharacter]->CanAct())
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_17, 0);
                GameUI_StatusBar_OnEvent(
                    localization->GetString(444));  // Ouch!   Ой!
                dword_4E455C = 0;
            }
            pMouse->RemoveHoldingItem();
            no_rightlick_in_inventory = 1;
            return;
        } else {  // if ( damage_level == 0 )
            if (alchemy_skill_points) {
                if (item->uItemID == ITEM_POTION_CATALYST ||
                    pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST) {
                    if (item->uItemID == ITEM_POTION_CATALYST)
                        item->uItemID = pParty->pPickedItem.uItemID;
                    if (pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST)
                        item->uEnchantmentType =
                            pParty->pPickedItem.uEnchantmentType;
                } else {
                    item->uItemID = potionID;
                    item->uEnchantmentType =
                        (pParty->pPickedItem.uEnchantmentType +
                         item->uEnchantmentType) /
                        2;
                    pPlayers[uActiveCharacter]->SetVariable(
                        VAR_AutoNotes,
                        pItemsTable->potion_note[potion1_id][potion2_id]);
                }
                int bottle =
                    pPlayers[uActiveCharacter]->AddItem(-1, ITEM_POTION_BOTTLE);
                if (bottle)
                    pPlayers[uActiveCharacter]
                        ->pOwnItems[bottle - 1]
                        .uAttributes = ITEM_IDENTIFIED;
                if (!(pItemsTable->pItems[item->uItemID].uItemID_Rep_St))
                    item->uAttributes |= 1;
                if (!dword_4E455C) {
                    pMouse->RemoveHoldingItem();
                    no_rightlick_in_inventory = 1;
                    return;
                }
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_DO_POTION_FINE, 0);
                dword_4E455C = 0;
                pMouse->RemoveHoldingItem();
                no_rightlick_in_inventory = 1;
                return;
            }
            GameUI_DrawItemInfo(item);
            return;
        }
    }

    GameUI_DrawItemInfo(item);
    return;
}

//----- (0045828B) --------------------------------------------------------
unsigned int GetSpellColor(signed int a1) {
    if (a1 == 0) return Color16(0, 0, 0);
    if (a1 < 12) return Color16(255, 85, 0);
    if (a1 < 23) return Color16(150, 212, 255);
    if (a1 < 34) return Color16(0, 128, 255);
    if (a1 < 45) return Color16(128, 128, 128);
    if (a1 < 56) return Color16(225, 225, 225);
    if (a1 < 67) return Color16(235, 15, 255);
    if (a1 < 78) return Color16(255, 128, 0);
    if (a1 < 89) return Color16(255, 255, 155);
    if (a1 < 100)
        return Color16(192, 192, 240);
    else
        __debugbreak();
}

//----- (004B46F8) --------------------------------------------------------
__int64 GetExperienceRequiredForLevel(int level) {
    __int64 v1;  // eax@1
    int i;       // edx@1

    v1 = 0;
    for (i = 0; i < level; ++i) v1 += i + 1;
    return 1000 * v1;
}
