#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>

#include "src/Application/Game.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Party.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIPartyCreation.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"


using Io::TextInputType;


GUIFont *ui_partycreation_font;

Image *ui_partycreation_top = nullptr;
Image *ui_partycreation_sky_scroller = nullptr;

Image *ui_partycreation_left = nullptr;
Image *ui_partycreation_right = nullptr;
Image *ui_partycreation_minus = nullptr;
Image *ui_partycreation_plus = nullptr;
Image *ui_partycreation_buttmake2 = nullptr;
Image *ui_partycreation_buttmake = nullptr;

std::array<Image *, 9> ui_partycreation_class_icons;
std::array<Image *, 22> ui_partycreation_portraits;

std::array<Image *, 20> ui_partycreation_arrow_r;
std::array<Image *, 20> ui_partycreation_arrow_l;

bool PartyCreationUI_LoopInternal();
void PartyCreationUI_DeleteFont();

bool PlayerCreation_Choose4Skills() {
    int skills_count;

    for (uint j = 0; j < 4; ++j) {
        skills_count = 0;
        for (uint i = 0; i < 37; ++i) {
            if (pParty->pPlayers[j].pActiveSkills[i]) ++skills_count;
        }
        if (skills_count < 4) {
            return false;
        }
    }
    return true;
}

void CreateParty_EventLoop() {
    auto pPlayer = pParty->pPlayers.data();
    while (!pMessageQueue_50CBD0->Empty()) {
        UIMessageType msg;
        int param, param2;
        pMessageQueue_50CBD0->PopMessage(&msg, &param, &param2);

        switch (msg) {
        case UIMSG_PlayerCreation_SelectAttribute:
        {
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
            break;
        }
        case UIMSG_PlayerCreation_VoicePrev:
        {
            int sex = pParty->pPlayers[param].GetSexByVoice();
            do {
                if (pParty->pPlayers[param].uVoiceID == 0)
                    pParty->pPlayers[param].uVoiceID = 19;
                else
                    --pParty->pPlayers[param].uVoiceID;
            } while (pParty->pPlayers[param].GetSexByVoice() != sex);
            auto pButton = pCreationUI_BtnPressLeft2[param];

            new OnButtonClick(pButton->uX, pButton->uY, 0, 0, pButton,
                String(), false);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
            pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            break;
        }
        case UIMSG_PlayerCreation_VoiceNext:
        {
            int sex = pParty->pPlayers[param].GetSexByVoice();
            do {
                pParty->pPlayers[param].uVoiceID =
                    (pParty->pPlayers[param].uVoiceID + 1) % 20;
            } while (pParty->pPlayers[param].GetSexByVoice() != sex);
            auto pButton = pCreationUI_BtnPressRight2[param];
            new OnButtonClick(pButton->uX, pButton->uY, 0, 0, pButton,
                String(), false);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
            pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            break;
        }
        case UIMSG_PlayerCreation_FacePrev:
            // pPlayer = &pParty->pPlayers[pParam];
            if (!pParty->pPlayers[param].uCurrentFace)
                pParty->pPlayers[param].uCurrentFace = 19;
            else
                pParty->pPlayers[param].uCurrentFace -= 1;
            pParty->pPlayers[param].uVoiceID =
                pParty->pPlayers[param].uCurrentFace;
            pParty->pPlayers[param].SetInitialStats();
            pParty->pPlayers[param].SetSexByVoice();
            pParty->pPlayers[param].RandomizeName();
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            new OnButtonClick(pCreationUI_BtnPressLeft[param]->uX,
                pCreationUI_BtnPressLeft[param]->uY, 0, 0,
                (GUIButton *)pCreationUI_BtnPressLeft[param],
                String(), false);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
            pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            break;
        case UIMSG_PlayerCreation_FaceNext:
            // pPlayer = &pParty->pPlayers[pParam];
            int v20;
            v20 =
                (char)((int)pParty->pPlayers[param].uCurrentFace + 1) % 20;
            pParty->pPlayers[param].uCurrentFace = v20;
            pParty->pPlayers[param].uVoiceID = v20;
            pParty->pPlayers[param].SetInitialStats();
            pParty->pPlayers[param].SetSexByVoice();
            pParty->pPlayers[param].RandomizeName();
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            new OnButtonClick(pCreationUI_BtnPressRight[param]->uX,
                pCreationUI_BtnPressRight[param]->uY, 0, 0,
                (GUIButton *)pCreationUI_BtnPressRight[param],
                String(), false);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
            pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            break;
        case UIMSG_PlayerCreationClickPlus:
            new OnButtonClick2(613, 393, 0, 0,
                (GUIButton *)pPlayerCreationUI_BtnPlus, String(),
                false);
            pPlayer[uPlayerCreationUI_SelectedCharacter].IncreaseAttribute(
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7);
            pAudioPlayer->PlaySound(SOUND_ClickMinus, 0, 0, -1, 0, 0);
            break;
        case UIMSG_PlayerCreationClickMinus:
            new OnButtonClick2(523, 393, 0, 0,
                (GUIButton *)pPlayerCreationUI_BtnMinus, String(),
                false);
            pPlayer[uPlayerCreationUI_SelectedCharacter].DecreaseAttribute(
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7);
            pAudioPlayer->PlaySound(SOUND_ClickPlus, 0, 0, -1, 0, 0);
            break;
        case UIMSG_PlayerCreationSelectActiveSkill:
            if (pPlayer[uPlayerCreationUI_SelectedCharacter]
                .GetSkillIdxByOrder(3) == 37)
                pParty->pPlayers[uPlayerCreationUI_SelectedCharacter]
                .pActiveSkills
                [pPlayer[uPlayerCreationUI_SelectedCharacter]
                .GetSkillIdxByOrder(param + 4)] = 1;
            pAudioPlayer->PlaySound(SOUND_ClickSkill, 0, 0, -1, 0, 0);
            break;
        case UIMSG_PlayerCreationSelectClass:
            pPlayer[uPlayerCreationUI_SelectedCharacter].Reset(
                (PLAYER_CLASS_TYPE)param);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
            break;
        case UIMSG_PlayerCreationClickOK:
            new OnButtonClick2(580, 431, 0, 0,
                (GUIButton *)pPlayerCreationUI_BtnOK);
            if (PlayerCreation_GetUnspentAttributePointCount() ||
                !PlayerCreation_Choose4Skills())
                game_ui_status_bar_event_string_time_left =
                OS_GetTime() + 4000;
            else
                uGameState = GAME_STATE_STARTING_NEW_GAME;
            break;
        case UIMSG_PlayerCreationClickReset:
            new OnButtonClick2(527, 431, 0, 0,
                (GUIButton *)pPlayerCreationUI_BtnReset);
            pParty->Reset();
            break;
        case UIMSG_PlayerCreationRemoveUpSkill:
        {
            int v4;
            v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem +
                7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(2) !=
                37) {  // 37 - None(Нет)
                pParty->pPlayers[param]
                    .pActiveSkills[pPlayer[param].GetSkillIdxByOrder(2)] =
                    0;
            }
            break;
        }
        case UIMSG_PlayerCreationRemoveDownSkill:
        {
            int v4;
            v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem +
                7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(3) !=
                37)  // 37 - None(Нет)
                pParty->pPlayers[param]
                .pActiveSkills[pPlayer[param].GetSkillIdxByOrder(3)] =
                0;
        } break;
        case UIMSG_PlayerCreationChangeName:
            pAudioPlayer->PlaySound(SOUND_ClickSkill, 0, 0, -1, 0, 0);
            uPlayerCreationUI_SelectedCharacter = param;
            keyboardInputHandler->StartTextInput(TextInputType::Text, 15, pGUIWindow_CurrentMenu);
            pGUIWindow_CurrentMenu->ptr_1C = (void *)param;
            break;
        case UIMSG_Escape:
            if (pModalWindow) {
                pModalWindow->Release();
                pModalWindow = nullptr;
                break;
            }
            if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_4000)) break;
            viewparams->bRedrawGameUI = true;
            viewparams->field_48 = 1;
            if (GetCurrentMenuID() == MENU_MAIN ||
                GetCurrentMenuID() == MENU_MMT_MAIN_MENU ||
                GetCurrentMenuID() == MENU_CREATEPARTY ||
                GetCurrentMenuID() == MENU_NAMEPANELESC) {
                // if ( current_screen_type == SCREEN_VIDEO )
                // pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_ChangeGameState,
                    0, 0);
            }
            break;
        case UIMSG_ChangeGameState:
            uGameState = GAME_FINISHED;
            break;
        }
    }
}

bool PartyCreationUI_Loop() {
    pAudioPlayer->MusicStop();

    pParty->Reset();
    pParty->CreateDefaultParty();

    _449B7E_toggle_bit(pParty->_quest_bits,
        PARTY_QUEST_EMERALD_RED_POTION_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_SEASHELL_ACTIVE,
        1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_LONGBOW_ACTIVE,
        1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_PLATE_ACTIVE,
        1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_LUTE_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_HAT_ACTIVE, 1);

    pGUIWindow_CurrentMenu = new GUIWindow_PartyCreation();
    if (PartyCreationUI_LoopInternal()) {
        PartyCreationUI_DeleteFont();
        return false;
    } else {
        PartyCreationUI_DeleteFont();
        return true;
    }
}

//----- (00495B39) --------------------------------------------------------
// void PlayerCreationUI_Draw()
void GUIWindow_PartyCreation::Update() {
    int pTextCenter;                // eax@3
    Icon *pFrame;                   // eax@3
    int pX;                         // ecx@7
    GUIButton *uPosActiveItem;      // edi@12
    int v17;                        // eax@33
    unsigned int pStatColor;        // eax@44
    PLAYER_SKILL_TYPE pSkillsType;  // eax@44
    PLAYER_CLASS_TYPE uClassType;   // edi@53
    int pColorText;                 // eax@53
    PLAYER_SKILL_TYPE pSkillId;     // edi@72
    size_t pLenText;                // eax@72
    signed int v104;                // ecx@72
    signed int pBonusNum;           // edi@82
    char pText[200];                // [sp+10h] [bp-160h]@14
    GUIWindow message_window;       // [sp+D8h] [bp-98h]@83
    int v126;                       // [sp+148h] [bp-28h]@25
    int pIntervalY;                 // [sp+150h] [bp-20h]@14
    int pX_Numbers;                 // [sp+154h] [bp-1Ch]@18
    int uX;                         // [sp+160h] [bp-10h]@18
    int pIntervalX;
    int pCorrective;

    float renwidth = render->GetRenderWidth();
    float renheight = render->GetRenderHeight();

    // move sky
    render->BeginScene();
    render->DrawTextureNew(0, 0, main_menu_background);
    int sky_slider_anim_timer = (OS_GetTime() % (window->GetWidth() * 20)) / 20;
    render->DrawTextureAlphaNew(sky_slider_anim_timer / renwidth, 2 / renheight, ui_partycreation_sky_scroller);
    render->DrawTextureAlphaNew((sky_slider_anim_timer - (int)window->GetWidth()) / renwidth, 2 / renheight, ui_partycreation_sky_scroller);
    render->DrawTextureAlphaNew(0, 0, ui_partycreation_top);

    uPlayerCreationUI_SelectedCharacter = (pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) / 7;
    switch (uPlayerCreationUI_SelectedCharacter) {
        case 0:
            pX = 12;
            break;
        case 1:
            pX = 171;
            break;
        case 2:
            pX = 329;
            break;
        case 3:
            pX = 488;
            break;
        default:
            Error("Invalid selected character");
    }

    pTextCenter = ui_partycreation_font->AlignText_Center(640, localization->GetString(51));   // 640
    pGUIWindow_CurrentMenu->DrawText(
        ui_partycreation_font, pTextCenter + 1, 0, 0,
        localization->GetString(51), 0, 0,
        0);  // CREATE PARTY / С О З Д А Т Ь  О Т Р Я Д

    render->DrawTextureAlphaNew(
        17 / renwidth, 35 / renheight,
        ui_partycreation_portraits[pParty->pPlayers[0].uCurrentFace]);
    render->DrawTextureAlphaNew(
        176 / renwidth, 35 / renheight,
        ui_partycreation_portraits[pParty->pPlayers[1].uCurrentFace]);
    render->DrawTextureAlphaNew(
        335 / renwidth, 35 / renheight,
        ui_partycreation_portraits[pParty->pPlayers[2].uCurrentFace]);
    render->DrawTextureAlphaNew(
        494 / renwidth, 35 / renheight,
        ui_partycreation_portraits[pParty->pPlayers[3].uCurrentFace]);

    // arrows
    pFrame = pIconsFrameTable->GetFrame(uIconID_CharacterFrame, pEventTimer->uStartTime);
    render->DrawTextureAlphaNew(pX / renwidth, 29 / renheight, pFrame->GetTexture());
    uPosActiveItem = pGUIWindow_CurrentMenu->GetControl(pGUIWindow_CurrentMenu->pCurrentPosActiveItem);
    uPlayerCreationUI_ArrowAnim = 18 - (OS_GetTime() % 450) / 25;
    render->DrawTextureAlphaNew((uPosActiveItem->uZ - 4) / renwidth, uPosActiveItem->uY / renheight, ui_partycreation_arrow_l[uPlayerCreationUI_ArrowAnim + 1]);
    render->DrawTextureAlphaNew((uPosActiveItem->uX - 12) / renwidth, uPosActiveItem->uY / renheight, ui_partycreation_arrow_r[uPlayerCreationUI_ArrowAnim + 1]);

    memset(pText, 0, 200);
    strcpy(pText, localization->GetString(205));  // "Skills"
    for (int i = strlen(pText) - 1; i >= 0; i--)  //???
        pText[i] = toupper((uint8_t)pText[i]);

    pIntervalX = 18;
    pIntervalY = pFontCreate->GetHeight() - 2;
    uX = 32;
    pX_Numbers = 493;

    for (int i = 0; i < 4; ++i) {
        pGUIWindow_CurrentMenu->DrawText(
            pFontCreate, pIntervalX + 73, 100, 0,
            localization->GetClassName(pParty->pPlayers[i].classType), 0, 0, 0);
        render->DrawTextureAlphaNew(
            (pIntervalX + 77) / renwidth, 50 / renheight,
            ui_partycreation_class_icons[pParty->pPlayers[i].classType / 4]);

        if (pGUIWindow_CurrentMenu->keyboard_input_status != WindowInputStatus::WINDOW_INPUT_NONE &&
            pGUIWindow_CurrentMenu->ptr_1C == (void *)i) {
            switch (pGUIWindow_CurrentMenu->keyboard_input_status) {
            case WindowInputStatus::WINDOW_INPUT_IN_PROGRESS:  // press name panel
                v17 = pGUIWindow_CurrentMenu->DrawTextInRect(
                    pFontCreate,
                    159 * (int64_t)pGUIWindow_CurrentMenu->ptr_1C + 18, 124, 0,
                    keyboardInputHandler->GetTextInput().c_str(), 120, 1);
                pGUIWindow_CurrentMenu->DrawFlashingInputCursor(
                    159 * (uint64_t)pGUIWindow_CurrentMenu->ptr_1C +
                    v17 + 20,
                    124, pFontCreate);
                break;
            case WindowInputStatus::WINDOW_INPUT_CONFIRMED:  // press enter
                pGUIWindow_CurrentMenu->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
                v126 = 0;
                for (int j = 0;
                    j < strlen(keyboardInputHandler->GetTextInput().c_str());
                    ++j) {  // edit name
                    if (keyboardInputHandler->GetTextInput().c_str()[j] == ' ') ++v126;
                }
                if (strlen(keyboardInputHandler->GetTextInput().c_str()) > 0 &&
                    v126 != strlen(keyboardInputHandler->GetTextInput().c_str()))
                    strcpy(pParty->pPlayers[i].pName, keyboardInputHandler->GetTextInput().c_str());
                pGUIWindow_CurrentMenu->DrawTextInRect(
                    pFontCreate, pIntervalX, 124, 0,
                    pParty->pPlayers[i].pName, 130, 0);
                pParty->pPlayers[i].field_1988[27] = 1;
                break;
            case WindowInputStatus::WINDOW_INPUT_CANCELLED:  // press escape
                pGUIWindow_CurrentMenu->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
                pGUIWindow_CurrentMenu->DrawTextInRect(
                    pFontCreate, pIntervalX, 124, 0,
                    pParty->pPlayers[i].pName, 130, 0);
                SetCurrentMenuID(MENU_NAMEPANELESC);
                break;
            }
        } else {
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontCreate, pIntervalX, 124,
                0, pParty->pPlayers[i].pName,
                130, 0);
        }

        String pRaceName = pParty->pPlayers[i].GetRaceName();
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontCreate, pIntervalX + 72, pIntervalY + 12, 0,
            pRaceName, 130, 0);

        pTextCenter = pFontCreate->AlignText_Center(150, pText);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + uX - 24,
            291, Color16(0xD1, 0xBB, 0x61), pText,
            0, 0, 0);  // Skills

        auto str1 =
            StringPrintf("%s\r%03d%d", localization->GetString(144), pX_Numbers,
                pParty->pPlayers[i].GetActualMight());  // "Might"
        pStatColor = pParty->pPlayers[i].GetStatColor(0);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX, 169, pStatColor,
            str1);

        auto str2 = StringPrintf(
            "%s\r%03d%d", localization->GetString(116), pX_Numbers,
            pParty->pPlayers[i].GetActualIntelligence());  // "Intellect"
        pStatColor = pParty->pPlayers[i].GetStatColor(1);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX, pIntervalY + 169,
            pStatColor, str2);

        auto str3 = StringPrintf(
            "%s\r%03d%d", localization->GetString(163), pX_Numbers,
            pParty->pPlayers[i].GetActualWillpower());  // "Personality"
        pStatColor = pParty->pPlayers[i].GetStatColor(2);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX, 2 * pIntervalY + 169,
            pStatColor, str3);

        auto str4 = StringPrintf(
            "%s\r%03d%d", localization->GetString(75), pX_Numbers,
            pParty->pPlayers[i].GetActualEndurance());  // "Endurance"
        pStatColor = pParty->pPlayers[i].GetStatColor(3);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX, 3 * pIntervalY + 169,
            pStatColor, str4);

        auto str5 = StringPrintf(
            "%s\r%03d%d", localization->GetString(1), pX_Numbers,
            pParty->pPlayers[i].GetActualAccuracy());  // "Accuracy"
        pStatColor = pParty->pPlayers[i].GetStatColor(4);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX, 4 * pIntervalY + 169,
            pStatColor, str5);

        auto str6 =
            StringPrintf("%s\r%03d%d", localization->GetString(211), pX_Numbers,
                pParty->pPlayers[i].GetActualSpeed());  // "Speed"
        pStatColor = pParty->pPlayers[i].GetStatColor(5);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX, 5 * pIntervalY + 169,
            pStatColor, str6);

        auto str7 =
            StringPrintf("%s\r%03d%d", localization->GetString(136), pX_Numbers,
                pParty->pPlayers[i].GetActualLuck());  // "Luck"
        pStatColor = pParty->pPlayers[i].GetStatColor(6);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX, 6 * pIntervalY + 169,
            pStatColor, str7);

        pSkillsType = pParty->pPlayers[i].GetSkillIdxByOrder(0);
        pTextCenter = pFontCreate->AlignText_Center(
            150, localization->GetSkillName(pSkillsType));
        auto str8 = StringPrintf("\t%03u%s", pTextCenter,
            localization->GetSkillName(pSkillsType));
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX - 24, 311,
            Color16(0xFF, 0xFF, 0xFF), str8);

        pSkillsType = pParty->pPlayers[i].GetSkillIdxByOrder(1);
        pTextCenter = pFontCreate->AlignText_Center(
            150, localization->GetSkillName(pSkillsType));
        auto str9 = StringPrintf("\t%03u%s", pTextCenter,
            localization->GetSkillName(pSkillsType));
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, uX - 24, pIntervalY + 311,
            Color16(0xFF, 0xFF, 0xFF), str9);

        pSkillsType = pParty->pPlayers[i].GetSkillIdxByOrder(2);
        pTextCenter = pFontCreate->AlignText_Center(
            150, localization->GetSkillName(pSkillsType));
        auto str10 = StringPrintf("\t%03u%s", pTextCenter,
            localization->GetSkillName(pSkillsType));
        pColorText = Color16(0, 0xFF, 0);
        if ((signed int)pSkillsType >= 37) pColorText = Color16(0, 0xF7, 0xF7);
        pGUIWindow_CurrentMenu->DrawText(
            pFontCreate, uX - 24, 2 * pIntervalY + 311, pColorText, str10);

        pSkillsType = pParty->pPlayers[i].GetSkillIdxByOrder(3);
        pTextCenter = pFontCreate->AlignText_Center(
            150, localization->GetSkillName(pSkillsType));
        auto str11 = StringPrintf("\t%03u%s", pTextCenter,
            localization->GetSkillName(pSkillsType));
        pColorText = Color16(0, 0xFF, 0);
        if ((signed int)pSkillsType >= 37) pColorText = Color16(0, 0xF7, 0xF7);
        pGUIWindow_CurrentMenu->DrawText(
            pFontCreate, uX - 24, 3 * pIntervalY + 311, pColorText, str11);

        pIntervalX += 159;
        pX_Numbers -= 158;
        uX += 158;
    }

    strcpy(pText, localization->GetString(41));  // "Class"
    for (int i = strlen(pText) - 1; i >= 0; i--)
        pText[i] = toupper((unsigned __int8)pText[i]);

    uClassType =
        pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].classType;
    pTextCenter = pFontCreate->AlignText_Center(193, pText);
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 324, 395,
        Color16(0xD1, 0xBB, 0x61), pText, 0, 0,
        0);  // Classes

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType) pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(0));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 323, 417,
        pColorText, localization->GetClassName(0),
        0, 0, 0);

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType != PLAYER_CLASS_PALADIN)
        pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(12));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 323,
        pIntervalY + 417, pColorText,
        localization->GetClassName(12), 0, 0, 0);

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType != PLAYER_CLASS_RANGER)
        pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(20));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 323,
        2 * pIntervalY + 417, pColorText,
        localization->GetClassName(20), 0, 0, 0);

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType != PLAYER_CLASS_CLERIC)
        pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(24));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 388, 417,
        pColorText, localization->GetClassName(24),
        0, 0, 0);

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType != PLAYER_CLASS_DRUID)
        pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(28));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 388,
        pIntervalY + 417, pColorText,
        localization->GetClassName(28), 0, 0, 0);

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType != PLAYER_CLASS_SORCERER)
        pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(32));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 388,
        2 * pIntervalY + 417, pColorText,
        localization->GetClassName(32), 0, 0, 0);

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType != PLAYER_CLASS_ARCHER)
        pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(16));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 453, 417,
        pColorText, localization->GetClassName(16),
        0, 0, 0);

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType != PLAYER_CLASS_MONK) pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(8));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 453,
        pIntervalY + 417, pColorText,
        localization->GetClassName(8), 0, 0, 0);

    pColorText = Color16(0, 0xF7, 0xF7);
    if (uClassType != PLAYER_CLASS_THIEF)
        pColorText = Color16(0xFF, 0xFF, 0xFF);
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(4));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 453,
        2 * pIntervalY + 417, pColorText,
        localization->GetClassName(4), 0, 0, 0);

    pTextCenter = pFontCreate->AlignText_Center(
        236, localization->GetString(20));  // "Available Skills"
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 37, 395,
        Color16(0xD1, 0xBB, 0x61),
        localization->GetString(20), 0, 0, 0);
    for (uint i = 0; i < 9; ++i) {
        pSkillId = pParty->pPlayers[uPlayerCreationUI_SelectedCharacter]
            .GetSkillIdxByOrder(i + 4);
        strcpy(pText, localization->GetSkillName(pSkillId));
        pLenText = strlen(pText);
        v104 = 0;
        if ((signed int)pLenText > 0) {
            if (pText[v104] == 32) {
                pText[v104] = 0;
            } else {
                while (pText[v104] != 32) {
                    ++v104;
                    if (v104 >= (signed int)pLenText) break;
                }
            }
        }
        pCorrective = -10;             // -5
        if ((signed int)pLenText < 8)  // if ( (signed int)v124 > 2 )
            pCorrective = 0;
        pColorText = Color16(0, 0xF7, 0xF7);
        if (!pParty->pPlayers[uPlayerCreationUI_SelectedCharacter]
            .pActiveSkills[pSkillId])
            pColorText = Color16(0xFF, 0xFF, 0xFF);
        pTextCenter = pFontCreate->AlignText_Center(100, pText);
        pGUIWindow_CurrentMenu->DrawText(
            pFontCreate, 100 * (i / 3) + pTextCenter + pCorrective + 17,
            pIntervalY * (i % 3) + 417, pColorText, pText, 0, 0, 0);
    }

    pTextCenter = pFontCreate->AlignText_Center(
        0x5C, localization->GetString(30));  // "Bonus"
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 533, 394,
        Color16(0xD1, 0xBB, 0x61),
        localization->GetString(30), 0, 0, 0);
    pBonusNum = PlayerCreation_GetUnspentAttributePointCount();

    auto unspent_attribute_bonus_label = StringPrintf("%d", pBonusNum);
    pTextCenter =
        pFontCreate->AlignText_Center(84, unspent_attribute_bonus_label);
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, pTextCenter + 530, 410,
        Color16(0xFF, 0xFF, 0xFF),
        unspent_attribute_bonus_label);
    if (game_ui_status_bar_event_string_time_left > OS_GetTime()) {
        message_window.sHint = localization->GetString(
            412);  // "Create Party cannot be completed unless you have assigned
                   // all characters 2 extra skills and have spent all of your
                   // bonus points."
        if (pBonusNum < 0)
            message_window.sHint = localization->GetString(
                413);  // "You can't spend more than 50 points."
        message_window.uFrameWidth = 300;
        message_window.uFrameHeight = 100;
        message_window.uFrameX = 170;
        message_window.uFrameY = 140;
        message_window.uFrameZ = 469;
        message_window.uFrameW = 239;
        message_window.DrawMessageBox(0);
    }
    render->EndScene();
}

//----- (0049695A) --------------------------------------------------------
GUIWindow_PartyCreation::GUIWindow_PartyCreation() :
    GUIWindow(WINDOW_CharacterCreation, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    pMessageQueue_50CBD0->Flush();

    main_menu_background = assets->GetImage_PCXFromIconsLOD("makeme.pcx");

    current_screen_type = CURRENT_SCREEN::SCREEN_PARTY_CREATION;
    uPlayerCreationUI_ArrowAnim = 0;
    uPlayerCreationUI_SelectedCharacter = 0;
    unsigned int v0 = pFontCreate->GetHeight() - 2;

    ui_partycreation_class_icons[0] = assets->GetImage_ColorKey("IC_KNIGHT", 0x7FF);
    ui_partycreation_class_icons[1] = assets->GetImage_ColorKey("IC_THIEF", 0x7FF);
    ui_partycreation_class_icons[2] = assets->GetImage_ColorKey("IC_MONK", 0x7FF);
    ui_partycreation_class_icons[3] = assets->GetImage_ColorKey("IC_PALAD", 0x7FF);
    ui_partycreation_class_icons[4] = assets->GetImage_ColorKey("IC_ARCH", 0x7FF);
    ui_partycreation_class_icons[5] = assets->GetImage_ColorKey("IC_RANGER", 0x7FF);
    ui_partycreation_class_icons[6] = assets->GetImage_ColorKey("IC_CLER", 0x7FF);
    ui_partycreation_class_icons[7] = assets->GetImage_ColorKey("IC_DRUID", 0x7FF);
    ui_partycreation_class_icons[8] = assets->GetImage_ColorKey("IC_SORC", 0x7FF);

    ui_partycreation_top = assets->GetImage_Alpha("MAKETOP");  // , 0x7FF);
    ui_partycreation_sky_scroller = assets->GetImage_Solid("MAKESKY");

    for (int uX = 0; uX < 22; ++uX) {
        ui_partycreation_portraits[uX] = assets->GetImage_ColorKey(StringPrintf("%s01", pPlayerPortraitsNames[uX]), 0x7FF);
    }

    ui_partycreation_minus = assets->GetImage_ColorKey("buttminu", 0x7FF);
    ui_partycreation_plus = assets->GetImage_ColorKey("buttplus", 0x7FF);
    ui_partycreation_right = assets->GetImage_ColorKey("presrigh", 0x7FF);
    ui_partycreation_left = assets->GetImage_ColorKey("presleft", 0x7FF);

    for (int i = 1; i < 20; ++i) {
        ui_partycreation_arrow_l[i] = assets->GetImage_Alpha(StringPrintf("arrowl%d", i));
        ui_partycreation_arrow_r[i] = assets->GetImage_Alpha(StringPrintf("arrowr%d", i));
    }

    // pGUIWindow_CurrentMenu = new GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0);
    int uControlParam = 0;
    uControlParam = 0;
    int uX = 8;
    do {
        CreateButton(uX, 120, 145, 25, 1, 0, UIMSG_PlayerCreationChangeName, uControlParam);
        uX += 158;
        ++uControlParam;
    } while ((signed int)uX < 640);  // 640

    pCreationUI_BtnPressLeft[0] = CreateButton(10, 32, 11, 13, 1, 0, UIMSG_PlayerCreation_FacePrev, 0, GameKey::None, "", { { ui_partycreation_left } });
    pCreationUI_BtnPressLeft[1] = CreateButton(169, 32, 11, 13, 1, 0, UIMSG_PlayerCreation_FacePrev, 1, GameKey::None, "", { { ui_partycreation_left } });
    pCreationUI_BtnPressLeft[2] = CreateButton(327, 32, 11, 13, 1, 0, UIMSG_PlayerCreation_FacePrev, 2, GameKey::None, "", { { ui_partycreation_left } });
    pCreationUI_BtnPressLeft[3] = CreateButton(486, 32, 11, 13, 1, 0, UIMSG_PlayerCreation_FacePrev, 3, GameKey::None, "", { { ui_partycreation_left } });

    pCreationUI_BtnPressRight[0] = CreateButton(74, 32, 11, 13, 1, 0, UIMSG_PlayerCreation_FaceNext, 0, GameKey::None, "", { { ui_partycreation_right } });
    pCreationUI_BtnPressRight[1] = CreateButton(233, 32, 11, 13, 1, 0, UIMSG_PlayerCreation_FaceNext, 1, GameKey::None, "", { { ui_partycreation_right } });
    pCreationUI_BtnPressRight[2] = CreateButton(391, 32, 11, 13, 1, 0, UIMSG_PlayerCreation_FaceNext, 2, GameKey::None, "", { { ui_partycreation_right } });
    pCreationUI_BtnPressRight[3] = CreateButton(549, 32, 11, 13, 1, 0, UIMSG_PlayerCreation_FaceNext, 3, GameKey::None, "", { { ui_partycreation_right } });

    pCreationUI_BtnPressLeft2[0] = CreateButton(10, 103, 11, 13, 1, 0, UIMSG_PlayerCreation_VoicePrev, 0, GameKey::None, "", { { ui_partycreation_left } });
    pCreationUI_BtnPressLeft2[1] = CreateButton(169, 103, 11, 13, 1, 0, UIMSG_PlayerCreation_VoicePrev, 1, GameKey::None, "", { { ui_partycreation_left } });
    pCreationUI_BtnPressLeft2[2] = CreateButton(327, 103, 11, 13, 1, 0, UIMSG_PlayerCreation_VoicePrev, 2, GameKey::None, "", { { ui_partycreation_left } });
    pCreationUI_BtnPressLeft2[3] = CreateButton(486, 103, 11, 13, 1, 0, UIMSG_PlayerCreation_VoicePrev, 3, GameKey::None, "", { { ui_partycreation_left } });

    pCreationUI_BtnPressRight2[0] = CreateButton(74, 103, 11, 13, 1, 0, UIMSG_PlayerCreation_VoiceNext, 0, GameKey::None, "", { { ui_partycreation_right } });
    pCreationUI_BtnPressRight2[1] = CreateButton(233, 103, 11, 13, 1, 0, UIMSG_PlayerCreation_VoiceNext, 1, GameKey::None, "", { { ui_partycreation_right } });
    pCreationUI_BtnPressRight2[2] = CreateButton(391, 103, 11, 13, 1, 0, UIMSG_PlayerCreation_VoiceNext, 2, GameKey::None, "", { { ui_partycreation_right } });
    pCreationUI_BtnPressRight2[3] = CreateButton(549, 103, 11, 13, 1, 0, UIMSG_PlayerCreation_VoiceNext, 3, GameKey::None, "", { { ui_partycreation_right } });

    uControlParam = 0;
    uX = 8;
    do {
        CreateButton(uX, 308, 150, v0, 1, 0, UIMSG_48, uControlParam);
        CreateButton(uX, v0 + 308, 150, v0, 1, 0, UIMSG_49, uControlParam);
        CreateButton(uX, 2 * v0 + 308, 150, v0, 1, 0, UIMSG_PlayerCreationRemoveUpSkill, uControlParam);
        CreateButton(uX, 3 * v0 + 308, 150, v0, 1, 0, UIMSG_PlayerCreationRemoveDownSkill, uControlParam);

        uX += 158;
        ++uControlParam;
    } while ((signed int)uX < 640);  // 640

    CreateButton(5, 21, 153, 365, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 0, GameKey::Digit1);
    CreateButton(163, 21, 153, 365, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 1, GameKey::Digit2);
    CreateButton(321, 21, 153, 365, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 2, GameKey::Digit3);
    CreateButton(479, 21, 153, 365, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 3, GameKey::Digit4);

    uX = 23;
    uControlParam = 2;
    do {
        CreateButton(uX, 169, 120, 20, 1, 0, UIMSG_0, uControlParam - 2);
        CreateButton(uX, v0 + 169, 120, 20, 1, 0, UIMSG_0, uControlParam - 1);
        CreateButton(uX, 2 * v0 + 169, 120, 20, 1, 0, UIMSG_0, uControlParam);
        CreateButton(uX, 3 * v0 + 169, 120, 20, 1, 0, UIMSG_0, uControlParam + 1);
        CreateButton(uX, 4 * v0 + 169, 120, 20, 1, 0, UIMSG_0, uControlParam + 2);
        CreateButton(uX, 5 * v0 + 169, 120, 20, 1, 0, UIMSG_0, uControlParam + 3);
        CreateButton(uX, 6 * v0 + 169, 120, 20, 1, 0, UIMSG_0, uControlParam + 4);

        uControlParam += 7;
        uX += 158;
    } while ((signed int)uControlParam < 30);

    _41D08F_set_keyboard_control_group(28, 0, 7, 40);

    CreateButton(323, 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 0);
    CreateButton(323, v0 + 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 0xC);
    CreateButton(323, 2 * v0 + 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 0x14);
    CreateButton(388, 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 0x18);
    CreateButton(388, v0 + 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 0x1C);
    CreateButton(388, 2 * v0 + 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 0x20);
    CreateButton(453, 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 0x10);
    CreateButton(453, v0 + 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 8);
    CreateButton(453, 2 * v0 + 417, 65, v0, 1, 0, UIMSG_PlayerCreationSelectClass, 4);

    uControlParam = 0;
    do {
        uX = -5;
        if (uControlParam <= 3)
            uX = 0;
        CreateButton(100 * (uControlParam / 3) + uX + 17, v0 * (uControlParam % 3) + 417, 100, v0, 1, 0, UIMSG_PlayerCreationSelectActiveSkill,
            uControlParam);
        ++uControlParam;
    } while (uControlParam < 9);

    ui_partycreation_buttmake = assets->GetImage_Alpha("BUTTMAKE");
    ui_partycreation_buttmake2 = assets->GetImage_Alpha("BUTTMAKE2");

    pPlayerCreationUI_BtnOK = CreateButton(580, 431, 51, 39, 1, 0, UIMSG_PlayerCreationClickOK, 0, GameKey::Return, "", { { ui_partycreation_buttmake } });
    pPlayerCreationUI_BtnReset = CreateButton(527, 431, 51, 39, 1, 0, UIMSG_PlayerCreationClickReset, 0, GameKey::C, "", { { ui_partycreation_buttmake2 } });
    pPlayerCreationUI_BtnMinus = CreateButton(523, 393, 20, 35, 1, 0, UIMSG_PlayerCreationClickMinus, 0, GameKey::Subtract, "", { { ui_partycreation_minus } });
    pPlayerCreationUI_BtnPlus = CreateButton(613, 393, 20, 35, 1, 0, UIMSG_PlayerCreationClickPlus, 1, GameKey::Add, "", { { ui_partycreation_plus } });

    ui_partycreation_font = GUIFont::LoadFont("cchar.fnt", "FONTPAL");
}

GUIWindow_PartyCreation::~GUIWindow_PartyCreation() {
    main_menu_background->Release();
}

void PartyCreationUI_DeleteFont() {
    free(ui_partycreation_font);
    ui_partycreation_font = 0;
}

//----- (00497526) --------------------------------------------------------
bool PartyCreationUI_LoopInternal() {
    // GUIButton *pControlsHead; // edx@6
    // int pControlParam; // esi@12
    signed int v8;                 // edi@30
    int v9;                        // edx@31
                                   //  char *v10; // ebx@37
    ItemGen item;                  // [sp+Ch] [bp-74h]@37
    char v20[32];                  // [sp+30h] [bp-50h]@29
    bool party_not_creation_flag;  // [sp+74h] [bp-Ch]@1

    party_not_creation_flag = false;

    pGUIWindow_CurrentMenu->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
    SetCurrentMenuID(MENU_CREATEPARTY);
    while (GetCurrentMenuID() == MENU_CREATEPARTY) {
        MessageLoopWithWait();

        // PlayerCreationUI_Draw();
        // MainMenu_EventLoop();
        CreateParty_EventLoop();
        render->BeginScene();
        GUI_UpdateWindows();
        render->EndScene();
        render->Present();
        if (uGameState ==
            GAME_FINISHED) {  // if click Esc in PlayerCreation Window
            party_not_creation_flag = true;
            SetCurrentMenuID(MENU_MAIN);
            continue;
        }
        if (uGameState ==
            GAME_STATE_STARTING_NEW_GAME) {  // if click OK in PlayerCreation
                                             // Window
            uGameState = GAME_STATE_PLAYING;
            SetCurrentMenuID(MENU_NEWGAME);
            continue;
        }
    }

    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;

    memset(v20, 0, 32);
    for (int i = 0; i < 32; i++) {
        for (v8 = 0; v8 < 10; ++v8) {
            v9 = rand() % 32;
            if (!v20[v9]) break;
        }
        if (v8 == 10) {
            v9 = 0;
            if (v20[0]) {
                do
                    ++v9;
                while (v20[v9]);
            }
        }
        pParty->field_854[i] = v9;
        v20[v9] = 1;
    }

    item.Reset();
    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].classType == PLAYER_CLASS_KNIGHT)
            pParty->pPlayers[i].sResMagicBase = 10;
        pParty->pPlayers[i].pPlayerBuffs[22].expire_time.Reset();
        for (uint j = 0; j < 9; j++) {
            if (pParty->pPlayers[i].pActiveSkills[PLAYER_SKILL_FIRE + j]) {
                pParty->pPlayers[i].lastOpenedSpellbookPage = j;
                break;
            }
        }
        pItemsTable->GenerateItem(2, 40, &item);
        pParty->pPlayers[i].AddItem2(-1, &item);

        pParty->pPlayers[i].sHealth = pParty->pPlayers[i].GetMaxHealth();
        pParty->pPlayers[i].sMana = pParty->pPlayers[i].GetMaxMana();
        for (uint j = 0; j < 37; ++j) {
            if (!pParty->pPlayers[i].pActiveSkills[j]) continue;

            switch (j) {
            case PLAYER_SKILL_STAFF:
                pParty->pPlayers[i].AddItem(-1, ITEM_STAFF_1);
                break;
            case PLAYER_SKILL_SWORD:
                pParty->pPlayers[i].AddItem(-1, ITEM_LONGSWORD_1);
                break;
            case PLAYER_SKILL_DAGGER:
                pParty->pPlayers[i].AddItem(-1, ITEM_DAGGER_1);
                break;
            case PLAYER_SKILL_AXE:
                pParty->pPlayers[i].AddItem(-1, ITEM_AXE_1);
                break;
            case PLAYER_SKILL_SPEAR:
                pParty->pPlayers[i].AddItem(-1, ITEM_SPEAR_1);
                break;
            case PLAYER_SKILL_BOW:
                pParty->pPlayers[i].AddItem(-1, ITEM_CROSSBOW_1);
                break;
            case PLAYER_SKILL_MACE:
                pParty->pPlayers[i].AddItem(-1, ITEM_MACE_1);
                break;
            case PLAYER_SKILL_BLASTER:
                Error("No blasters at startup :p");
            case PLAYER_SKILL_SHIELD:
                pParty->pPlayers[i].AddItem(-1, ITEM_BUCKLER_1);
                break;
            case PLAYER_SKILL_LEATHER:
                pParty->pPlayers[i].AddItem(-1, ITEM_LEATHER_1);
                break;
            case PLAYER_SKILL_CHAIN:
                pParty->pPlayers[i].AddItem(-1, ITEM_CHAINMAIL_1);
                break;
            case PLAYER_SKILL_PLATE:
                pParty->pPlayers[i].AddItem(-1, ITEM_PLATE_1);
                break;
            case PLAYER_SKILL_FIRE:
                pParty->pPlayers[i].AddItem(-1, ITEM_SPELLBOOK_FIRE_STRIKE);
                pParty->pPlayers[i]
                    .spellbook.pFireSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_AIR:
                pParty->pPlayers[i].AddItem(
                    -1, ITEM_SPELLBOOK_AIR_FEATHER_FALL);
                pParty->pPlayers[i]
                    .spellbook.pAirSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_WATER:
                pParty->pPlayers[i].AddItem(
                    -1, ITEM_SPELLBOOK_WATER_POISON_SPRAY);
                pParty->pPlayers[i]
                    .spellbook.pWaterSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_EARTH:
                pParty->pPlayers[i].AddItem(-1, ITEM_SPELLBOOK_EARTH_SLOW);
                pParty->pPlayers[i]
                    .spellbook.pEarthSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_SPIRIT:
                pParty->pPlayers[i].AddItem(-1,
                    ITEM_SPELLBOOK_SPIRIT_BLESS);
                pParty->pPlayers[i]
                    .spellbook.pSpiritSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_MIND:
                pParty->pPlayers[i].AddItem(-1,
                    ITEM_SPELLBOOK_MIND_MIND_BLAST);
                pParty->pPlayers[i]
                    .spellbook.pMindSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_BODY:
                pParty->pPlayers[i].AddItem(-1,
                    ITEM_SPELLBOOK_BODY_FIRST_AID);
                pParty->pPlayers[i]
                    .spellbook.pBodySpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_LIGHT:
            case PLAYER_SKILL_DARK:
            case PLAYER_SKILL_DIPLOMACY:
                Error("No dimoplacy in mm7 (yet)");
                break;
            case PLAYER_SKILL_ITEM_ID:
            case PLAYER_SKILL_REPAIR:
            case PLAYER_SKILL_MEDITATION:
            case PLAYER_SKILL_PERCEPTION:
            case PLAYER_SKILL_TRAP_DISARM:
            case PLAYER_SKILL_LEARNING:
                pParty->pPlayers[i].AddItem(-1, ITEM_POTION_BOTTLE);
                pParty->pPlayers[i].AddItem(-1, 5 * (rand() % 3 + 40));
                break;
            case PLAYER_SKILL_DODGE:
                pParty->pPlayers[i].AddItem(-1, ITEM_BOOTS_1);
                break;
            case PLAYER_SKILL_UNARMED:
                pParty->pPlayers[i].AddItem(-1, ITEM_GAUNTLETS_1);
                break;
            default:
                break;
            }

            for (uint k = 0; k < 138; k++) {
                if (pParty->pPlayers[i].pOwnItems[k].uItemID)
                    pParty->pPlayers[i].pOwnItems[k].SetIdentified();
            }
        }
    }

    pAudioPlayer->StopChannels(-1, -1);
    return party_not_creation_flag;
}
