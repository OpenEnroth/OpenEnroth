#include "GUI/GUIWindow.h"

#include <stdlib.h>
#include <sstream>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Autonotes.h"
#include "Engine/Awards.h"
#include "Engine/Events.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/IocContainer.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/StorylineTextTable.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIArena.h"
#include "GUI/UI/UIBooks.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/InputAction.h"
#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"


using EngineIoc = Engine_::IocContainer;
using Io::InputAction;

GUIWindow *pPrimaryWindow;
GUIWindow *pChestWindow;
GUIWindow *pDialogueWindow;
GUIWindow *window_SpeakInHouse;
GUIWindow *pGUIWindow_ScrollWindow;
GUIWindow *ptr_507BC8;
GUIWindow *pGUIWindow_CurrentMenu;
GUIWindow *ptr_507BD0;
GUIWindow *pGUIWindow_CastTargetedSpell;
GUIWindow *pModalWindow;
GUIWindow *pGUIWindow_EscMessageWindow;
GUIWindow *pBooksButtonOverlay;
GUIWindow *pGUIWindow2;

typedef struct _RGBColor {
    unsigned char R;
    unsigned char B;
    unsigned char G;
} RGBColor;

std::array<RGBColor, 20> spell_tooltip_colors = { {
    { 0x96, 0xD4, 0xFF }, { 0xFF, 0x80, 0x00 }, { 0xFF, 0xFF, 0x9B },
    { 0xE1, 0xE1, 0xE1 }, { 0x80, 0x80, 0x80 }, { 0x96, 0xD4, 0xFF },
    { 0xFF, 0x55, 0x00 }, { 0x96, 0xD4, 0xFF }, { 0xFF, 0x55, 0x00 },
    { 0xE1, 0xE1, 0xE1 }, { 0xFF, 0x55, 0x00 }, { 0x96, 0xD4, 0xFF },
    { 0xEB, 0x0F, 0xFF }, { 0xFF, 0x80, 0x00 }, { 0x96, 0xD4, 0xFF },
    { 0x80, 0x80, 0x80 }, { 0xFF, 0x55, 0x00 }, { 0x00, 0x80, 0xFF },
    { 0x00, 0x80, 0xFF }, { 0x96, 0xD4, 0xFF }
} };

enum WindowType current_character_screen_window;
std::list<GUIWindow*> lWindowList;

MENU_STATE sCurrentMenuID;

enum CURRENT_SCREEN current_screen_type = CURRENT_SCREEN::SCREEN_VIDEO;
enum CURRENT_SCREEN prev_screen_type;

struct GUIMessageQueue *pMessageQueue_50CBD0 = new GUIMessageQueue;
struct GUIMessageQueue *pMessageQueue_50C9E8 = new GUIMessageQueue;

Image *ui_exit_cancel_button_background = nullptr;
Image *game_ui_right_panel_frame = nullptr;
Image *dialogue_ui_x_ok_u = nullptr;
Image *dialogue_ui_x_x_u = nullptr;

Image *ui_buttdesc2 = nullptr;
Image *ui_buttyes2 = nullptr;

Image *ui_btn_npc_right = nullptr;
Image *ui_btn_npc_left = nullptr;

Image *ui_ar_dn_dn = nullptr;
Image *ui_ar_dn_up = nullptr;
Image *ui_ar_up_dn = nullptr;
Image *ui_ar_up_up = nullptr;

Image *ui_leather_mm6 = nullptr;
Image *ui_leather_mm7 = nullptr;

DIALOGUE_TYPE _dword_F8B1D8_last_npc_topic_menu;
AwardType dword_F8B1AC_award_bit_number;


const wchar_t *MENU_STATE_to_string(MENU_STATE m) {
    switch (m) {
    case -1: return L"-1";
    case MENU_MAIN: return L"MENU_MAIN";
    case MENU_NEWGAME: return L"MENU_NEWGAME";
    case MENU_CREDITS: return L"MENU_CREDITS";
    case MENU_SAVELOAD: return L"MENU_SAVELOAD";
    case MENU_EXIT_GAME: return L"MENU_EXIT_GAME";
    case MENU_5: return L"MENU_5";
    case MENU_CREATEPARTY: return L"MENU_CREATEPARTY";
    case MENU_NAMEPANELESC: return L"MENU_NAMEPANELESC";
    case MENU_CREDITSPROC: return L"MENU_CREDITSPROC";
    case MENU_LoadingProcInMainMenu: return L"MENU_LoadingProcInMainMenu";
    case MENU_DebugBLVLevel: return L"MENU_DebugBLVLevel";
    case MENU_CREDITSCLOSE: return L"MENU_CREDITSCLOSE";
    case MENU_MMT_MAIN_MENU: return L"MENU_MMT_MAIN_MENU";
    default: return L"invalid";
    }
}

void SetCurrentMenuID(MENU_STATE uMenu) {
    sCurrentMenuID = uMenu;
    logger->Warning("CurrentMenu = %s \n", MENU_STATE_to_string(uMenu));
}

MENU_STATE GetCurrentMenuID() {
    return sCurrentMenuID;
}

bool PauseGameDrawing() {
    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME &&
        current_screen_type != CURRENT_SCREEN::SCREEN_NPC_DIALOGUE &&
        current_screen_type != CURRENT_SCREEN::SCREEN_CHANGE_LOCATION) {
        if (current_screen_type == CURRENT_SCREEN::SCREEN_INPUT_BLV) return uCurrentHouse_Animation;
        if (current_screen_type != CURRENT_SCREEN::SCREEN_BRANCHLESS_NPC_DIALOG) return true;
    }
    return false;
}

OnCastTargetedSpell::OnCastTargetedSpell(unsigned int x, unsigned int y,
    unsigned int width,
    unsigned int height, GUIButton *button,
    const String &hint
)
    : GUIWindow(WINDOW_CastSpell, x, y, width, height, button, hint) {
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    mouse->SetCursorImage("MICON2");
    GameUI_SetStatusBar(LSTR_CHOOSE_TARGET);
}

void GUIMessageQueue::Flush() {
    if (qMessages.size()) {
        GUIMessage message = qMessages.front();
        Clear();
        if (message.field_8 != 0) {
            qMessages.push(message);
        }
    }
}

void GUIMessageQueue::Clear() {
    std::queue<GUIMessage> empty;
    std::swap(qMessages, empty);
}

void GUIMessageQueue::PopMessage(enum UIMessageType *pType, int *pParam,
    int *a4) {
    *pType = (UIMessageType)-1;
    *pParam = 0;
    *a4 = 0;

    if (qMessages.empty()) {
        return;
    }

    GUIMessage message = qMessages.front();
    qMessages.pop();

    *pType = message.eType;
    *pParam = message.param;
    *a4 = message.field_8;
}

void GUI_ReplaceHotkey(GameKey oldKey, GameKey newKey, char bFirstCall) {
    if (bFirstCall) {
        for (GUIWindow *pWindow : lWindowList) {
            for (GUIButton *j : pWindow->vButtons) {
                j->field_28 = 0;
            }
        }
    }

    for (GUIWindow *pWindow : lWindowList) {
        for (GUIButton *j : pWindow->vButtons) {
            if (j->hotkey == oldKey) {
                if (j->field_28 == 0) {
                    j->field_28 = 1;
                    j->hotkey = newKey;
                }
            }
        }
    }
}

GUIButton *GUI_HandleHotkey(GameKey hotkey) {
    for (GUIWindow *pWindow : lWindowList) {
        for (GUIButton *result : pWindow->vButtons) {
            if (result->hotkey == hotkey) {
                pMessageQueue_50CBD0->AddGUIMessage(result->msg, result->msg_param, 0);
                return result;
            }
        }
        if (pWindow->uFrameX == 0 && pWindow->uFrameY == 0 &&
            pWindow->uFrameWidth == window->GetWidth() && pWindow->uFrameHeight == window->GetWidth()) {
            break;
        }
    }
    return 0;
}

//----- (0041D73D) --------------------------------------------------------
void GUIWindow::_41D73D_draw_buff_tooltip() {
    unsigned short text_color;
    int Y_pos;         // esi@11
    int string_count;  // [sp+20h] [bp-4h]@7

    string_count = 0;
    for (int i = 0; i < 20; ++i) {
        if (pParty->pPartyBuffs[i].Active()) ++string_count;
    }

    uFrameHeight = pFontArrus->GetHeight() + 72;
    uFrameHeight += (string_count - 1) * pFontArrus->GetHeight();
    uFrameZ = uFrameWidth + uFrameX - 1;
    uFrameW = uFrameY + uFrameHeight - 1;
    DrawMessageBox(0);
    DrawTitleText(pFontArrus, 0, 12, 0, localization->GetString(LSTR_ACTIVE_PARTY_SPELLS), 3);
    if (!string_count)
        DrawTitleText(pFontComic, 0, 40, 0, localization->GetString(LSTR_NONE), 3);

    string_count = 0;
    for (int i = 0; i < 20; ++i) {
        if (pParty->pPartyBuffs[i].Active()) {
            auto remaing_time =
                pParty->pPartyBuffs[i].expire_time - pParty->GetPlayingTime();
            Y_pos = string_count * pFontComic->GetHeight() + 40;
            text_color =
                Color16(spell_tooltip_colors[i].R, spell_tooltip_colors[i].G,
                    spell_tooltip_colors[i].B);
            DrawText(pFontComic, 52, Y_pos, text_color,
                localization->GetSpellName(i), 0, 0, 0);
            DrawBuff_remaining_time_string(Y_pos, this, remaing_time,
                pFontComic);
            ++string_count;
        }
    }
}

//----- (0041D08F) --------------------------------------------------------
void GUIWindow::_41D08F_set_keyboard_control_group(int num_buttons, int a3,
    int a4, int a5) {
    if (num_buttons) {
        this->pNumPresenceButton = num_buttons;
        this->field_30 = a3;
        this->field_34 = a4;
        this->pCurrentPosActiveItem = a5;
        this->pStartingPosActiveItem = a5;
        this->receives_keyboard_input = true;
    } else {
        this->pNumPresenceButton = 0;
        this->field_30 = a3;
        this->field_34 = a4;
        this->pCurrentPosActiveItem = 0;
        this->pStartingPosActiveItem = 0;
        this->receives_keyboard_input = false;
    }
}

void GUIWindow::Release() {
    if (!this || this->eWindowType == WINDOW_null) {
        // added the check to avoid releasing
        // windows already released
        return;
    }
    DeleteButtons();

    lWindowList.remove(this);

    log->Info("Release window: %s", ToString(eWindowType));
}

void GUIWindow::DeleteButtons() {
    while (vButtons.size()) {
        vButtons.front()->Release();
    }
}

GUIButton *GUIWindow::GetControl(unsigned int uID) {
    if (uID >= vButtons.size()) {
        return nullptr;
    }

    return vButtons[uID];
}

void GUIWindow::DrawMessageBox(bool inside_game_viewport) {
    int x = 0;
    int y = 0;
    int z, w;
    if (inside_game_viewport) {
        x = pViewport->uViewportTL_X;
        z = pViewport->uViewportBR_X;
        y = pViewport->uViewportTL_Y;
        w = pViewport->uViewportBR_Y;
    } else {
        z = window->GetWidth();
        w = window->GetHeight();
    }

    Point cursor = mouse->GetCursorPos();
    if ((int)this->uFrameX >= x) {
        if ((int)(this->uFrameWidth + this->uFrameX) > z) {
            this->uFrameX = z - this->uFrameWidth;
            this->uFrameY = cursor.y + 30;
        }
    } else {
        this->uFrameX = x;
        this->uFrameY = cursor.y + 30;
    }

    if ((int)this->uFrameY >= y) {
        if ((int)(this->uFrameY + this->uFrameHeight) > w) {
            this->uFrameY = cursor.y - this->uFrameHeight - 30;
        }
    } else {
        this->uFrameY = cursor.y + 30;
    }
    if ((signed int)this->uFrameY < y) {
        this->uFrameY = y;
    }
    if ((signed int)this->uFrameX < x) {
        this->uFrameX = x;
    }
    this->uFrameZ = this->uFrameWidth + this->uFrameX - 1;
    this->uFrameW = this->uFrameHeight + this->uFrameY - 1;

    GUIWindow current_window = *this;
    current_window.uFrameX += 12;
    current_window.uFrameWidth -= 24;
    current_window.uFrameY += 12;
    current_window.uFrameHeight -= 12;
    current_window.uFrameZ =
        current_window.uFrameWidth + current_window.uFrameX - 1;
    current_window.uFrameW =
        current_window.uFrameHeight + current_window.uFrameY - 1;
    unsigned int uBoxHeight;
    if (!sHint.empty()) {
        uBoxHeight =
            pFontLucida->CalcTextHeight(sHint, current_window.uFrameWidth, 0) +
            24;
    } else {
        uBoxHeight = uFrameHeight;
    }
    if (uBoxHeight < 64) {
        uBoxHeight = 64;
    }
    if ((int)(uBoxHeight + this->uFrameY) > 479) {
        uBoxHeight = 479 - this->uFrameY;
    }
    DrawPopupWindow(this->uFrameX, this->uFrameY, this->uFrameWidth,
        uBoxHeight);
    if (!sHint.empty()) {
        current_window.DrawTitleText(
            pFontLucida, 0,
            (int)(uBoxHeight -
                pFontLucida->CalcTextHeight(this->sHint,
                    current_window.uFrameWidth, 0)) /
            2 -
            14,
            0, this->sHint, 3);
    }
}

void GUIWindow::HouseDialogManager() {
    if (!window_SpeakInHouse) {
        return;
    }
    GUIWindow pWindow = *this;
    pWindow.uFrameWidth -= 18;
    pWindow.uFrameZ -= 18;
    uint16_t pWhiteColor = Color16(0xFFu, 0xFFu, 0xFFu);
    uint16_t pColor2 = Color16(0x15u, 0x99u, 0xE9u);
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    if (pDialogueNPCCount != uNumDialogueNPCPortraits || !uHouse_ExitPic) {
        const char *pHouseName = p2DEvents[window_SpeakInHouse->par1C - 1].pName;
        if (pHouseName) {
            int v3 = 2 * pFontCreate->GetHeight() - 6 -
                pFontCreate->CalcTextHeight(pHouseName, 130, 0);
            if (v3 < 0) v3 = 0;
            pWindow.DrawTitleText(pFontCreate, 0x1EAu, v3 / 2 + 4, pWhiteColor,
                                  p2DEvents[window_SpeakInHouse->par1C - 1].pName, 3);
        }
    }
    pWindow.uFrameWidth += 8;
    pWindow.uFrameZ += 8;
    if (!pDialogueNPCCount) {
        if (in_current_building_type == BuildingType_Jail) {
            JailDialog();
            if (pDialogueNPCCount == uNumDialogueNPCPortraits &&
                uHouse_ExitPic) {
                render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f,
                    dialogue_ui_x_x_u);
                render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f,
                    dialogue_ui_x_ok_u);
            } else {
                render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
                    ui_exit_cancel_button_background);
            }
            return;
        }
        if (!current_npc_text.empty()) {
            GUIWindow pDialogWindow;
            pDialogWindow.uFrameWidth = 458;
            pDialogWindow.uFrameZ = 457;
            int pTextHeight = pFontArrus->CalcTextHeight(
                current_npc_text, pDialogWindow.uFrameWidth, 13);
            int v6 = pTextHeight + 7;
            render->DrawTextureCustomHeight(8 / 640.0f,
                (352 - (pTextHeight + 7)) / 480.0f,
                ui_leather_mm7, pTextHeight + 7);
            render->DrawTextureAlphaNew(8 / 640.0f, (347 - v6) / 480.0f,
                _591428_endcap);
            window_SpeakInHouse->DrawText(
                pFontArrus, 13, 354 - v6, 0,
                pFontArrus->FitTextInAWindow(current_npc_text,
                    pDialogWindow.uFrameWidth, 13),
                0, 0, 0);
        }
        if (uNumDialogueNPCPortraits <= 0) {
            if (pDialogueNPCCount == uNumDialogueNPCPortraits &&
                uHouse_ExitPic) {
                render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f,
                    dialogue_ui_x_x_u);
                render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f,
                    dialogue_ui_x_ok_u);
            } else {
                render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
                    ui_exit_cancel_button_background);
            }
            return;
        }

        unsigned int v10 = 0;

        for (int v8 = 0; v8 < uNumDialogueNPCPortraits; ++v8) {
            render->DrawTextureAlphaNew(
                (pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v8] - 4) /
                640.0f,
                (pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] - 4) /
                480.0f,
                game_ui_evtnpc);
            render->DrawTextureAlphaNew(
                pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v8] / 640.0f,
                pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] / 480.0f,
                pDialogueNPCPortraits[v8]);
            if (uNumDialogueNPCPortraits < 4) {
                String pTitleText;
                int v9 = 0;
                if (v8 + 1 == uNumDialogueNPCPortraits && uHouse_ExitPic) {
                    pTitleText = pMapStats->pInfos[uHouse_ExitPic].pName;
                    v9 = 94 * v8 + 113;
                } else {
                    if (!v8 && dword_591080) {
                        pTitleText = (char*)p2DEvents[window_SpeakInHouse->par1C - 1].pProprieterTitle;
                        pWindow.DrawTitleText(pFontCreate, 0x1E3u, 113, pColor2, pTitleText, 3);
                        continue;
                    }
                    pTitleText = HouseNPCData[v8 + 1 - (dword_591080 != 0)]->pName;
                    v9 = pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] + pDialogueNPCPortraits[v8]->GetHeight() + 2;
                }
                v10 = v9;
                pWindow.DrawTitleText(pFontCreate, 483, v10, pColor2, pTitleText, 3);
            }
        }
        if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
            render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f,
                dialogue_ui_x_x_u);
            render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f,
                dialogue_ui_x_ok_u);
        } else {
            render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
                ui_exit_cancel_button_background);
        }
        return;
    }

    char *v4 = (char *)pDialogueNPCCount - 1;
    render->DrawTextureAlphaNew((pNPCPortraits_x[0][0] - 4) / 640.0f,
        (pNPCPortraits_y[0][0] - 4) / 480.0f,
        game_ui_evtnpc);
    render->DrawTextureAlphaNew(pNPCPortraits_x[0][0] / 640.0f,
        pNPCPortraits_y[0][0] / 480.0f,
        pDialogueNPCPortraits[(int64_t)v4]);
    if (current_screen_type == CURRENT_SCREEN::SCREEN_E) {
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
            render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f,
                dialogue_ui_x_x_u);
            render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f,
                dialogue_ui_x_ok_u);
        } else {
            render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
                ui_exit_cancel_button_background);
        }
        return;
    }
    if (v4 || !dword_591080) {  // emerald isle ship before quest's done   / на
                                // изумрудном острове заходит на корабле пока не
                                // выполнены квесты
        SimpleHouseDialog();
    } else {
        pWindow.DrawTitleText(
            pFontCreate, 0x1E3u, 0x71u, pColor2,
            NameAndTitle(
                p2DEvents[window_SpeakInHouse->par1C - 1].pProprieterName,
                p2DEvents[window_SpeakInHouse->par1C - 1].pProprieterTitle
            ), 3);
        switch (in_current_building_type) {
        case BuildingType_WeaponShop:
            WeaponShopDialog();
            break;
        case BuildingType_ArmorShop:
            ArmorShopDialog();
            break;
        case BuildingType_MagicShop:
            MagicShopDialog();
            break;
        case BuildingType_AlchemistShop:
            AlchemistDialog();
            break;
        case BuildingType_FireGuild:
        case BuildingType_AirGuild:
        case BuildingType_WaterGuild:
        case BuildingType_EarthGuild:
        case BuildingType_SpiritGuild:
        case BuildingType_MindGuild:
        case BuildingType_BodyGuild:
        case BuildingType_LightGuild:
        case BuildingType_DarkGuild:
            GuildDialog();
            break;
        case BuildingType_18:
            __debugbreak();  // What over the dialog?
            sub_4B6478();
            break;
        case BuildingType_TownHall:
            TownHallDialog();
            break;
        case BuildingType_Tavern:
            TavernDialog();
            break;
        case BuildingType_Bank:
            BankDialog();
            break;
        case BuildingType_Temple:
            TempleDialog();
            break;
        case BuildingType_Stables:
        case BuildingType_Boats:
            TravelByTransport();
            break;
        case BuildingType_Training:
            // __debugbreak(); // param was passed via pTmpBuf, investiage
            // ?? no idea why this could pass an argument - its always reset
            TrainingDialog("");
            break;
        case BuildingType_Jail:
            JailDialog();
            break;
        default:
            // __debugbreak();//New BuildingType (if enter Boat)
            break;
        }
    }
    if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
        render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f,
            dialogue_ui_x_x_u);
        render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f,
            dialogue_ui_x_ok_u);
    } else {
        render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
            ui_exit_cancel_button_background);
    }
}

String MakeDateTimeString(GameTime time) {
    int seconds = time.GetSecondsFraction();
    int minutes = time.GetMinutesFraction();
    int hours = time.GetHoursOfDay();
    int days = time.GetDays();

    String str = "";
    if (days) {
        auto day_str = localization->GetString(LSTR_DAYS);
        if (days <= 1) day_str = localization->GetString(LSTR_DAY_CAPITALIZED);

        str += StringPrintf("%d %s ", days, day_str);
    }

    if (hours) {
        auto hour_str = localization->GetString(LSTR_HOURS);
        if (hours <= 1) hour_str = localization->GetString(LSTR_HOUR);

        str += StringPrintf("%d %s ", hours, hour_str);
    }

    if (minutes && !days) {
        auto minute_str = localization->GetString(LSTR_MINUTES);
        if (minutes <= 1) minute_str = localization->GetString(LSTR_MINUTE);

        str += StringPrintf("%d %s ", minutes, minute_str);
    }

    if (seconds && !hours) {
        auto seconds_str = localization->GetString(LSTR_SECONDS);
        if (seconds <= 1) seconds_str = localization->GetString(LSTR_SECOND);

        str += StringPrintf("%d %s ", seconds, seconds_str);
    }

    return str;
}

//----- (004B1854) --------------------------------------------------------
void GUIWindow::DrawShops_next_generation_time_string(GameTime time) {
    auto str = MakeDateTimeString(time);
    this->DrawTitleText(
        pFontArrus, 0,
        (212 - pFontArrus->CalcTextHeight(str, this->uFrameWidth, 0)) / 2 + 101,
        Color16(0xFFu, 0xFFu, 0x9Bu),
        localization->GetString(LSTR_PLEASE_TRY_BACK_IN) + str, 3
    );
}

void GUIWindow::DrawTitleText(GUIFont *font, int horizontal_margin,
    int vertical_margin,
    unsigned __int16 uDefaultColor, const String &str,
    int line_spacing) {
    this->DrawTitleText(font, horizontal_margin, vertical_margin, uDefaultColor,
        str.c_str(), line_spacing);
}

//----- (0044D406) --------------------------------------------------------
void GUIWindow::DrawTitleText(GUIFont *pFont, int uHorizontalMargin,
    int uVerticalMargin,
    unsigned __int16 uDefaultColor,
    const char *pInString, int uLineSpacing) {
    int width = this->uFrameWidth - uHorizontalMargin;
    ui_current_text_color = uDefaultColor;
    String resString = pFont->FitTextInAWindow(pInString, this->uFrameWidth,
        uHorizontalMargin);
    std::istringstream stream(resString);
    String line;
    int x = uHorizontalMargin + this->uFrameX;
    int y = uVerticalMargin + this->uFrameY;
    while (std::getline(stream, line)) {
        unsigned int x_offset = pFont->AlignText_Center(width, line);
        pFont->DrawTextLine(line, uDefaultColor, x + x_offset, y,
            window->GetWidth());
        y += pFont->GetHeight() - uLineSpacing;
    }
}

void GUIWindow::DrawText(GUIFont *font, int x, int y, unsigned short uFontColor,
    const String &str, bool present_time_transparency,
    int max_text_height, int uFontShadowColor) {
    this->DrawText(font, x, y, uFontColor, str.c_str(),
        present_time_transparency, max_text_height,
        uFontShadowColor);
}

//----- (0044CE08) --------------------------------------------------------
void GUIWindow::DrawText(GUIFont *font, int uX, int uY,
    unsigned short uFontColor, const char *Str,
    bool present_time_transparency, int max_text_height,
    int uFontShadowColor) {
    font->DrawText(this, uX, uY, uFontColor, Str, present_time_transparency,
        max_text_height, uFontShadowColor);
}

int GUIWindow::DrawTextInRect(GUIFont *font, unsigned int x, unsigned int y,
    unsigned int color, const char *text,
    int rect_width, int reverse_text) {
    String label = String(text);
    return DrawTextInRect(font, x, y, color, label, rect_width, reverse_text);
}

//----- (0044CB4F) --------------------------------------------------------
int GUIWindow::DrawTextInRect(GUIFont *pFont, unsigned int uX, unsigned int uY,
    unsigned int uColor, String &str, int rect_width,
    int reverse_text) {
    return pFont->DrawTextInRect(this, uX, uY, uColor, str, rect_width,
        reverse_text);
}

GUIButton *GUIWindow::CreateButton(int uX, int uY, int uWidth, int uHeight,
    int a6, int a7, UIMessageType msg,
    unsigned int msg_param, GameKey hotkey,
    const String &label,
    const std::vector<Image *> &textures) {
    GUIButton *pButton = new GUIButton();

    pButton->pParent = this;
    pButton->uWidth = uWidth;
    pButton->uHeight = uHeight;

    if (a6 == 2 && !uHeight) {
        pButton->uHeight = uWidth;
    }

    pButton->uButtonType = a6;
    pButton->uX = uX + this->uFrameX;
    pButton->uY = uY + this->uFrameY;
    pButton->uZ = pButton->uX + uWidth;
    pButton->uW = pButton->uY + uHeight;
    pButton->field_2C_is_pushed = false;
    pButton->field_1C = a7;
    pButton->msg = msg;
    pButton->msg_param = msg_param;
    pButton->hotkey = hotkey;
    pButton->sLabel = label;
    pButton->vTextures = textures;

    vButtons.push_back(pButton);

    return pButton;
}

bool GUIWindow::Contains(unsigned int x, unsigned int y) {
    return (x >= uFrameX && x <= uFrameZ && y >= uFrameY && y <= uFrameW);
}

void GUIWindow::InitializeGUI() {
    SetUserInterface(PartyAlignment::PartyAlignment_Neutral, false);
    MainMenuUI_LoadFontsAndSomeStuff();
}

void GUIWindow::DrawFlashingInputCursor(int uX, int uY, GUIFont *a2) {
    if (OS_GetTime() % 1000 > 500) {
        DrawText(a2, uX, uY, 0, "_", 0, 0, 0);
    }
}

GUIWindow::GUIWindow() : eWindowType(WINDOW_null) {
    this->mouse = EngineIoc::ResolveMouse();
    this->log = EngineIoc::ResolveLogger();
}

GUIWindow::GUIWindow(WindowType windowType, unsigned int uX, unsigned int uY, unsigned int uWidth,
    unsigned int uHeight, GUIButton *pButton, const String &hint
)
    : eWindowType(windowType) {
    this->mouse = EngineIoc::ResolveMouse();
    this->log = EngineIoc::ResolveLogger();

    log->Info("New window: %s", ToString(windowType));
    lWindowList.push_front(this);
    this->uFrameWidth = uWidth;
    this->uFrameHeight = uHeight;

    this->uFrameX = uX;
    this->uFrameY = uY;
    this->uFrameZ = uX + uWidth - 1;
    this->uFrameW = uY + uHeight - 1;

    this->ptr_1C = (void *)pButton;
    this->sHint = hint;

    this->receives_keyboard_input = false;
}

//----- (004B3EF0) --------------------------------------------------------
void DrawJoinGuildWindow(GUILD_ID guild_id) {
    uDialogueType = DIALOGUE_81_join_guild;
    current_npc_text = (char *)pNPCTopics[guild_id + 99].pText;
    GetJoinGuildDialogueOption(guild_id);
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, 0, 0, window->GetWidth(), 350, (GUIButton *)guild_id);
    pBtn_ExitCancel = pDialogueWindow->CreateButton(
        471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, GameKey::None,
        localization->GetString(LSTR_CANCEL),
        { { ui_exit_cancel_button_background } }
    );
    pDialogueWindow->CreateButton(0, 0, 0, 0, 1, 0,
        UIMSG_BuyInShop_Identify_Repair, 0, GameKey::None, "");
    pDialogueWindow->CreateButton(480, 160, 140, 30, 1, 0, UIMSG_ClickNPCTopic,
        DIALOGUE_82_join_guild, GameKey::None, localization->GetString(LSTR_JOIN));
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

void DialogueEnding() {
    sDialogue_SpeakingActorNPC_ID = 0;
    if (pDialogueWindow) {
        pDialogueWindow->Release();
    }
    pDialogueWindow = nullptr;
    pMiscTimer->Resume();
    pEventTimer->Resume();
}

void GUIWindow_BooksButtonOverlay::Update() {
    GUIButton *pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameY / 640.0f, uFrameX / 480.0f,
        pButton->vTextures[0]);
    viewparams->bRedrawGameUI = true;
}

void GUIWindow_Scroll::Update() {
    CreateScrollWindow();
}

void OnButtonClick::Update() {
    if (bPlaySound) {
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    }
    GUIButton *pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[0]);
    viewparams->bRedrawGameUI = true;
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, 0, 0);
    }
    Release();
}

void OnButtonClick2::Update() {
    if (bPlaySound) {
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    }
    GUIButton *pButton = (GUIButton *)ptr_1C;
    if (pButton->uX >= 0 && pButton->uX <= window->GetWidth()) {
        if (pButton->uY >= 0 && pButton->uY <= window->GetHeight()) {
            render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[0]);
        }
    }
    viewparams->bRedrawGameUI = true;
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, 0, 0);
    }
    Release();
}

void OnButtonClick3::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    }
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f,
        pButton->vTextures[1]);
    viewparams->bRedrawGameUI = 1;
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, 0, 0);
    }
    Release();
}

void OnButtonClick4::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    }
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f,
        pButton->vTextures[1]);
    viewparams->bRedrawGameUI = true;

    Release();
}

void OnSaveLoad::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    }
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f,
        pButton->vTextures[0]);
    viewparams->bRedrawGameUI = true;
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, 0, 0);
    }
    Release();

    if (current_screen_type == CURRENT_SCREEN::SCREEN_SAVEGAME) {
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveGame, 0, 0);
    } else {
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_LoadGame, 0, 0);
    }
}

void OnCancel::Update() {
    if (sHint.empty()) {
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    }
    auto pGUIButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f,
        pGUIButton->vTextures[0]);
    viewparams->bRedrawGameUI = true;
    if (!sHint.empty()) {
        pGUIButton->DrawLabel(sHint, pFontCreate, 0, 0);
    }
    Release();

    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
}

void OnCancel2::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    }
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[1]);
    viewparams->bRedrawGameUI = true;
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, 0, 0);
    }
    Release();

    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
}

void OnCancel3::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    }

    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[0]);
    viewparams->bRedrawGameUI = true;
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, 0, 0);
    }
    Release();

    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
}

void GUI_UpdateWindows() {
    if (GetCurrentMenuID() != MENU_CREATEPARTY) {
        extern bool UI_OnKeyDown(GameKey key);
        UI_OnKeyDown(GameKey::PageDown);
    }

    std::list<GUIWindow *> tmpWindowList(lWindowList);
    tmpWindowList.reverse();  // new windows are push front - but front should be drawn last?? testing
    for (GUIWindow *pWindow : tmpWindowList) {
        pWindow->Update();
    }

    if (GetCurrentMenuID() == -1) {
        GameUI_DrawFoodAndGold();
    }
    if (sub_4637E0_is_there_popup_onscreen()) {
        std::shared_ptr<Mouse> mouse = EngineIoc::ResolveMouse();
        UI_OnMouseRightClick(mouse->GetCursorPos().x, mouse->GetCursorPos().y);
    }
}

void CreateScrollWindow() {
    GUIWindow a1 = *pGUIWindow_ScrollWindow;
    a1.sHint.clear();
    a1.uFrameX = 1;
    a1.uFrameY = 1;
    a1.uFrameWidth = 468;
    unsigned int v0 =
        pFontSmallnum->CalcTextHeight(pScrolls[pGUIWindow_ScrollWindow->par1C],
            a1.uFrameWidth, 0) +
        2 * (unsigned char)pFontCreate->GetHeight() + 24;
    a1.uFrameHeight = v0;
    if ((v0 + a1.uFrameY) > 479) {
        v0 = 479 - a1.uFrameY;
        a1.uFrameHeight = 479 - a1.uFrameY;
    }
    a1.uFrameZ = a1.uFrameWidth + a1.uFrameX - 1;
    a1.uFrameW = v0 + a1.uFrameY - 1;
    a1.DrawMessageBox(0);
    a1.uFrameX += 12;
    a1.uFrameWidth -= 24;
    a1.uFrameY += 12;
    a1.uFrameHeight -= 12;
    a1.uFrameZ = a1.uFrameWidth + a1.uFrameX - 1;
    a1.uFrameW = a1.uFrameHeight + a1.uFrameY - 1;

    char *v1 = pItemsTable->pItems[(uint64_t)pGUIWindow_ScrollWindow->ptr_1C + 700].pName;

    a1.DrawTitleText(
        pFontCreate, 0, 0, 0,
        StringPrintf(format_4E2D80, Color16(0xFFu, 0xFFu, 0x9Bu), v1), 3);
    a1.DrawText(pFontSmallnum, 1, pFontCreate->GetHeight() - 3, 0,
        pScrolls[(uint64_t)pGUIWindow_ScrollWindow->ptr_1C], 0, 0,
        0);
}

//----- (00467F48) --------------------------------------------------------
void CreateMsgScrollWindow(signed int mscroll_id) {
    if (!pGUIWindow_ScrollWindow && mscroll_id >= 700) {
        if (mscroll_id <= 782) {
            pGUIWindow_ScrollWindow =
                new GUIWindow_Scroll(0, 0, window->GetWidth(),
                    window->GetHeight(), (GUIButton *)(mscroll_id - 700), "");
        }
    }
}

//----- (00467F9F) --------------------------------------------------------
void free_book_subwindow() {
    if (pGUIWindow_ScrollWindow) {
        pGUIWindow_ScrollWindow->Release();
        pGUIWindow_ScrollWindow = 0;
    }
}
//----- (004226EF) --------------------------------------------------------
void SetUserInterface(PartyAlignment align, bool bReplace) {
    extern void set_default_ui_skin();
    set_default_ui_skin();

    if (!parchment) {
        parchment = assets->GetImage_ColorKey("parchment", 0x7FF);
    }

    if (align == PartyAlignment::PartyAlignment_Evil) {
        if (bReplace) {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-C.pcx");
            game_ui_bottomframe =
                assets->GetImage_PCXFromIconsLOD("ib-b-C.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-C.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-C.pcx");
            game_ui_statusbar =
                assets->GetImage_PCXFromIconsLOD("IB-Foot-c.pcx");

            game_ui_right_panel_frame = assets->GetImage_Alpha("ib-mb-C");

            game_ui_minimap_frame = assets->GetImage_Alpha("ib-autmask-c");
            game_ui_minimap_compass =
                assets->GetImage_ColorKey("IB-COMP-C", 0x7FF);

            game_ui_player_alert_green = assets->GetImage_Alpha("IB-InitG-c");
            game_ui_player_alert_yellow = assets->GetImage_Alpha("IB-InitY-c");
            game_ui_player_alert_red = assets->GetImage_Alpha("IB-InitR-c");

            ui_btn_npc_left = assets->GetImage_ColorKey("IB-NPCLD-C", 0x7FF);
            ui_btn_npc_right = assets->GetImage_ColorKey("IB-NPCRD-C", 0x7FF);
            game_ui_btn_zoomin = assets->GetImage_Alpha("ib-autout-C");
            game_ui_btn_zoomout = assets->GetImage_Alpha("ib-autin-C");
            game_ui_player_selection_frame =
                assets->GetImage_ColorKey("IB-selec-C", 0x7FF);
            game_ui_btn_cast = assets->GetImage_Alpha("ib-m1d-c");
            game_ui_btn_rest = assets->GetImage_Alpha("ib-m2d-c");
            game_ui_btn_quickref = assets->GetImage_Alpha("ib-m3d-c");
            game_ui_btn_settings = assets->GetImage_Alpha("ib-m4d-c");

            game_ui_playerbuff_bless =
                assets->GetImage_ColorKey("isg-01-c", 0x7FF);
            game_ui_playerbuff_preservation =
                assets->GetImage_ColorKey("isg-02-c", 0x7FF);
            game_ui_playerbuff_hammerhands =
                assets->GetImage_ColorKey("isg-03-c", 0x7FF);
            game_ui_playerbuff_pain_reflection =
                assets->GetImage_ColorKey("isg-04-c", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeC");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);

            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchC");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background =
                assets->GetImage_ColorKey("ib-bcu-c", 0x7FF);

            game_ui_evtnpc = assets->GetImage_ColorKey("evtnpc-c", 0x7FF);
            ui_character_inventory_background =
                assets->GetImage_ColorKey("fr_inven-c", 0x7FF);
            messagebox_corner_y = assets->GetImage_Alpha("cornr_ll-c");
            messagebox_corner_w = assets->GetImage_Alpha("cornr_lr-c");
            messagebox_corner_x = assets->GetImage_Alpha("cornr_ul-c");
            messagebox_corner_z = assets->GetImage_Alpha("cornr_ur-c");
            messagebox_border_bottom = assets->GetImage_Alpha("edge_btm-c");
            messagebox_border_left = assets->GetImage_Alpha("edge_lf-c");
            messagebox_border_right = assets->GetImage_Alpha("edge_rt-c");
            messagebox_border_top = assets->GetImage_Alpha("edge_top-c");
            _591428_endcap = assets->GetImage_ColorKey("endcap-c", 0x7FF);
        } else {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-C.pcx");
            game_ui_bottomframe =
                assets->GetImage_PCXFromIconsLOD("ib-b-c.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-C.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-C.pcx");
            game_ui_statusbar =
                assets->GetImage_PCXFromIconsLOD("IB-Foot-c.pcx");

            game_ui_right_panel_frame = assets->GetImage_Alpha("ib-mb-C");
            game_ui_minimap_frame = assets->GetImage_Alpha("ib-autmask-c");
            game_ui_minimap_compass =
                assets->GetImage_ColorKey("IB-COMP-C", 0x7FF);
            game_ui_player_alert_green = assets->GetImage_Alpha("IB-InitG-c");
            game_ui_player_alert_yellow = assets->GetImage_Alpha("IB-InitY-c");
            game_ui_player_alert_red = assets->GetImage_Alpha("IB-InitR-c");

            ui_btn_npc_left = assets->GetImage_ColorKey("IB-NPCLD-C", 0x7FF);
            ui_btn_npc_right = assets->GetImage_ColorKey("IB-NPCRD-C", 0x7FF);
            game_ui_btn_zoomin =
                assets->GetImage_ColorKey("ib-autout-C", 0x7FF);
            game_ui_btn_zoomout =
                assets->GetImage_ColorKey("ib-autin-C", 0x7FF);
            game_ui_player_selection_frame =
                assets->GetImage_ColorKey("IB-selec-C", 0x7FF);
            game_ui_btn_cast = assets->GetImage_Alpha("ib-m1d-c");
            game_ui_btn_rest = assets->GetImage_Alpha("ib-m2d-c");
            game_ui_btn_quickref = assets->GetImage_Alpha("ib-m3d-c");
            game_ui_btn_settings = assets->GetImage_Alpha("ib-m4d-c");
            ui_exit_cancel_button_background =
                assets->GetImage_ColorKey("ib-bcu-c", 0x7FF);

            game_ui_playerbuff_bless =
                assets->GetImage_ColorKey("isg-01-c", 0x7FF);
            game_ui_playerbuff_preservation =
                assets->GetImage_ColorKey("isg-02-c", 0x7FF);
            game_ui_playerbuff_hammerhands =
                assets->GetImage_ColorKey("isg-03-c", 0x7FF);
            game_ui_playerbuff_pain_reflection =
                assets->GetImage_ColorKey("isg-04-c", 0x7FF);

            game_ui_evtnpc = assets->GetImage_ColorKey("evtnpc-c", 0x7FF);
            ui_character_inventory_background =
                assets->GetImage_ColorKey("fr_inven", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeC");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);

            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchC");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);
        }
        uGameUIFontMain = Color16(0xC8u, 0, 0);
        uGameUIFontShadow = Color16(10, 0, 0);
    } else if (align == PartyAlignment::PartyAlignment_Neutral) {
        if (bReplace) {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-a.pcx");
            game_ui_bottomframe =
                assets->GetImage_PCXFromIconsLOD("ib-b-a.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-a.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-a.pcx");
            game_ui_statusbar =
                assets->GetImage_PCXFromIconsLOD("IB-Foot-a.pcx");

            game_ui_right_panel_frame = assets->GetImage_Alpha("ib-mb-a");
            game_ui_minimap_frame = assets->GetImage_Alpha("ib-autmask-a");
            game_ui_minimap_compass =
                assets->GetImage_ColorKey("IB-COMP-a", 0x7FF);
            game_ui_player_alert_green = assets->GetImage_Alpha("IB-InitG-a");
            game_ui_player_alert_yellow = assets->GetImage_Alpha("IB-InitY-a");
            game_ui_player_alert_red = assets->GetImage_Alpha("IB-InitR-a");

            ui_btn_npc_left = assets->GetImage_ColorKey("IB-NPCLD-a", 0x7FF);
            ui_btn_npc_right = assets->GetImage_ColorKey("IB-NPCRD-a", 0x7FF);
            game_ui_btn_zoomin =
                assets->GetImage_ColorKey("ib-autout-a", 0x7FF);
            game_ui_btn_zoomout =
                assets->GetImage_ColorKey("ib-autin-a", 0x7FF);
            game_ui_player_selection_frame =
                assets->GetImage_ColorKey("IB-selec-a", 0x7FF);
            game_ui_btn_cast = assets->GetImage_Alpha("ib-m1d-a");
            game_ui_btn_rest = assets->GetImage_Alpha("ib-m2d-a");
            game_ui_btn_quickref = assets->GetImage_Alpha("ib-m3d-a");
            game_ui_btn_settings = assets->GetImage_Alpha("ib-m4d-a");

            game_ui_playerbuff_bless =
                assets->GetImage_ColorKey("isg-01-a", 0x7FF);
            game_ui_playerbuff_preservation =
                assets->GetImage_ColorKey("isg-02-a", 0x7FF);
            game_ui_playerbuff_hammerhands =
                assets->GetImage_ColorKey("isg-03-a", 0x7FF);
            game_ui_playerbuff_pain_reflection =
                assets->GetImage_ColorKey("isg-04-a", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeA");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchA");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background =
                assets->GetImage_ColorKey("ib-bcu-a", 0x7FF);

            game_ui_evtnpc = assets->GetImage_ColorKey("evtnpc", 0x7FF);
            ui_character_inventory_background =
                assets->GetImage_ColorKey("fr_inven", 0x7FF);
            messagebox_corner_y = assets->GetImage_Alpha("cornr_ll");
            messagebox_corner_w = assets->GetImage_Alpha("cornr_lr");
            messagebox_corner_x = assets->GetImage_Alpha("cornr_ul");
            messagebox_corner_z = assets->GetImage_Alpha("cornr_ur");
            messagebox_border_bottom = assets->GetImage_Alpha("edge_btm");
            messagebox_border_left = assets->GetImage_Alpha("edge_lf");
            messagebox_border_right = assets->GetImage_Alpha("edge_rt");
            messagebox_border_top = assets->GetImage_Alpha("edge_top");
            _591428_endcap = assets->GetImage_ColorKey("endcap", 0x7FF);
        } else {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-A.pcx");
            game_ui_bottomframe =
                assets->GetImage_PCXFromIconsLOD("ib-b-A.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-A.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-A.pcx");
            game_ui_statusbar =
                assets->GetImage_PCXFromIconsLOD("IB-Foot-a.pcx");

            game_ui_right_panel_frame = assets->GetImage_Alpha("ib-mb-A");
            game_ui_minimap_frame = assets->GetImage_Alpha("ib-autmask-a");
            game_ui_minimap_compass =
                assets->GetImage_ColorKey("IB-COMP-A", 0x7FF);
            game_ui_player_alert_green = assets->GetImage_Alpha("IB-InitG-a");
            game_ui_player_alert_yellow = assets->GetImage_Alpha("IB-InitY-a");
            game_ui_player_alert_red = assets->GetImage_Alpha("IB-InitR-a");

            ui_btn_npc_left = assets->GetImage_ColorKey("IB-NPCLD-A", 0x7FF);
            ui_btn_npc_right = assets->GetImage_ColorKey("IB-NPCRD-A", 0x7FF);
            game_ui_player_selection_frame =
                assets->GetImage_ColorKey("IB-selec-A", 0x7FF);
            game_ui_btn_cast = assets->GetImage_Alpha("ib-m1d-a");
            game_ui_btn_rest = assets->GetImage_Alpha("ib-m2d-a");
            game_ui_btn_quickref = assets->GetImage_Alpha("ib-m3d-a");
            game_ui_btn_settings = assets->GetImage_Alpha("ib-m4d-a");
            game_ui_btn_zoomin =
                assets->GetImage_ColorKey("ib-autout-a", 0x7FF);
            game_ui_btn_zoomout =
                assets->GetImage_ColorKey("ib-autin-a", 0x7FF);
            ui_exit_cancel_button_background =
                assets->GetImage_ColorKey("ib-bcu-a", 0x7FF);

            game_ui_playerbuff_bless =
                assets->GetImage_ColorKey("isg-01-a", 0x7FF);
            game_ui_playerbuff_preservation =
                assets->GetImage_ColorKey("isg-02-a", 0x7FF);
            game_ui_playerbuff_hammerhands =
                assets->GetImage_ColorKey("isg-03-a", 0x7FF);
            game_ui_playerbuff_pain_reflection =
                assets->GetImage_ColorKey("isg-04-a", 0x7FF);

            game_ui_evtnpc = assets->GetImage_ColorKey("evtnpc", 0x7FF);
            ui_character_inventory_background =
                assets->GetImage_ColorKey("fr_inven", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeA");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchA");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            messagebox_corner_y = assets->GetImage_Alpha("cornr_ll");
            messagebox_corner_w = assets->GetImage_Alpha("cornr_lr");
            messagebox_corner_x = assets->GetImage_Alpha("cornr_ul");
            messagebox_corner_z = assets->GetImage_Alpha("cornr_ur");
            messagebox_border_bottom = assets->GetImage_Alpha("edge_btm");
            messagebox_border_left = assets->GetImage_Alpha("edge_lf");
            messagebox_border_right = assets->GetImage_Alpha("edge_rt");
            messagebox_border_top = assets->GetImage_Alpha("edge_top");
            _591428_endcap = assets->GetImage_ColorKey("endcap", 0x7FF);
        }
        uGameUIFontMain = Color16(0xAu, 0, 0);
        uGameUIFontShadow = Color16(230, 214, 193);
    } else if (align == PartyAlignment::PartyAlignment_Good) {
        if (bReplace) {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-B.pcx");
            game_ui_bottomframe =
                assets->GetImage_PCXFromIconsLOD("ib-b-B.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-B.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-B.pcx");
            game_ui_statusbar =
                assets->GetImage_PCXFromIconsLOD("IB-Foot-b.pcx");

            game_ui_right_panel_frame = assets->GetImage_Alpha("ib-mb-B");
            game_ui_minimap_frame = assets->GetImage_Alpha("ib-autmask-b");
            game_ui_minimap_compass =
                assets->GetImage_ColorKey("IB-COMP-B", 0x7FF);
            game_ui_player_alert_green = assets->GetImage_Alpha("IB-InitG-b");
            game_ui_player_alert_yellow = assets->GetImage_Alpha("IB-InitY-b");
            game_ui_player_alert_red = assets->GetImage_Alpha("IB-InitR-b");

            ui_btn_npc_left = assets->GetImage_ColorKey("IB-NPCLD-B", 0x7FF);
            ui_btn_npc_right = assets->GetImage_ColorKey("IB-NPCRD-B", 0x7FF);
            game_ui_btn_zoomin =
                assets->GetImage_ColorKey("ib-autout-B", 0x7FF);
            game_ui_btn_zoomout =
                assets->GetImage_ColorKey("ib-autin-B", 0x7FF);
            game_ui_player_selection_frame =
                assets->GetImage_ColorKey("IB-selec-B", 0x7FF);
            game_ui_btn_cast = assets->GetImage_Alpha("ib-m1d-b");
            game_ui_btn_rest = assets->GetImage_Alpha("ib-m2d-b");
            game_ui_btn_quickref = assets->GetImage_Alpha("ib-m3d-b");
            game_ui_btn_settings = assets->GetImage_Alpha("ib-m4d-b");

            game_ui_playerbuff_bless =
                assets->GetImage_ColorKey("isg-01-b", 0x7FF);
            game_ui_playerbuff_preservation =
                assets->GetImage_ColorKey("isg-02-b", 0x7FF);
            game_ui_playerbuff_hammerhands =
                assets->GetImage_ColorKey("isg-03-b", 0x7FF);
            game_ui_playerbuff_pain_reflection =
                assets->GetImage_ColorKey("isg-04-b", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeB");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchB");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background =
                assets->GetImage_ColorKey("ib-bcu-b", 0x7FF);
            game_ui_evtnpc = assets->GetImage_ColorKey("evtnpc-b", 0x7FF);
            ui_character_inventory_background =
                assets->GetImage_ColorKey("fr_inven-b", 0x7FF);
            messagebox_corner_y = assets->GetImage_Alpha("cornr_ll-b");
            messagebox_corner_w = assets->GetImage_Alpha("cornr_lr-b");
            messagebox_corner_x = assets->GetImage_Alpha("cornr_ul-b");
            messagebox_corner_z = assets->GetImage_Alpha("cornr_ur-b");
            messagebox_border_bottom = assets->GetImage_Alpha("edge_btm-b");
            messagebox_border_left = assets->GetImage_Alpha("edge_lf-b");
            messagebox_border_right = assets->GetImage_Alpha("edge_rt-b");
            messagebox_border_top = assets->GetImage_Alpha("edge_top-b");
            _591428_endcap = assets->GetImage_ColorKey("endcap-b", 0x7FF);
        }
        uGameUIFontMain = Color16(0, 0, 0xC8u);
        uGameUIFontShadow = Color16(255, 255, 255);
    } else {
        Error("Invalid alignment type: %u", align);
    }
}

void DrawBuff_remaining_time_string(int uY, GUIWindow *window, GameTime remaining_time,
    GUIFont *Font) {
    window->DrawText(Font, 32, uY, 0, "\r020" + MakeDateTimeString(remaining_time), 0, 0, 0);
}

void GUIMessageQueue::AddMessageImpl(UIMessageType msg, int param,
    unsigned int a4, const char *file,
    int line) {
    // logger->Warning("%s @ (%S %u)", UIMessage2String(msg), file, line);
    GUIMessage message;
    message.eType = msg;
    message.param = param;
    message.field_8 = a4;
    message.file = file;
    message.line = line;
    qMessages.push(message);
}

char sub_4637E0_is_there_popup_onscreen() {
    return dword_507BF0_is_there_popup_onscreen == 1;
}

//----- (00417AD4) --------------------------------------------------------
unsigned int GetSkillColor(unsigned int uPlayerClass,
    PLAYER_SKILL_TYPE uPlayerSkillType,
    signed int skill_level) {
    switch (uPlayerClass % 4) {
    case 0:
    {
        if (byte_4ED970_skill_learn_ability_by_class_table
            [uPlayerClass][uPlayerSkillType] >= skill_level)
            return ui_character_skillinfo_can_learn;
        if (byte_4ED970_skill_learn_ability_by_class_table
            [uPlayerClass + 1][uPlayerSkillType] < skill_level &&
            byte_4ED970_skill_learn_ability_by_class_table
            [uPlayerClass + 2][uPlayerSkillType] < skill_level) {
            if (byte_4ED970_skill_learn_ability_by_class_table
                [uPlayerClass + 3][uPlayerSkillType] < skill_level)
                return ui_character_skillinfo_cant_learn;
        }
        return ui_character_skillinfo_can_learn_gm;
    } break;

    case 1:
    {
        if (byte_4ED970_skill_learn_ability_by_class_table
            [uPlayerClass][uPlayerSkillType] >= skill_level)
            return ui_character_skillinfo_can_learn;
        if (byte_4ED970_skill_learn_ability_by_class_table
            [uPlayerClass + 1][uPlayerSkillType] < skill_level) {
            if (byte_4ED970_skill_learn_ability_by_class_table
                [uPlayerClass + 2][uPlayerSkillType] < skill_level)
                return ui_character_skillinfo_cant_learn;
        }
        return ui_character_skillinfo_can_learn_gm;
    } break;

    case 2:
    case 3:
    {
        if (byte_4ED970_skill_learn_ability_by_class_table
            [uPlayerClass][uPlayerSkillType] < skill_level)
            return ui_character_skillinfo_cant_learn;
        return ui_character_skillinfo_can_learn;
    } break;
    }
    Error("Invalid player class: %u", uPlayerClass);
}

void ClickNPCTopic(DIALOGUE_TYPE topic) {
    int pEventNumber;  // ecx@8
    Player *v4;        // esi@20
    char *v12;         // eax@53
    char *v13;         // eax@56
    char *v14;         // eax@57
    char *v15;         // eax@58
    int pPrice;        // ecx@70
    char *v22;         // [sp-Ch] [bp-18h]@73
    char *v24;         // [sp-8h] [bp-14h]@73

    uDialogueType = (DIALOGUE_TYPE)(topic + 1);
    NPCData *pCurrentNPCInfo =
        HouseNPCData[pDialogueNPCCount - ((dword_591080 != 0) ? 1 : 0)];  //- 1
    if (topic <= DIALOGUE_SCRIPTED_LINE_6) {
        switch (topic) {
        case DIALOGUE_13_hiring_related:
            current_npc_text = BuildDialogueString(
                pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                uActiveCharacter - 1, 0, 0, 0
            );
            NPCHireableDialogPrepare();
            dialogue_show_profession_details = false;
            BackToHouseMenu();
            return;
        case DIALOGUE_SCRIPTED_LINE_1:
            pEventNumber = pCurrentNPCInfo->dialogue_1_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_2:
            pEventNumber = pCurrentNPCInfo->dialogue_2_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_3:
            pEventNumber = pCurrentNPCInfo->dialogue_3_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_4:
            pEventNumber = pCurrentNPCInfo->dialogue_4_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_5:
            pEventNumber = pCurrentNPCInfo->dialogue_5_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_6:
            pEventNumber = pCurrentNPCInfo->dialogue_6_evt_id;
            break;
        default:
            BackToHouseMenu();
            return;
        }

        if (pEventNumber < 200 || pEventNumber > 310) {
            if (pEventNumber < 400 || pEventNumber > 410) {
                if (pEventNumber == 139) {
                    OracleDialogue();
                } else {
                    if (pEventNumber == 311) {
                        CheckBountyRespawnAndAward();
                    } else {
                        current_npc_text.clear();
                        activeLevelDecoration = (LevelDecoration *)1;
                        EventProcessor(pEventNumber, 0, 1);
                        activeLevelDecoration = nullptr;
                    }
                }
            } else {
                _dword_F8B1D8_last_npc_topic_menu = topic;
                DrawJoinGuildWindow((GUILD_ID)(pEventNumber - 400));
            }
        } else {
            _4B3FE5_training_dialogue(pEventNumber);
        }
        BackToHouseMenu();
        return;
    }
    if (topic != DIALOGUE_HIRE_FIRE) {
        if (topic == DIALOGUE_PROFESSION_DETAILS) {
            // uBoxHeight = pCurrentNPCInfo->uProfession;
            __debugbreak();  // probably hirelings found in buildings, not
                             // present in MM7, changed
                             // "pCurrentNPCInfo->uProfession - 1" to
                             // "pCurrentNPCInfo->uProfession", have to check in
                             // other versions whether it's ok
            if (dialogue_show_profession_details) {
                current_npc_text = BuildDialogueString(
                    pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                    uActiveCharacter - 1, 0, 0, 0);
            } else {
                current_npc_text = BuildDialogueString(
                    pNPCStats->pProfessions[pCurrentNPCInfo->profession].pBenefits,
                    uActiveCharacter - 1, 0, 0, 0);
            }
            dialogue_show_profession_details = ~dialogue_show_profession_details;
        } else {
            if (topic == DIALOGUE_79_mastery_teacher) {
                if (guild_membership_approved) {
                    Party::TakeGold(gold_transaction_amount);
                    if (uActiveCharacter) {
                        v12 = (char *)&pPlayers[uActiveCharacter]
                            ->pActiveSkills[dword_F8B1AC_award_bit_number];
                        *(short *)v12 &= 0x3Fu;
                        switch (dword_F8B1B0_MasteryBeingTaught) {
                        case 2:
                            v15 = (char *)&pPlayers[uActiveCharacter]
                                ->pActiveSkills[dword_F8B1AC_award_bit_number];
                            *v15 |= 0x40u;
                            break;
                        case 3:
                            v14 = (char *)&pPlayers[uActiveCharacter]
                                ->pActiveSkills[dword_F8B1AC_award_bit_number];
                            *v14 |= 0x80u;
                            break;
                        case 4:
                            v13 = (char *)&pPlayers[uActiveCharacter]
                                ->pActiveSkills[dword_F8B1AC_award_bit_number];
                            v13[1] |= 1u;
                            break;
                        }
                        pPlayers[uActiveCharacter]->PlaySound(SPEECH_85, 0);
                    }
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
                }
            } else {
                if (topic == DIALOGUE_82_join_guild && guild_membership_approved) {
                    // join guild
                    Party::TakeGold(gold_transaction_amount);
                    v4 = pParty->pPlayers.data();
                    do {
                        v4->SetVariable(VAR_Award, dword_F8B1AC_award_bit_number);
                        ++v4;
                    } while ((int64_t)v4 < (int64_t)pParty->pHirelings.data());
                    switch (_dword_F8B1D8_last_npc_topic_menu) {
                    case DIALOGUE_SCRIPTED_LINE_1:
                        pEventNumber = pCurrentNPCInfo->dialogue_1_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_1_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_2:
                        pEventNumber = pCurrentNPCInfo->dialogue_2_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_2_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_3:
                        pEventNumber = pCurrentNPCInfo->dialogue_3_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_3_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_4:
                        pEventNumber = pCurrentNPCInfo->dialogue_4_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_4_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_5:
                        pEventNumber = pCurrentNPCInfo->dialogue_5_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_5_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_6:
                        pEventNumber = pCurrentNPCInfo->dialogue_6_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_6_evt_id = 0;
                        break;
                    }
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
                    if (uActiveCharacter) {
                        pPlayers[uActiveCharacter]->PlaySound(SPEECH_86, 0);
                        BackToHouseMenu();
                        return;
                    }
                }
            }
        }
        BackToHouseMenu();
        return;
    }

    if (pParty->pHirelings[0].pName && pParty->pHirelings[1].pName) {
        GameUI_SetStatusBar(LSTR_HIRE_NO_ROOM);
        BackToHouseMenu();
        return;
    }

    if (pCurrentNPCInfo->profession != Burglar) {  // burglars have no hiring price
        __debugbreak();  // probably hirelings found in buildings, not present
                         // in MM7, changed "pCurrentNPCInfo->uProfession - 1"
                         // to "pCurrentNPCInfo->uProfession", have to check in
                         // other versions whether it's ok
        pPrice =
            pNPCStats->pProfessions[pCurrentNPCInfo->profession].uHirePrice;
        if (pParty->GetGold() < (unsigned int)pPrice) {
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            dialogue_show_profession_details = false;
            uDialogueType = DIALOGUE_13_hiring_related;
            current_npc_text = BuildDialogueString(
                pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                uActiveCharacter - 1, 0, 0, 0);
            if (uActiveCharacter)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_NotEnoughGold, 0);
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            BackToHouseMenu();
            return;
        } else {
            Party::TakeGold(pPrice);
        }
    }

    pCurrentNPCInfo->uFlags |= 128;
    pParty->hirelingScrollPosition = 0;
    pParty->CountHirelings();
    if (pParty->pHirelings[0].pName) {
        memcpy(&pParty->pHirelings[1], pCurrentNPCInfo, sizeof(pParty->pHirelings[1]));
        v24 = pCurrentNPCInfo->pName;
        v22 = pParty->pHireling2Name;
    } else {
        memcpy(&pParty->pHirelings[0], pCurrentNPCInfo, sizeof(pParty->pHirelings[0]));
        v24 = pCurrentNPCInfo->pName;
        v22 = pParty->pHireling1Name;
    }
    strcpy(v22, v24);
    pParty->hirelingScrollPosition = 0;
    pParty->CountHirelings();
    PrepareHouse((HOUSE_ID)(int64_t)window_SpeakInHouse->ptr_1C);
    dialog_menu_id = DIALOGUE_MAIN;

    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
    if (uActiveCharacter)
        pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)61, 0);

    BackToHouseMenu();
}


void _4B3FE5_training_dialogue(int a4) {
    uDialogueType = DIALOGUE_SKILL_TRAINER;
    current_npc_text = String(pNPCTopics[a4 + 168].pText);
    _4B254D_SkillMasteryTeacher(a4);  // checks whether the facility can be used
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, 0, 0, window->GetWidth(), 350, (GUIButton *)a4);
    pBtn_ExitCancel = pDialogueWindow->CreateButton(
        471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, GameKey::None,
        localization->GetString(LSTR_CANCEL), { { ui_exit_cancel_button_background } }
    );
    pDialogueWindow->CreateButton(0, 0, 0, 0, 1, 0,
        UIMSG_BuyInShop_Identify_Repair, 0, GameKey::None, "");
    pDialogueWindow->CreateButton(
        480, 160, 0x8Cu, 0x1Eu, 1, 0, UIMSG_ClickNPCTopic,
        DIALOGUE_79_mastery_teacher, GameKey::None,
        guild_membership_approved ? localization->GetString(LSTR_LEARN) : "");
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

//----- (004B1ECE) --------------------------------------------------------
void OracleDialogue() {
    __int16 *v0;     // edi@1
    signed int v4;   // eax@9
    int v5;          // ebx@11
    signed int v8;   // edi@14
    ItemGen *v9;     // [sp+Ch] [bp-Ch]@11
    signed int v10;  // [sp+10h] [bp-8h]@13
    int v11;         // [sp+14h] [bp-4h]@1

    guild_membership_approved = 0;
    v11 = 0;
    uDialogueType = DIALOGUE_84_oracle;
    current_npc_text = (char *)pNPCTopics[667].pText;
    v0 = _4F0882_evt_VAR_PlayerItemInHands_vals.data();

    for (uint i = 0; i <= 53; i++) {
        if ((unsigned __int16)_449B57_test_bit(pParty->_quest_bits, *v0)) {
            for (uint pl = 0; pl < 4; pl++) {
                if (pParty->pPlayers[pl].CompareVariable(VAR_PlayerItemInHands, *(v0 + 1)))
                    break;
            }
        }
        ++v11;
    }
    if (v0 <= &_4F0882_evt_VAR_PlayerItemInHands_vals[53]) {
        current_npc_text = (char *)pNPCTopics[666].pText;  // Here's %s that you lost. Be careful
        v4 = _4F0882_evt_VAR_PlayerItemInHands_vals[2 * v11];
        guild_membership_approved = _4F0882_evt_VAR_PlayerItemInHands_vals[2 * v11];
        pParty->pPlayers[0].AddVariable(VAR_PlayerItemInHands, v4);
    }
    if (guild_membership_approved == ITEM_LICH_JAR_FULL) {
        v5 = 0;
        // v12 = pParty->pPlayers.data();//[0].uClass;
        v9 = 0;
        // while ( 1 )
        for (uint i = 0; i < 4; i++) {
            if (pParty->pPlayers[i].classType == PLAYER_CLASS_LICH) {
                v10 = 0;
                // v6 =
                // pParty->pPlayers.data();//[0].pInventoryItems[0].field_1A;
                for (uint pl = 0; pl < 4; pl++) {
                    for (v8 = 0; v8 < 126; v8++) {  // 138
                        if (pParty->pPlayers[pl].pInventoryItemList[v8].uItemID == ITEM_LICH_JAR_FULL) {
                            if (!pParty->pPlayers[pl].pInventoryItemList[v8].uHolderPlayer)
                                v9 = &pParty->pPlayers[pl].pInventoryItemList[v8];
                            if (pParty->pPlayers[pl].pInventoryItemList[v8].uHolderPlayer == v5)
                                v10 = 1;
                        }
                    }
                }
                if (!v10) break;
            }
            //      ++v12;
            ++v5;
            //  if ( v12 > &pParty->pPlayers[3] )
            //  return;
        }
        if (v9) v9->uHolderPlayer = v5;
    }
}

void CheckBountyRespawnAndAward() {
    int i;                // eax@2
    int rand_monster_id;  // edx@3

    uDialogueType = DIALOGUE_83_bounty_hunting;
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, 0, 0, window->GetWidth(), 350, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton(
        471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, GameKey::None,
        localization->GetString(LSTR_CANCEL),
        { { ui_exit_cancel_button_background } }
    );
    pDialogueWindow->CreateButton(0, 0, 0, 0, 1, 0,
        UIMSG_BuyInShop_Identify_Repair, 0, GameKey::None, "");
    pDialogueWindow->CreateButton(480, 160, 140, 30,
        1, 0, UIMSG_0, DIALOGUE_83_bounty_hunting, GameKey::None, "");
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
    // get new monster for hunting
    if (pParty->PartyTimes.bountyHunting_next_generation_time[(
        int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] <
        pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[(
            int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] = false;
        pParty->PartyTimes.bountyHunting_next_generation_time[(
            int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] =
            GameTime((int64_t)((double)(0x12750000 *
            (pParty->uCurrentMonth +
                12 * pParty->uCurrentYear - 14015)) *
                0.033333335));
        for (i = rand();; i = rand()) {
            rand_monster_id = i % 258 + 1;
            pParty->monster_id_for_hunting[(int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] =
                rand_monster_id;
            if ((uint16_t)rand_monster_id < 0x73u ||
                (uint16_t)rand_monster_id > 0x84u) {
                if (((uint16_t)rand_monster_id < 0xEBu ||
                    (uint16_t)rand_monster_id > 0xFCu) &&
                    ((uint16_t)rand_monster_id < 0x85u ||
                    (uint16_t)rand_monster_id > 0x96u) &&
                        ((uint16_t)rand_monster_id < 0x97u ||
                    (uint16_t)rand_monster_id > 0xBAu) &&
                            ((uint16_t)rand_monster_id < 0xC4u ||
                    (uint16_t)rand_monster_id > 0xC6u))
                    break;
            }
        }
    }
    bountyHunting_monster_id_for_hunting = pParty->monster_id_for_hunting[(
        int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)];
    if (!pParty->monster_for_hunting_killed[(
        int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)]) {
        bountyHunting_text = pNPCTopics[351].pText;
        if (!pParty->monster_id_for_hunting[(
            int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)])
            bountyHunting_text = pNPCTopics[353].pText;
    } else {  // get prize
        if (pParty->monster_id_for_hunting[(
            int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)]) {
            pParty->PartyFindsGold(
                100 *
                pMonsterStats
                ->pInfos
                [(unsigned __int16)pParty->monster_id_for_hunting[(
                    int64_t)((char *)window_SpeakInHouse->ptr_1C -
                        102)]]
            .uLevel,
                0);
            for (uint i = 0; i < 4; ++i)
                pParty->pPlayers[i].SetVariable(VAR_Award, 86);
            pParty->uNumBountiesCollected +=
                100 * pMonsterStats
                ->pInfos[pParty->monster_id_for_hunting[(
                    int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)]]
                .uLevel;
            pParty->monster_id_for_hunting[(
                int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] = 0;
            pParty->monster_for_hunting_killed[(
                int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] = false;
        }
        bountyHunting_text = pNPCTopics[352].pText;
    }
}

//----- (004B254D) --------------------------------------------------------
String _4B254D_SkillMasteryTeacher(int trainerInfo) {
    int teacherLevel;                // edx@1
    int skillBeingTaught;            // ecx@1
    int pClassType;                  // eax@7
    int currClassMaxMastery;         // eax@7
    int pointsInSkillWOutMastery;    // ebx@7
    int classBaseId;                 // eax@8
    unsigned int skillMastery;       // eax@29
    unsigned __int16 pointsInSkill;  // [sp+1Ch] [bp-10h]@7
    int masteryLevelBeingTaught;     // [sp+24h] [bp-8h]@7

    guild_membership_approved = false;
    teacherLevel = (trainerInfo - 200) % 3;
    skillBeingTaught = (trainerInfo - 200) / 3;
    Player *activePlayer = pPlayers[uActiveCharacter];
    pClassType = activePlayer->classType;
    currClassMaxMastery =
        byte_4ED970_skill_learn_ability_by_class_table[pClassType]
        [skillBeingTaught];
    masteryLevelBeingTaught = teacherLevel + 2;
    dword_F8B1B0_MasteryBeingTaught = masteryLevelBeingTaught;
    if (currClassMaxMastery < masteryLevelBeingTaught) {
        classBaseId = pClassType - pClassType % 4;

        if (byte_4ED970_skill_learn_ability_by_class_table[classBaseId + 1]
            [skillBeingTaught] >=
            masteryLevelBeingTaught) {
            return localization->FormatString(
                LSTR_FMT_HAVE_TO_BE_PROMOTED,
                localization->GetClassName(classBaseId + 1));
        } else if (byte_4ED970_skill_learn_ability_by_class_table
            [classBaseId + 2][skillBeingTaught] >= masteryLevelBeingTaught &&
            byte_4ED970_skill_learn_ability_by_class_table
            [classBaseId + 3][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(
                LSTR_FMT_HAVE_TO_BE_PROMOTED_2,
                localization->GetClassName(classBaseId + 2),
                localization->GetClassName(classBaseId + 3)
            );
        } else if (byte_4ED970_skill_learn_ability_by_class_table
            [classBaseId + 2][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(
                LSTR_FMT_HAVE_TO_BE_PROMOTED,
                localization->GetClassName(classBaseId + 2)
            );
        } else if (byte_4ED970_skill_learn_ability_by_class_table
            [classBaseId + 3][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(
                LSTR_FMT_HAVE_TO_BE_PROMOTED,
                localization->GetClassName(classBaseId + 3)
            );
        } else {
            return localization->FormatString(
                LSTR_FMT_SKILL_CANT_BE_LEARNED,
                localization->GetClassName(pClassType)
            );
        }
    }

    if (!activePlayer->CanAct())
        return String(pNPCTopics[122].pText);  // Not in your condition!

    pointsInSkill = activePlayer->pActiveSkills[skillBeingTaught];
    pointsInSkillWOutMastery = pointsInSkill & 0x3F;
    if (!pointsInSkillWOutMastery)
        return String(
            pNPCTopics[131].pText);  // You must know the skill before you can
                                     // become an expert in it!

    skillMastery = SkillToMastery(pointsInSkill);
    if (skillMastery > teacherLevel + 1)
        return String(
            pNPCTopics[teacherLevel + 128]
            .pText);  // You are already an SKILLLEVEL in this skill.

    dword_F8B1AC_award_bit_number = (AwardType)skillBeingTaught;
    if (masteryLevelBeingTaught == 2 && pointsInSkillWOutMastery < 4 ||
        masteryLevelBeingTaught == 3 && pointsInSkillWOutMastery < 7 ||
        masteryLevelBeingTaught == 4 && pointsInSkillWOutMastery < 10)
        return String(
            pNPCTopics[127].pText);  // You don't meet the requirements, and
                                     // cannot be taught until you do.

    switch (dword_F8B1AC_award_bit_number) {
    case PLAYER_SKILL_STAFF:
    case PLAYER_SKILL_SWORD:
    case PLAYER_SKILL_DAGGER:
    case PLAYER_SKILL_AXE:
    case PLAYER_SKILL_SPEAR:
    case PLAYER_SKILL_BOW:
    case PLAYER_SKILL_MACE:
    case PLAYER_SKILL_ARMSMASTER:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 2000;
            break;
        case 3:
            gold_transaction_amount = 5000;
            break;
        case 4:
            gold_transaction_amount = 8000;
            break;
        }
        break;
    case PLAYER_SKILL_BLASTER:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 0;
            break;
        case 3:
            gold_transaction_amount = 0;
            break;
        case 4:
            gold_transaction_amount = 0;
            break;
        }
        break;
    case PLAYER_SKILL_SHIELD:
    case PLAYER_SKILL_LEATHER:
    case PLAYER_SKILL_CHAIN:
    case PLAYER_SKILL_PLATE:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 1000;
            break;
        case 3:
            gold_transaction_amount = 3000;
            break;
        case 4:
            gold_transaction_amount = 7000;
            break;
        }
        break;
    case PLAYER_SKILL_FIRE:
    case PLAYER_SKILL_AIR:
    case PLAYER_SKILL_WATER:
    case PLAYER_SKILL_EARTH:
    case PLAYER_SKILL_SPIRIT:
    case PLAYER_SKILL_MIND:
    case PLAYER_SKILL_BODY:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 1000;
            break;
        case 3:
            gold_transaction_amount = 4000;
            break;
        case 4:
            gold_transaction_amount = 8000;
            break;
        }
        break;
    case PLAYER_SKILL_LIGHT:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 2000;
            break;
        case 3:
            if (!_449B57_test_bit(pParty->_quest_bits, 114))
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 5000;
            break;
        case 4:
            if (!activePlayer->IsClass(PLAYER_CLASS_ARCHMAGE) ||
                !activePlayer->IsClass(PLAYER_CLASS_PRIEST_OF_SUN))
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 8000;
            break;
        }
        break;
    case PLAYER_SKILL_DARK:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 2000;
            break;
        case 3:
            if (!_449B57_test_bit(pParty->_quest_bits, 110))
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 5000;
            break;
        case 4:
            if (!activePlayer->IsClass(PLAYER_CLASS_LICH) ||
                !activePlayer->IsClass(PLAYER_CLASS_PRIEST_OF_MOON))
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 8000;
            break;
        }
        break;
    case PLAYER_SKILL_ITEM_ID:
    case PLAYER_SKILL_REPAIR:
    case PLAYER_SKILL_MEDITATION:
    case PLAYER_SKILL_PERCEPTION:
    case PLAYER_SKILL_TRAP_DISARM:
    case PLAYER_SKILL_MONSTER_ID:
    case PLAYER_SKILL_STEALING:
    case PLAYER_SKILL_ALCHEMY:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 500;
            break;
        case 3:
            gold_transaction_amount = 2500;
            break;
        case 4:
            gold_transaction_amount = 6000;
            break;
        }
        break;
    case PLAYER_SKILL_MERCHANT:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 2000;
            break;
        case 3:
            if (activePlayer->GetBaseWillpower() < 50)
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 5000;
            break;
        case 4:
            gold_transaction_amount = 8000;
            break;
        }
        break;
    case PLAYER_SKILL_BODYBUILDING:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 500;
            break;
        case 3:
            if (activePlayer->GetBaseEndurance() < 50)
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 2500;
            break;
        case 4:
            gold_transaction_amount = 6000;
            break;
        }
        break;
    case PLAYER_SKILL_DIPLOMACY:
        Error("Diplomacy not used");
        break;
    case PLAYER_SKILL_THIEVERY:
        Error("Thievery not used");
        break;
    case PLAYER_SKILL_DODGE:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 2000;
            break;
        case 3:
            gold_transaction_amount = 5000;
            break;
        case 4:
            if ((activePlayer->pActiveSkills[PLAYER_SKILL_UNARMED] & 63) < 0xA)
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 8000;
            break;
        }
        break;
    case PLAYER_SKILL_UNARMED:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 2000;
            break;
        case 3:
            gold_transaction_amount = 5000;
            break;
        case 4:
            if ((activePlayer->pActiveSkills[PLAYER_SKILL_DODGE] & 63) < 0xA)
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 8000;
            break;
        }
        break;

    case PLAYER_SKILL_LEARNING:
        switch (masteryLevelBeingTaught) {
        case 2:
            gold_transaction_amount = 2000;
            break;
        case 3:
            if (activePlayer->GetBaseIntelligence() < 50)
                return String(pNPCTopics[127].pText);
            gold_transaction_amount = 5000;
            break;
        case 4:
            gold_transaction_amount = 8000;
            break;
        }
        break;
    default:
        Error("Unknown skill");
    }

    if (gold_transaction_amount > pParty->GetGold())
        return String(pNPCTopics[124].pText);  // You don't have enough gold!

    guild_membership_approved = true;
    if (masteryLevelBeingTaught == 2) {
        return localization->FormatString(
            LSTR_FMT_BECOME_S_IN_S_FOR_D_GOLD,
            localization->GetString(LSTR_EXPERT),
            localization->GetSkillName(dword_F8B1AC_award_bit_number),
            gold_transaction_amount);
    } else if (masteryLevelBeingTaught == 3) {
        return localization->FormatString(
            LSTR_FMT_BECOME_S_IN_S_FOR_D_GOLD,
            localization->GetString(LSTR_MASTER),
            localization->GetSkillName(dword_F8B1AC_award_bit_number),
            gold_transaction_amount);
    } else if (masteryLevelBeingTaught == 4) {
        return localization->FormatString(
            LSTR_FMT_BECOME_S_IN_S_FOR_D_GOLD,
            localization->GetString(LSTR_GRANDMASTER),
            localization->GetSkillName(dword_F8B1AC_award_bit_number),
            gold_transaction_amount);
    }

    return String("");
}

String BuildDialogueString(const char *lpsz, unsigned __int8 uPlayerID,
    ItemGen *a3, char *a4, int a5, GameTime *a6) {
    String str = String(lpsz);
    return BuildDialogueString(str, uPlayerID, a3, a4, a5, a6);
}

//----- (00495461) --------------------------------------------------------
String BuildDialogueString(String &str, unsigned __int8 uPlayerID, ItemGen *a3,
    char *a4, int shop_screen, GameTime *a6) {
    char v1[256] = "";
    Player *pPlayer;       // ebx@3
    const char *pText;     // esi@7
    int v17;               // eax@10
    signed __int64 v18;    // qax@18
    unsigned __int8 *v20;  // ebx@32
    int v21;               // ecx@34
    int v29;               // eax@68
    __int16 v55[56];       // [sp+10h] [bp-128h]@34
    SummonedItem v56;      // [sp+80h] [bp-B8h]@107
    int v63;               // [sp+12Ch] [bp-Ch]@32

    pPlayer = &pParty->pPlayers[uPlayerID];

    NPCData *npc = nullptr;
    if (dword_5C35D4)
        npc = HouseNPCData[(uint64_t)((char *)pDialogueNPCCount +
            -(dword_591080 != 0))];  //- 1
    else
        npc = GetNPCData(sDialogue_SpeakingActorNPC_ID);

    String result;

    // pText = a4;
    uint len = str.length();
    for (int i = 0, dst = 0; i < len; ++i) {
        char c = str[i];  // skip through string till we find insertion point
        if (c != '%') {
            result += c;  // add char to result string
        } else {
            v17 = 10 * (int)(str[i + 1] - '0') + str[i + 2] -
                '0';  // v17 tells what the gap needs filling with
            i += 2;

            switch (v17) {
            case 1:
                result += npc->pName;
                break;
            case 2:
                result += pPlayer->pName;
                i += 2;
                break;
            case 3:
            case 4:
                result += v1;
                break;
            case 5:
                v18 = pParty->GetPlayingTime().GetHoursOfDay();
                pText = localization->GetString(LSTR_EVENING);
                if (HEXRAYS_SHIDWORD(v18) <= 0 &&
                    HEXRAYS_SHIDWORD(v18) >= 0 && (unsigned int)v18 >= 5 &&
                    HEXRAYS_SHIDWORD(v18) <= 0) {
                    if (HEXRAYS_SHIDWORD(v18) >= 0 &&
                        (unsigned int)v18 >= 11) {
                        if (v18 < 20)
                            pText = localization->GetString(LSTR_DAY);
                    } else {
                        pText = localization->GetString(LSTR_MORNING);
                    }
                }
                result += pText;
                break;
            case 6:
                if (pPlayer->uSex)
                    result += localization->GetString(LSTR_LADY_LOWERCASE);
                else
                    result += localization->GetString(LSTR_SIR_LOWERCASE);
                break;
            case 7:
                if (pPlayer->uSex)
                    result += localization->GetString(LSTR_LADY);
                else
                    result += localization->GetString(LSTR_SIR);
                break;
            case 8:
                v63 = 0;
                v20 = (unsigned __int8 *)pPlayer->_achieved_awards_bits;
                for (uint _i = 0; _i < 28; ++_i) {
                    if (_449B57_test_bit(
                        v20, word_4EE150[i])) {
                        v21 = v63;
                        ++v63;
                        v55[v63] = word_4EE150[i];
                    }
                }
                if (v63) {
                    if (dword_A74CDC == -1) dword_A74CDC = rand() % v63;
                    pText =
                        pAwards[v55[dword_A74CDC]]
                        .pText;  // (char *)dword_723E80_award_related[2
                                 // * v55[v24]];
                } else {
                    pText = pNPCTopics[55].pText;
                }
                result += pText;
                break;
            case 9:
                if (npc->uSex)
                    result += localization->GetString(LSTR_HER);
                else
                    result += localization->GetString(LSTR_HIS);
                break;
            case 10:
                if (pPlayer->uSex)
                    result += localization->GetString(LSTR_LADY);
                else
                    result += localization->GetString(LSTR_LORD);
                break;
            case 11:
                result += GetReputationString(pParty->GetPartyReputation());
                break;
            case 12:
                result += GetReputationString(npc->rep);
                break;
            case 13:
                result += pNPCStats->sub_495366_MispronounceName(pPlayer->pName[0], pPlayer->uSex);
                break;
            case 14:
                if (npc->uSex)
                    result += localization->GetString(LSTR_SISTER);
                else
                    result += localization->GetString(LSTR_BROTHER);
                break;
            case 15:
                result += localization->GetString(LSTR_DAUGHTER);
                break;
            case 16:
                if (npc->uSex)
                    result += localization->GetString(LSTR_SISTER);
                else
                    result += localization->GetString(LSTR_BROTHER);
                break;
            case 17:  // hired npc text
            {
                uint pay_percentage = pNPCStats->pProfessions[npc->profession].uHirePrice / 100;
                if (pay_percentage == 0) pay_percentage = 1;
                sprintf(v1, "%lu", pay_percentage);
                result += v1;
                break;
            }
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 26:
                strncpy(v1, str.c_str() + i + 1, 2);
                sprintf(v1, "%lu", atoi(v1));
                result += v1;
                break;
            case 23:
                if (pMapStats->GetMapInfo(pCurrentMapName))
                    result += pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)].pName;
                else
                    result += localization->GetString(LSTR_UNKNOWN);
                break;

            case 24:  // item name
                sprintf(v1, format_4E2D80, Color16(255, 255, 155), a3->GetDisplayName().c_str());
                result += v1;
                break;

            case 25:  // base prices
                v29 = pPlayer->GetBaseBuyingPrice(
                    a3->GetValue(), p2DEvents[(int64_t)a4 - 1].fPriceMultiplier
                );
                switch (shop_screen) {
                case 3:
                    v29 = pPlayer->GetBaseSellingPrice(
                        a3->GetValue(), p2DEvents[(int64_t)a4 - 1].fPriceMultiplier
                    );
                    break;
                case 4:
                    v29 = pPlayer->GetBaseIdentifyPrice(
                        p2DEvents[(int64_t)a4 - 1].fPriceMultiplier
                    );
                    break;
                case 5:
                    v29 = pPlayer->GetBaseRepairPrice(
                        a3->GetValue(),
                        p2DEvents[(int64_t)a4 - 1].fPriceMultiplier);
                    break;
                case 6:
                    v29 = pPlayer->GetBaseSellingPrice(
                        a3->GetValue(),
                        p2DEvents[(int64_t)a4 - 1]
                        .fPriceMultiplier) /
                        2;
                    break;
                }
                sprintf(v1, "%lu", v29);
                result += v1;
                break;

            case 27:  // actual price
                v29 = pPlayer->GetBuyingPrice(
                    a3->GetValue(),
                    p2DEvents[(int64_t)a4 - 1].fPriceMultiplier);
                if (shop_screen == 3) {
                    // v29 = pPlayer->GetPriceSell(a3->GetValue(),
                    // p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
                    v29 = pPlayer->GetPriceSell(
                        *a3,
                        p2DEvents[(int64_t)a4 - 1].fPriceMultiplier);
                    // if (a3->IsBroken())
                    // v29 = 1;
                    sprintf(v1, "%lu", v29);
                    result += v1;
                    break;
                }
                if (shop_screen != 4) {
                    if (shop_screen == 5) {
                        v29 = pPlayer->GetPriceRepair(
                            a3->GetValue(),
                            p2DEvents[(int64_t)a4 - 1].fPriceMultiplier);
                    } else {
                        if (shop_screen == 6) {
                            // v29 = pPlayer->GetPriceSell(a3->GetValue(),
                            // p2DEvents[(signed int)a4 -
                            // 1].fPriceMultiplier) / 2;
                            v29 = pPlayer->GetPriceSell(
                                *a3, p2DEvents[(int64_t)a4 - 1]
                                .fPriceMultiplier) /
                                2;
                            // if (a3->IsBroken())
                            // v29 = 1;
                            if (!v29)  // cannot be 0
                                v29 = 1;
                            sprintf(v1, "%lu", v29);
                            result += v1;
                            break;
                        }
                    }
                    sprintf(v1, "%lu", v29);
                    result += v1;
                    break;
                }
                sprintf(
                    v1, "%lu",
                    pPlayer->GetPriceIdentification(
                        p2DEvents[(int64_t)a4 - 1].fPriceMultiplier));
                result += v1;
                break;

            case 28:  // shop type - blacksmith ect..
                result += p2DEvents[(int64_t)a4 - 1].pProprieterTitle;
                break;

            case 29:  // identify cost
                sprintf(
                    v1, "%lu",
                    pPlayer->GetPriceIdentification(
                        p2DEvents[(int64_t)a4 - 1].fPriceMultiplier));
                result += v1;
                break;
            case 30:
                if (!a6) {
                    result += a4;
                    break;
                }
                v56.Initialize(*a6);
                result += localization->FormatString(
                    LSTR_FMT_S_D_D,
                    localization->GetMonthName(v56.field_14_exprie_month),
                    v56.field_C_expire_day + 1,
                    v56.field_18_expire_year);
                break;
            case 31:
            case 32:
            case 33:
            case 34:
                result += pParty->pPlayers[v17 - 31].pName;
                break;
            default:
                if (v17 <= 50 || v17 > 70) {
                    strncpy(v1, str.c_str() + i + 1, 2);
                    sprintf(v1, "%lu", atoi(v1));
                    result += v1;
                    break;
                }
                if (v17 - 51 >= 20) {
                    result += a4;
                    break;
                }

                v56.Initialize(pParty->PartyTimes._s_times[v17 - 51]);
                result += localization->FormatString(
                    LSTR_FMT_S_D_D,
                    localization->GetMonthName(v56.field_14_exprie_month),
                    v56.field_C_expire_day + 1,
                    v56.field_18_expire_year);
                break;
            }
        }
    }

    return result;
}

WindowManager windowManager;

void WindowManager::DeleteAllVisibleWindows() {
    while (lWindowList.size() > 1) {
        GUIWindow *pWindow = lWindowList.front();
        pWindow->Release();
        delete pWindow;
        lWindowList.pop_front();
    }
}

void MainMenuUI_LoadFontsAndSomeStuff() {
    // pIcons_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
    pIcons_LOD->SetupPalettes(5, 6, 5);
    // pPaletteManager->SetColorChannelInfo(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
    pPaletteManager->SetColorChannelInfo(5, 6, 5);
    pPaletteManager->RecalculateAll();

    // for (uint i = 0; i < 480; ++i) {  // must be 480 - needs sorting
    //     pSRZBufferLineOffsets[i] = 640 * i;  // must be 640 - needs sorting
    // }

    pFontArrus = GUIFont::LoadFont("arrus.fnt", "FONTPAL");
    pFontLucida = GUIFont::LoadFont("lucida.fnt", "FONTPAL");
    pFontCreate = GUIFont::LoadFont("create.fnt", "FONTPAL");
    pFontSmallnum = GUIFont::LoadFont("smallnum.fnt", "FONTPAL");
    pFontComic = GUIFont::LoadFont("comic.fnt", "FONTPAL");
}

static void LoadPartyBuffIcons() {
    for (uint i = 0; i < 14; ++i) {
        party_buff_icons[i] = assets->GetImage_ColorKey(StringPrintf("isn-%02d", i + 1), 0x7FF);
    }

    uIconIdx_FlySpell = pIconsFrameTable->FindIcon("spell21");
    uIconIdx_WaterWalk = pIconsFrameTable->FindIcon("spell27");
}

void UI_Create() {
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeC"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeB"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeA"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchC"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchB"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchA"));

    game_ui_minimap_dirs[0] = assets->GetImage_Alpha("MAPDIR1");
    game_ui_minimap_dirs[1] = assets->GetImage_Alpha("MAPDIR2");
    game_ui_minimap_dirs[2] = assets->GetImage_Alpha("MAPDIR3");
    game_ui_minimap_dirs[3] = assets->GetImage_Alpha("MAPDIR4");
    game_ui_minimap_dirs[4] = assets->GetImage_Alpha("MAPDIR5");
    game_ui_minimap_dirs[5] = assets->GetImage_Alpha("MAPDIR6");
    game_ui_minimap_dirs[6] = assets->GetImage_Alpha("MAPDIR7");
    game_ui_minimap_dirs[7] = assets->GetImage_Alpha("MAPDIR8");

    game_ui_bar_blue = assets->GetImage_ColorKey("ib-statB", 0x7FF);
    game_ui_bar_green = assets->GetImage_ColorKey("ib-statG", 0x7FF);
    game_ui_bar_yellow = assets->GetImage_ColorKey("ib-statY", 0x7FF);
    game_ui_bar_red = assets->GetImage_ColorKey("ib-statR", 0x7FF);
    game_ui_monster_hp_background = assets->GetImage_ColorKey("mhp_bg", 0x7FF);
    game_ui_monster_hp_border_left = assets->GetImage_ColorKey("mhp_capl", 0x7FF);
    game_ui_monster_hp_border_right = assets->GetImage_ColorKey("mhp_capr", 0x7FF);
    game_ui_monster_hp_green = assets->GetImage_ColorKey("mhp_grn", 0x7FF);
    game_ui_monster_hp_red = assets->GetImage_ColorKey("mhp_red", 0x7FF);
    game_ui_monster_hp_yellow = assets->GetImage_ColorKey("mhp_yel", 0x7FF);
    ui_leather_mm7 = assets->GetImage_Solid("LEATHER");
    ui_leather_mm6 = assets->GetImage_Solid("ibground");
    dialogue_ui_x_x_u = assets->GetImage_ColorKey("x_x_u", 0x7FF);
    ui_buttdesc2 = assets->GetImage_Alpha("BUTTESC2");
    dialogue_ui_x_ok_u = assets->GetImage_ColorKey("x_ok_u", 0x7FF);
    ui_buttyes2 = assets->GetImage_Alpha("BUTTYES2");

    pPrimaryWindow = new GUIWindow(WINDOW_GameUI, 0, 0, window->GetWidth(), window->GetHeight(), 0);
    pPrimaryWindow->CreateButton(7, 8, 460, 343, 1, 0, UIMSG_MouseLeftClickInGame, 0);

    pPrimaryWindow->CreateButton(61, 424, 31, 80, 2, 94, UIMSG_SelectCharacter, 1, GameKey::Digit1);  // buttons for portraits
    pPrimaryWindow->CreateButton(177, 424, 31, 80, 2, 94, UIMSG_SelectCharacter, 2, GameKey::Digit2);
    pPrimaryWindow->CreateButton(292, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 3, GameKey::Digit3);
    pPrimaryWindow->CreateButton(407, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 4, GameKey::Digit4);

    pPrimaryWindow->CreateButton(24, 404, 5, 49, 1, 93, UIMSG_0, 1);  // buttons for HP
    pPrimaryWindow->CreateButton(139, 404, 5, 49, 1, 93, UIMSG_0, 2);
    pPrimaryWindow->CreateButton(255, 404, 5, 49, 1, 93, UIMSG_0, 3);
    pPrimaryWindow->CreateButton(370, 404, 5, 49, 1, 93, UIMSG_0, 4);

    pPrimaryWindow->CreateButton(97, 404, 5, 49, 1, 93, UIMSG_0, 1);  // buttons for SP
    pPrimaryWindow->CreateButton(212, 404, 5, 49, 1, 93, UIMSG_0, 2);
    pPrimaryWindow->CreateButton(328, 404, 5, 49, 1, 93, UIMSG_0, 3);
    pPrimaryWindow->CreateButton(443, 404, 5, 49, 1, 93, UIMSG_0, 4);

    game_ui_tome_quests = assets->GetImage_ColorKey("ib-td1-A", 0x7FF);
    pBtn_Quests = pPrimaryWindow->CreateButton(
        491, 353,
        game_ui_tome_quests->GetWidth(),
        game_ui_tome_quests->GetHeight(),
        1, 0, UIMSG_OpenQuestBook, 0, keyboardActionMapping->GetKey(InputAction::Quest),
        localization->GetString(LSTR_CURRENT_QUESTS), { { game_ui_tome_quests } });

    game_ui_tome_autonotes = assets->GetImage_ColorKey("ib-td2-A", 0x7FF);
    pBtn_Autonotes = pPrimaryWindow->CreateButton(
        527, 353,
        game_ui_tome_autonotes->GetWidth(),
        game_ui_tome_autonotes->GetHeight(),
        1, 0, UIMSG_OpenAutonotes, 0, keyboardActionMapping->GetKey(InputAction::Autonotes),
        localization->GetString(LSTR_AUTONOTES), { { game_ui_tome_autonotes } });

    game_ui_tome_maps = assets->GetImage_ColorKey("ib-td3-A", 0x7FF);
    pBtn_Maps = pPrimaryWindow->CreateButton(
        546, 353,
        game_ui_tome_maps->GetWidth(),
        game_ui_tome_maps->GetHeight(),
        1, 0, UIMSG_OpenMapBook, 0, keyboardActionMapping->GetKey(InputAction::Mapbook),
        localization->GetString(LSTR_MAPS), { { game_ui_tome_maps } });

    game_ui_tome_calendar = assets->GetImage_ColorKey("ib-td4-A", 0x7FF);
    pBtn_Calendar = pPrimaryWindow->CreateButton(
        570, 353,
        game_ui_tome_calendar->GetWidth(),
        game_ui_tome_calendar->GetHeight(),
        1, 0, UIMSG_OpenCalendar, 0, keyboardActionMapping->GetKey(InputAction::TimeCal),
        localization->GetString(LSTR_CALENDAR), { { game_ui_tome_calendar } });

    game_ui_tome_storyline = assets->GetImage_ColorKey("ib-td5-A", 0x7FF);
    pBtn_History = pPrimaryWindow->CreateButton(
        600, 361,
        game_ui_tome_storyline->GetWidth(),
        game_ui_tome_storyline->GetHeight(),
        1, 0, UIMSG_OpenHistoryBook, 0, GameKey::H,
        localization->GetString(LSTR_HISTORY), { { game_ui_tome_storyline } }
    );

    bFlashAutonotesBook = 0;
    bFlashQuestBook = 0;
    bFlashHistoryBook = 0;

    pBtn_ZoomIn = pPrimaryWindow->CreateButton(519, 136, game_ui_btn_zoomin->GetWidth(),
        game_ui_btn_zoomin->GetHeight(), 2, 0, UIMSG_ClickZoomInBtn, 0,
        keyboardActionMapping->GetKey(InputAction::ZoomIn),
        localization->GetString(LSTR_ZOOM_IN), { { game_ui_btn_zoomin } }
    );

    pBtn_ZoomOut = pPrimaryWindow->CreateButton(574, 136, game_ui_btn_zoomout->GetWidth(),
        game_ui_btn_zoomout->GetHeight(), 2, 0, UIMSG_ClickZoomOutBtn, 0,
        keyboardActionMapping->GetKey(InputAction::ZoomOut),
        localization->GetString(LSTR_ZOOM_OUT), { { game_ui_btn_zoomout } });

    pPrimaryWindow->CreateButton(481, 0, 153, 67, 1, 92, UIMSG_0, 0);
    pPrimaryWindow->CreateButton(491, 149, 64, 74, 1, 0, UIMSG_StartHireling1Dialogue, 0, GameKey::Digit5);
    pPrimaryWindow->CreateButton(561, 149, 64, 74, 1, 0, UIMSG_StartHireling2Dialogue, 0, GameKey::Digit6);
    pPrimaryWindow->CreateButton(476, 322, 77, 17, 1, 100, UIMSG_0, 0);
    pPrimaryWindow->CreateButton(555, 322, 77, 17, 1, 101, UIMSG_0, 0);

    pBtn_CastSpell = pPrimaryWindow->CreateButton(476, 450,
        game_ui_btn_cast->GetWidth(),
        game_ui_btn_cast->GetHeight(),
        1, 0, UIMSG_SpellBookWindow, 0, GameKey::C,
        localization->GetString(LSTR_CAST_SPELL), { { game_ui_btn_cast } });
    pBtn_Rest = pPrimaryWindow->CreateButton(518, 450,
        game_ui_btn_rest->GetWidth(),
        game_ui_btn_rest->GetHeight(),
        1, 0, UIMSG_RestWindow, 0, GameKey::R,
        localization->GetString(LSTR_REST), { { game_ui_btn_rest } });
    pBtn_QuickReference = pPrimaryWindow->CreateButton(560, 450,
        game_ui_btn_quickref->GetWidth(),
        game_ui_btn_quickref->GetHeight(),
        1, 0, UIMSG_QuickReference, 0, GameKey::Z,
        localization->GetString(LSTR_QUICK_REFERENCE), { { game_ui_btn_quickref } });
    pBtn_GameSettings = pPrimaryWindow->CreateButton(602, 450,
        game_ui_btn_settings->GetWidth(),
        game_ui_btn_settings->GetHeight(),
        1, 0, UIMSG_GameMenuButton, 0, GameKey::None,
        localization->GetString(LSTR_GAME_OPTIONS), { { game_ui_btn_settings } });

    pBtn_NPCLeft = pPrimaryWindow->CreateButton(469, 178,
        ui_btn_npc_left->GetWidth(),
        ui_btn_npc_left->GetHeight(),
        1, 0, UIMSG_ScrollNPCPanel, 0, GameKey::None, "", { { ui_btn_npc_left } });
    pBtn_NPCRight = pPrimaryWindow->CreateButton(626, 178,
        ui_btn_npc_right->GetWidth(),
        ui_btn_npc_right->GetHeight(),
        1, 0, UIMSG_ScrollNPCPanel, 1, GameKey::None, "", { { ui_btn_npc_right } });

    LoadPartyBuffIcons();
}


String NameAndTitle(const char* name, const char* title) {
    return localization->FormatString(
        LSTR_FMT_S_THE_S,
        name,
        title
    );
}


String NameAndTitle(const char* name, PLAYER_CLASS_TYPE class_type) {
    return NameAndTitle(
        name,
        localization->GetClassName(class_type)
    );
}


String NameAndTitle(const char* name, NPCProf profession) {
    return NameAndTitle(
        name,
        localization->GetNpcProfessionName(profession)
    );
}


String NameAndTitle(NPCData* npc) {
    if (npc->pName) {
        if (npc->profession) {
            Assert(npc->profession < 59);
            return NameAndTitle(npc->pName, npc->profession);
        }

        return npc->pName;
    }

    return String("");
}


String GetDisplayName(Actor* actor) {
    if (actor->dword_000334_unique_name)
        return pMonsterStats->pPlaceStrings[actor->dword_000334_unique_name];
    else
        return pMonsterStats->pInfos[actor->pMonsterInfo.uID].pName;
}


static String SeekKnowledgeElswhereString(Player *player) {
    return localization->FormatString(
        LSTR_FMT_SEEK_KNOWLEDGE_ELSEWHERE,
        player->pName,
        localization->GetClassName(player->classType)
    )
    + "\n \n"
    + localization->GetString(LSTR_NO_FURTHER_OFFERS);
}

void SeekKnowledgeElswhereDialogueOption(GUIWindow* dialogue, Player* player) {
    String str = SeekKnowledgeElswhereString(pPlayers[uActiveCharacter]);
    int text_height = pFontArrus->CalcTextHeight(str, dialogue->uFrameWidth, 0);

    dialogue->DrawTitleText(
        pFontArrus,
        0,
        (174 - text_height) / 2 + 138,
        Color16(0xFFu, 0xFFu, 0x9Bu),
        str,
        3
    );
}


void SkillTrainingDialogue(
    GUIWindow *dialogue,
    int num_skills_avaiable,
    int all_text_height,
    int skill_price
) {
    if (!num_skills_avaiable) {
        SeekKnowledgeElswhereDialogueOption(
            dialogue,
            pPlayers[uActiveCharacter]
        );

        return;
    }

    auto skill_price_label = localization->FormatString(
        LSTR_FMT_SKILL_COST_D, skill_price
    );
    dialogue->DrawTitleText(pFontArrus, 0, 146, 0, skill_price_label, 3);

    int textspacings = (149 - all_text_height) / num_skills_avaiable;
    if (textspacings > 32) textspacings = 32;
    int textoffset = (149 - num_skills_avaiable * textspacings - all_text_height) / 2
            - textspacings / 2 + 162;
    for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton;
        ++i
    ) {
        auto pButton = pDialogueWindow->GetControl(i);
        if (pButton->msg_param == DIALOGUE_GUILD_BUY_BOOKS) {
            pButton->uY = textspacings + textoffset;
            int line_height = pFontArrus->CalcTextHeight(
                localization->GetString(LSTR_BUY_SPELLS),
                dialogue->uFrameWidth,
                0
            );
            pButton->uHeight = line_height;
            textoffset = pButton->uY + line_height - 1;
            pButton->uW = textoffset + 6;
            int text_color = Color16(255, 255, 155);
            if (pDialogueWindow->pCurrentPosActiveItem != i) {
                text_color = Color16(255, 255, 255);
            }
            dialogue->DrawTitleText(
                pFontArrus, 0, pButton->uY, text_color,
                localization->GetString(LSTR_BUY_SPELLS),
                3
            );
        } else {
            auto skill_id = GetLearningDialogueSkill(
                (DIALOGUE_TYPE)pButton->msg_param
            );

            if (!byte_4ED970_skill_learn_ability_by_class_table
                [pPlayers[uActiveCharacter]->classType][skill_id]
                || pPlayers[uActiveCharacter]->pActiveSkills[skill_id]) {
                pButton->uW = 0;
                pButton->uHeight = 0;
                pButton->uY = 0;
            } else {
                auto skill_name_label = localization->GetSkillName(skill_id);
                int line_height = pFontArrus->CalcTextHeight(
                    skill_name_label,
                    dialogue->uFrameWidth,
                    0
                );
                pButton->uY = textspacings + textoffset;
                pButton->uHeight = line_height;
                pButton->uW = pButton->uY + line_height + 6 - 1;
                textoffset += textspacings + line_height - 1;
                int text_color = Color16(225, 205, 35);
                if (pDialogueWindow->pCurrentPosActiveItem != i)
                    text_color = Color16(255, 255, 255);
                dialogue->DrawTitleText(
                    pFontArrus, 0, pButton->uY, text_color,
                    skill_name_label,
                    3
                );
            }
        }
    }
}


//----- (004B29F2) --------------------------------------------------------
const char* GetJoinGuildDialogueOption(GUILD_ID guild_id) {
    static const int dialogue_base = 110;
    guild_membership_approved = false;
    dword_F8B1AC_award_bit_number = (AwardType)(Award_Membership_ElementalGuilds + guild_id);
    gold_transaction_amount = price_for_membership[guild_id];

    if (uActiveCharacter == 0)
        uActiveCharacter = pParty->GetFirstCanAct();  // avoid nzi

    if (pPlayers[uActiveCharacter]->CanAct()) {
        if (_449B57_test_bit(
            (uint8_t*)pPlayers[uActiveCharacter]->_achieved_awards_bits,
            dword_F8B1AC_award_bit_number)) {
            return pNPCTopics[dialogue_base + 13].pText;
        } else {
            if (gold_transaction_amount <= pParty->GetGold()) {
                guild_membership_approved = true;
                return pNPCTopics[dialogue_base + guild_id].pText;
            } else {
                return pNPCTopics[dialogue_base + 14].pText;
            }
        }
    } else {
        return pNPCTopics[dialogue_base + 12].pText;
    }
}
