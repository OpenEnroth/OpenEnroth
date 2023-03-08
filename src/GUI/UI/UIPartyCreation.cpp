#include <cstdlib>
#include <string>

#include "Application/Game.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
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


#include "Library/Random/Random.h"


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
        for (PLAYER_SKILL_TYPE i : AllSkills()) {
            if (pParty->pPlayers[j].pActiveSkills[i])
                ++skills_count;
        }
        if (skills_count < 4) {
            return false;
        }
    }
    return true;
}

void CreateParty_EventLoop() {
    auto pPlayer = pParty->pPlayers.data();
    while (!pCurrentFrameMessageQueue->Empty()) {
        UIMessageType msg;
        int param, param2;
        pCurrentFrameMessageQueue->PopMessage(&msg, &param, &param2);

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

            new OnButtonClick({pButton->uX, pButton->uY}, {0, 0}, pButton, std::string(), false);
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
            new OnButtonClick({pButton->uX, pButton->uY}, {0, 0}, pButton, std::string(), false);
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
            new OnButtonClick({pCreationUI_BtnPressLeft[param]->uX, pCreationUI_BtnPressLeft[param]->uY}, {0, 0},
                pCreationUI_BtnPressLeft[param], std::string(), false);
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
            new OnButtonClick({pCreationUI_BtnPressRight[param]->uX, pCreationUI_BtnPressRight[param]->uY}, {0, 0},
                pCreationUI_BtnPressRight[param], std::string(), false);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
            pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            break;
        case UIMSG_PlayerCreationClickPlus:
            new OnButtonClick2({613, 393}, {0, 0}, pPlayerCreationUI_BtnPlus, std::string(), false);
            pPlayer[uPlayerCreationUI_SelectedCharacter].IncreaseAttribute(
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7);
            pAudioPlayer->PlaySound(SOUND_ClickMinus, 0, 0, -1, 0, 0);
            break;
        case UIMSG_PlayerCreationClickMinus:
            new OnButtonClick2({523, 393}, {0, 0}, pPlayerCreationUI_BtnMinus, std::string(), false);
            pPlayer[uPlayerCreationUI_SelectedCharacter].DecreaseAttribute(
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7);
            pAudioPlayer->PlaySound(SOUND_ClickPlus, 0, 0, -1, 0, 0);
            break;
        case UIMSG_PlayerCreationSelectActiveSkill:
            if (pPlayer[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(3) == PLAYER_SKILL_INVALID)
                pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].pActiveSkills[pPlayer[uPlayerCreationUI_SelectedCharacter]
                    .GetSkillIdxByOrder(param + 4)] = 1;
            pAudioPlayer->PlaySound(SOUND_ClickSkill, 0, 0, -1, 0, 0);
            break;
        case UIMSG_PlayerCreationSelectClass:
            pPlayer[uPlayerCreationUI_SelectedCharacter].Reset(
                (PLAYER_CLASS_TYPE)param);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
            break;
        case UIMSG_PlayerCreationClickOK:
            new OnButtonClick2({580, 431}, {0, 0}, pPlayerCreationUI_BtnOK);
            if (PlayerCreation_GetUnspentAttributePointCount() ||
                !PlayerCreation_Choose4Skills())
                game_ui_status_bar_event_string_time_left =
                    platform->tickCount() + 4000;
            else
                uGameState = GAME_STATE_STARTING_NEW_GAME;
            break;
        case UIMSG_PlayerCreationClickReset:
            new OnButtonClick2({527, 431}, {0, 0}, pPlayerCreationUI_BtnReset);
            pParty->Reset();
            break;
        case UIMSG_PlayerCreationRemoveUpSkill:
        {
            int v4;
            v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(2) != PLAYER_SKILL_INVALID) {
                pParty->pPlayers[param].pActiveSkills[pPlayer[param].GetSkillIdxByOrder(2)] = 0;
            }
            break;
        }
        case UIMSG_PlayerCreationRemoveDownSkill:
        {
            int v4;
            v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(3) != PLAYER_SKILL_INVALID)
                pParty->pPlayers[param].pActiveSkills[pPlayer[param].GetSkillIdxByOrder(3)] = 0;
        } break;
        case UIMSG_PlayerCreationChangeName:
            pAudioPlayer->PlaySound(SOUND_ClickSkill, 0, 0, -1, 0, 0);
            uPlayerCreationUI_SelectedCharacter = param;
            keyboardInputHandler->StartTextInput(TextInputType::Text, 15, pGUIWindow_CurrentMenu);
            pGUIWindow_CurrentMenu->wData.val = param;
            break;
        case UIMSG_Escape:
            if (pGameOverWindow) {
                pGameOverWindow->Release();
                pGameOverWindow = nullptr;
                break;
            }
            if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_4000)) break;
            if (GetCurrentMenuID() == MENU_MAIN ||
                GetCurrentMenuID() == MENU_MMT_MAIN_MENU ||
                GetCurrentMenuID() == MENU_CREATEPARTY ||
                GetCurrentMenuID() == MENU_NAMEPANELESC) {
                // if ( current_screen_type == SCREEN_VIDEO )
                // pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_ChangeGameState,
                    0, 0);
            }
            break;
        case UIMSG_ChangeGameState:
            uGameState = GAME_FINISHED;
            break;
        default:
            break;
        }
    }
}

bool PartyCreationUI_Loop() {
    pAudioPlayer->MusicStop();

    pParty->Reset();
    pParty->CreateDefaultParty();

    _449B7E_toggle_bit(pParty->_quest_bits, QBIT_EMERALD_ISLAND_RED_POTION_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, QBIT_EMERALD_ISLAND_SEASHELL_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, QBIT_EMERALD_ISLAND_LONGBOW_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, QBIT_EMERALD_ISLAND_PLATE_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, QBIT_EMERALD_ISLAND_LUTE_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, QBIT_EMERALD_ISLAND_HAT_ACTIVE, 1);

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

    Sizef oldDims = {640.0, 480.0};

    // move sky
    render->BeginScene2D();
    render->DrawTextureNew(0, 0, main_menu_background);
    int sky_slider_anim_timer = (platform->tickCount() % ((int)oldDims.w * 20)) / 20;
    render->DrawTextureNew(sky_slider_anim_timer / oldDims.w, 2 / oldDims.h, ui_partycreation_sky_scroller);
    render->DrawTextureNew((sky_slider_anim_timer - (int)oldDims.w) / oldDims.w, 2 / oldDims.h, ui_partycreation_sky_scroller);
    render->DrawTextureNew(0, 0, ui_partycreation_top);

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

    pTextCenter = ui_partycreation_font->AlignText_Center(
        640, localization->GetString(LSTR_CREATE_PARTY_FANCY));
    pGUIWindow_CurrentMenu->DrawText(ui_partycreation_font, {pTextCenter + 1, 0}, colorTable.Black.C16(),
        localization->GetString(LSTR_CREATE_PARTY_FANCY), 0, 0, 0);

    render->DrawTextureNew(17 / oldDims.w, 35 / oldDims.h, ui_partycreation_portraits[pParty->pPlayers[0].uCurrentFace]);
    render->DrawTextureNew(176 / oldDims.w, 35 / oldDims.h, ui_partycreation_portraits[pParty->pPlayers[1].uCurrentFace]);
    render->DrawTextureNew(335 / oldDims.w, 35 / oldDims.h, ui_partycreation_portraits[pParty->pPlayers[2].uCurrentFace]);
    render->DrawTextureNew(494 / oldDims.w, 35 / oldDims.h, ui_partycreation_portraits[pParty->pPlayers[3].uCurrentFace]);

    // arrows
    pFrame = pIconsFrameTable->GetFrame(uIconID_CharacterFrame, pEventTimer->uStartTime);
    render->DrawTextureNew(pX / oldDims.w, 29 / oldDims.h, pFrame->GetTexture());
    uPosActiveItem = pGUIWindow_CurrentMenu->GetControl(pGUIWindow_CurrentMenu->pCurrentPosActiveItem);
    uPlayerCreationUI_ArrowAnim = 18 - (platform->tickCount() % 450) / 25;
    render->DrawTextureNew((uPosActiveItem->uZ - 4) / oldDims.w, uPosActiveItem->uY / oldDims.h, ui_partycreation_arrow_l[uPlayerCreationUI_ArrowAnim + 1]);
    render->DrawTextureNew((uPosActiveItem->uX - 12) / oldDims.w, uPosActiveItem->uY / oldDims.h, ui_partycreation_arrow_r[uPlayerCreationUI_ArrowAnim + 1]);

    memset(pText, 0, sizeof(pText));
    strcpy(pText, localization->GetString(LSTR_SKILLS));
    for (int i = strlen(pText) - 1; i >= 0; i--)
        pText[i] = toupper((uint8_t)pText[i]);

    pIntervalX = 18;
    pIntervalY = pFontCreate->GetHeight() - 2;
    uX = 32;
    pX_Numbers = oldDims.w - 147;  // 493;

    for (int i = 0; i < 4; ++i) {
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pIntervalX + 73, 100}, colorTable.Black.C16(),
            localization->GetClassName(pParty->pPlayers[i].classType), 0, 0, 0);
        render->DrawTextureNew((pIntervalX + 77) / oldDims.w, 50 / oldDims.h, ui_partycreation_class_icons[pParty->pPlayers[i].classType / 4]);

        if (pGUIWindow_CurrentMenu->keyboard_input_status != WindowInputStatus::WINDOW_INPUT_NONE &&
            pGUIWindow_CurrentMenu->wData.val == i) {
            switch (pGUIWindow_CurrentMenu->keyboard_input_status) {
            case WindowInputStatus::WINDOW_INPUT_IN_PROGRESS:  // press name panel
                v17 = pGUIWindow_CurrentMenu->DrawTextInRect(pFontCreate, {159 * pGUIWindow_CurrentMenu->wData.val + 18, 124}, 0,
                    keyboardInputHandler->GetTextInput().c_str(), 120, 1);
                pGUIWindow_CurrentMenu->DrawFlashingInputCursor(159 * pGUIWindow_CurrentMenu->wData.val + v17 + 20, 124, pFontCreate);
                break;
            case WindowInputStatus::WINDOW_INPUT_CONFIRMED:  // press enter
                pGUIWindow_CurrentMenu->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
                v126 = 0;
                for (int j = 0; j < strlen(keyboardInputHandler->GetTextInput().c_str()); ++j) {  // edit name
                    if (keyboardInputHandler->GetTextInput().c_str()[j] == ' ')
                        ++v126;
                }
                if (strlen(keyboardInputHandler->GetTextInput().c_str()) > 0 && v126 != strlen(keyboardInputHandler->GetTextInput().c_str()))
                    pParty->pPlayers[i].pName = keyboardInputHandler->GetTextInput();
                pGUIWindow_CurrentMenu->DrawTextInRect(pFontCreate, {pIntervalX, 124}, 0, pParty->pPlayers[i].pName, 130, 0);
                pParty->pPlayers[i].field_1988[27] = 1;
                break;
            case WindowInputStatus::WINDOW_INPUT_CANCELLED:  // press escape
                pGUIWindow_CurrentMenu->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
                pGUIWindow_CurrentMenu->DrawTextInRect(pFontCreate, {pIntervalX, 124}, 0, pParty->pPlayers[i].pName, 130, 0);
                SetCurrentMenuID(MENU_NAMEPANELESC);
                break;
            default:
                break;
            }
        } else {
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontCreate, {pIntervalX, 124}, 0, pParty->pPlayers[i].pName, 130, 0);
        }

        std::string pRaceName = pParty->pPlayers[i].GetRaceName();
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontCreate, {pIntervalX + 72, pIntervalY + 12}, 0, pRaceName, 130, 0);

        pTextCenter = pFontCreate->AlignText_Center(150, pText);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + uX - 24, 291}, colorTable.Tacha.C16(), pText, 0, 0, 0);  // Skills

        int posY = 169;

        auto str1 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_MIGHT), pX_Numbers, pParty->pPlayers[i].GetActualMight());
        pStatColor = pParty->pPlayers[i].GetStatColor(0);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX, posY}, pStatColor, str1);

        auto str2 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_INTELLECT), pX_Numbers, pParty->pPlayers[i].GetActualIntelligence());
        pStatColor = pParty->pPlayers[i].GetStatColor(1);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX, pIntervalY + posY}, pStatColor, str2);

        auto str3 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_PERSONALITY), pX_Numbers, pParty->pPlayers[i].GetActualWillpower());
        pStatColor = pParty->pPlayers[i].GetStatColor(2);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX, 2 * pIntervalY + posY}, pStatColor, str3);

        auto str4 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_ENDURANCE), pX_Numbers, pParty->pPlayers[i].GetActualEndurance());
        pStatColor = pParty->pPlayers[i].GetStatColor(3);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX, 3 * pIntervalY + posY}, pStatColor, str4);

        auto str5 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_ACCURACY), pX_Numbers, pParty->pPlayers[i].GetActualAccuracy());
        pStatColor = pParty->pPlayers[i].GetStatColor(4);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX, 4 * pIntervalY + posY}, pStatColor, str5);

        auto str6 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_SPEED), pX_Numbers, pParty->pPlayers[i].GetActualSpeed());
        pStatColor = pParty->pPlayers[i].GetStatColor(5);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX, 5 * pIntervalY + posY}, pStatColor, str6);

        auto str7 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_LUCK), pX_Numbers, pParty->pPlayers[i].GetActualLuck());
        pStatColor = pParty->pPlayers[i].GetStatColor(6);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX, 6 * pIntervalY + posY}, pStatColor, str7);

        posY = 311;

        pSkillsType = pParty->pPlayers[i].GetSkillIdxByOrder(0);
        pTextCenter = pFontCreate->AlignText_Center(150, localization->GetSkillName(pSkillsType));
        auto str8 = fmt::format("\t{:03}{}", pTextCenter, localization->GetSkillName(pSkillsType));
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX - 24, posY}, colorTable.White.C16(), str8);

        pSkillsType = pParty->pPlayers[i].GetSkillIdxByOrder(1);
        pTextCenter = pFontCreate->AlignText_Center(150, localization->GetSkillName(pSkillsType));
        auto str9 = fmt::format("\t{:03}{}", pTextCenter, localization->GetSkillName(pSkillsType));
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX - 24, pIntervalY + posY}, colorTable.White.C16(), str9);

        pSkillsType = pParty->pPlayers[i].GetSkillIdxByOrder(2);
        pTextCenter = pFontCreate->AlignText_Center(150, localization->GetSkillName(pSkillsType));
        auto str10 = fmt::format("\t{:03}{}", pTextCenter, localization->GetSkillName(pSkillsType));
        pColorText = colorTable.Green.C16();
        if (pSkillsType == PLAYER_SKILL_INVALID)
            pColorText = colorTable.Aqua.C16();
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX - 24, 2 * pIntervalY + posY}, pColorText, str10);

        pSkillsType = pParty->pPlayers[i].GetSkillIdxByOrder(3);
        pTextCenter = pFontCreate->AlignText_Center(150, localization->GetSkillName(pSkillsType));
        auto str11 = fmt::format("\t{:03}{}", pTextCenter, localization->GetSkillName(pSkillsType));
        pColorText = colorTable.Green.C16();
        if (pSkillsType == PLAYER_SKILL_INVALID)
            pColorText = colorTable.Aqua.C16();
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {uX - 24, 3 * pIntervalY + posY}, pColorText, str11);

        pIntervalX += 159;
        pX_Numbers -= 158;
        uX += 158;
    }

    strcpy(pText, localization->GetString(LSTR_CLASS));
    for (int i = strlen(pText) - 1; i >= 0; i--)
        pText[i] = toupper((uint8_t)pText[i]);

    uClassType = pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].classType;
    pTextCenter = pFontCreate->AlignText_Center(193, pText);
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 324, 395}, colorTable.Tacha.C16(), pText, 0, 0, 0);  // Classes

    pColorText = colorTable.Aqua.C16();
    if (uClassType)
        pColorText = colorTable.White.C16();
    pTextCenter = pFontCreate->AlignText_Center(65, localization->GetClassName(0));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 323, 417}, pColorText, localization->GetClassName(0), 0, 0, 0);

    pColorText = colorTable.Aqua.C16();
    if (uClassType != PLAYER_CLASS_PALADIN)
        pColorText = colorTable.White.C16();
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(12));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 323, pIntervalY + 417}, pColorText, localization->GetClassName(12), 0, 0, 0);

    pColorText = colorTable.Aqua.C16();
    if (uClassType != PLAYER_CLASS_RANGER)
        pColorText = colorTable.White.C16();
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(20));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 323, 2 * pIntervalY + 417}, pColorText, localization->GetClassName(20), 0, 0, 0);

    pColorText = colorTable.Aqua.C16();
    if (uClassType != PLAYER_CLASS_CLERIC)
        pColorText = colorTable.White.C16();
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(24));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 388, 417}, pColorText, localization->GetClassName(24), 0, 0, 0);

    pColorText = colorTable.Aqua.C16();
    if (uClassType != PLAYER_CLASS_DRUID)
        pColorText = colorTable.White.C16();
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(28));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 388, pIntervalY + 417}, pColorText, localization->GetClassName(28), 0, 0, 0);

    pColorText = colorTable.Aqua.C16();
    if (uClassType != PLAYER_CLASS_SORCERER)
        pColorText = colorTable.White.C16();
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(32));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 388, 2 * pIntervalY + 417}, pColorText, localization->GetClassName(32), 0, 0, 0);

    pColorText = colorTable.Aqua.C16();
    if (uClassType != PLAYER_CLASS_ARCHER)
        pColorText = colorTable.White.C16();
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(16));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 453, 417}, pColorText, localization->GetClassName(16), 0, 0, 0);

    pColorText = colorTable.Aqua.C16();
    if (uClassType != PLAYER_CLASS_MONK) pColorText = colorTable.White.C16();
    pTextCenter =
        pFontCreate->AlignText_Center(65, localization->GetClassName(8));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 453, pIntervalY + 417}, pColorText, localization->GetClassName(8), 0, 0, 0);

    pColorText = colorTable.Aqua.C16();
    if (uClassType != PLAYER_CLASS_THIEF)
        pColorText = colorTable.White.C16();
    pTextCenter = pFontCreate->AlignText_Center(65, localization->GetClassName(4));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 453, 2 * pIntervalY + 417}, pColorText, localization->GetClassName(4), 0, 0, 0);

    pTextCenter = pFontCreate->AlignText_Center(
        236, localization->GetString(LSTR_AVAILABLE_SKILLS));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 37, 395}, colorTable.Tacha.C16(), localization->GetString(LSTR_AVAILABLE_SKILLS), 0, 0, 0);
    for (int i = 0; i < 9; ++i) {
        pSkillId = pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(i + 4);
        strcpy(pText, localization->GetSkillName(pSkillId));
        pLenText = strlen(pText);
        // trim skills that are too long
        if (pLenText > 13) pText[12] = '\0';

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
        //if ((signed int)pLenText < 8)  // if ( (signed int)v124 > 2 )
        //    pCorrective = 0;
        pColorText = colorTable.Aqua.C16();
        if (!pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].pActiveSkills[pSkillId])
            pColorText = colorTable.White.C16();

        // align skills left / centre /right
        if ((i / 3) == 0) {
            pTextCenter = 5;
        } else if ((i / 3) == 1) {
            pTextCenter = pFontCreate->AlignText_Center(100, pText);
        } else {
            pTextCenter = 105 - pFontCreate->GetLineWidth(pText);
        }

        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {100 * (i / 3) + pTextCenter + pCorrective + 17, pIntervalY * (i % 3) + 417}, pColorText, pText, 0, 0, 0);
    }

    pTextCenter = pFontCreate->AlignText_Center(
        0x5C, localization->GetString(LSTR_BONUS));
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 533, 394}, colorTable.Tacha.C16(), localization->GetString(LSTR_BONUS), 0, 0, 0);

    // force draw so overlays dont get muddled
    render->DrawTwodVerts();
    render->EndTextNew();

    pBonusNum = PlayerCreation_GetUnspentAttributePointCount();

    auto unspent_attribute_bonus_label = fmt::format("{}", pBonusNum);
    pTextCenter =
        pFontCreate->AlignText_Center(84, unspent_attribute_bonus_label);
    pGUIWindow_CurrentMenu->DrawText(pFontCreate, {pTextCenter + 530, 410}, colorTable.White.C16(), unspent_attribute_bonus_label);
    if (game_ui_status_bar_event_string_time_left > platform->tickCount()) {
        message_window.Init();
        message_window.sHint = localization->GetString(LSTR_PARTY_UNASSIGNED_POINTS);
        if (pBonusNum < 0)
            message_window.sHint = localization->GetString(LSTR_PARTY_TOO_MUCH_POINTS);
        message_window.uFrameWidth = 300;
        message_window.uFrameHeight = 100;
        message_window.uFrameX = 170;
        message_window.uFrameY = 140;
        message_window.uFrameZ = 469;
        message_window.uFrameW = 239;
        message_window.DrawMessageBox(0);
    }

    // force draw so overlays dont get muddled
    render->DrawTwodVerts();
    render->EndTextNew();
}

//----- (0049695A) --------------------------------------------------------
GUIWindow_PartyCreation::GUIWindow_PartyCreation() :
    GUIWindow(WINDOW_CharacterCreation, {0, 0}, render->GetRenderDimensions(), 0) {
    pCurrentFrameMessageQueue->Flush();

    main_menu_background = assets->GetImage_PCXFromIconsLOD("makeme.pcx");

    current_screen_type = CURRENT_SCREEN::SCREEN_PARTY_CREATION;
    uPlayerCreationUI_ArrowAnim = 0;
    uPlayerCreationUI_SelectedCharacter = 0;
    int v0 = pFontCreate->GetHeight() - 2;

    ui_partycreation_class_icons[0] = assets->GetImage_ColorKey("IC_KNIGHT");
    ui_partycreation_class_icons[1] = assets->GetImage_ColorKey("IC_THIEF");
    ui_partycreation_class_icons[2] = assets->GetImage_ColorKey("IC_MONK");
    ui_partycreation_class_icons[3] = assets->GetImage_ColorKey("IC_PALAD");
    ui_partycreation_class_icons[4] = assets->GetImage_ColorKey("IC_ARCH");
    ui_partycreation_class_icons[5] = assets->GetImage_ColorKey("IC_RANGER");
    ui_partycreation_class_icons[6] = assets->GetImage_ColorKey("IC_CLER");
    ui_partycreation_class_icons[7] = assets->GetImage_ColorKey("IC_DRUID");
    ui_partycreation_class_icons[8] = assets->GetImage_ColorKey("IC_SORC");

    ui_partycreation_top = assets->GetImage_Alpha("MAKETOP");
    ui_partycreation_sky_scroller = assets->GetImage_Solid("MAKESKY");

    for (int uX = 0; uX < 22; ++uX) {
        ui_partycreation_portraits[uX] = assets->GetImage_ColorKey(fmt::format("{}01", pPlayerPortraitsNames[uX]));
    }

    ui_partycreation_minus = assets->GetImage_ColorKey("buttminu");
    ui_partycreation_plus = assets->GetImage_ColorKey("buttplus");
    ui_partycreation_right = assets->GetImage_ColorKey("presrigh");
    ui_partycreation_left = assets->GetImage_ColorKey("presleft");

    for (int i = 1; i < 20; ++i) {
        ui_partycreation_arrow_l[i] = assets->GetImage_Alpha(fmt::format("arrowl{}", i));
        ui_partycreation_arrow_r[i] = assets->GetImage_Alpha(fmt::format("arrowr{}", i));
    }

    // pGUIWindow_CurrentMenu = new GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0);
    int uControlParam = 0;
    uControlParam = 0;
    int uX = 8;
    for (int i = 0; i < 4; i++) {
        CreateButton({uX, 120}, {145, 25}, 1, 0, UIMSG_PlayerCreationChangeName, uControlParam);
        uX += 158;
        ++uControlParam;
    }

    pCreationUI_BtnPressLeft[0] = CreateButton({10, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FacePrev, 0, InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[1] = CreateButton({169, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FacePrev, 1, InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[2] = CreateButton({327, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FacePrev, 2, InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[3] = CreateButton({486, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FacePrev, 3, InputAction::Invalid, "", {ui_partycreation_left});

    pCreationUI_BtnPressRight[0] = CreateButton({74, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FaceNext, 0, InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[1] = CreateButton({233, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FaceNext, 1, InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[2] = CreateButton({391, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FaceNext, 2, InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[3] = CreateButton({549, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FaceNext, 3, InputAction::Invalid, "", {ui_partycreation_right});

    pCreationUI_BtnPressLeft2[0] = CreateButton({10, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoicePrev, 0, InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[1] = CreateButton({169, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoicePrev, 1, InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[2] = CreateButton({327, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoicePrev, 2, InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[3] = CreateButton({486, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoicePrev, 3, InputAction::Invalid, "", {ui_partycreation_left});

    pCreationUI_BtnPressRight2[0] = CreateButton({74, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoiceNext, 0, InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[1] = CreateButton({233, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoiceNext, 1, InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[2] = CreateButton({391, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoiceNext, 2, InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[3] = CreateButton({549, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoiceNext, 3, InputAction::Invalid, "", {ui_partycreation_right});

    uControlParam = 0;
    uX = 8;
    for (int i = 0 ; i < 4; i++) {
        CreateButton({uX, 308}, {150, v0}, 1, 0, UIMSG_48, uControlParam);
        CreateButton({uX, v0 + 308}, {150, v0}, 1, 0, UIMSG_49, uControlParam);
        CreateButton({uX, 2 * v0 + 308}, {150, v0}, 1, 0, UIMSG_PlayerCreationRemoveUpSkill, uControlParam);
        CreateButton({uX, 3 * v0 + 308}, {150, v0}, 1, 0, UIMSG_PlayerCreationRemoveDownSkill, uControlParam);

        uX += 158;
        ++uControlParam;
    }

    CreateButton({5, 21}, {153, 365}, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 0, InputAction::SelectChar1);
    CreateButton({163, 21}, {153, 365}, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 1, InputAction::SelectChar2);
    CreateButton({321, 21}, {153, 365}, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 2, InputAction::SelectChar3);
    CreateButton({479, 21}, {153, 365}, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 3, InputAction::SelectChar4);

    uX = 23;
    uControlParam = 2;
    do {
        CreateButton({uX, 169}, {120, 20}, 1, 0, UIMSG_0, uControlParam - 2);
        CreateButton({uX, v0 + 169}, {120, 20}, 1, 0, UIMSG_0, uControlParam - 1);
        CreateButton({uX, 2 * v0 + 169}, {120, 20}, 1, 0, UIMSG_0, uControlParam);
        CreateButton({uX, 3 * v0 + 169}, {120, 20}, 1, 0, UIMSG_0, uControlParam + 1);
        CreateButton({uX, 4 * v0 + 169}, {120, 20}, 1, 0, UIMSG_0, uControlParam + 2);
        CreateButton({uX, 5 * v0 + 169}, {120, 20}, 1, 0, UIMSG_0, uControlParam + 3);
        CreateButton({uX, 6 * v0 + 169}, {120, 20}, 1, 0, UIMSG_0, uControlParam + 4);

        uControlParam += 7;
        uX += 158;
    } while ((signed int)uControlParam < 30);

    _41D08F_set_keyboard_control_group(28, 0, 7, 40);

    CreateButton({323, 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 0);
    CreateButton({323, v0 + 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 0xC);
    CreateButton({323, 2 * v0 + 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 0x14);
    CreateButton({388, 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 0x18);
    CreateButton({388, v0 + 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 0x1C);
    CreateButton({388, 2 * v0 + 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 0x20);
    CreateButton({453, 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 0x10);
    CreateButton({453, v0 + 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 8);
    CreateButton({453, 2 * v0 + 417}, {65, v0}, 1, 0, UIMSG_PlayerCreationSelectClass, 4);

    uControlParam = 0;
    do {
        uX = -5;
        if (uControlParam <= 3)
            uX = 0;
        CreateButton({100 * (uControlParam / 3) + uX + 17, v0 * (uControlParam % 3) + 417}, {100, v0}, 1, 0, UIMSG_PlayerCreationSelectActiveSkill, uControlParam);
        ++uControlParam;
    } while (uControlParam < 9);

    ui_partycreation_buttmake = assets->GetImage_Solid("BUTTMAKE");
    ui_partycreation_buttmake2 = assets->GetImage_Solid("BUTTMAKE2");

    pPlayerCreationUI_BtnOK = CreateButton("PartyCreation_OK", {580, 431}, {51, 39}, 1, 0, UIMSG_PlayerCreationClickOK, 0, InputAction::Return, "", {ui_partycreation_buttmake});
    pPlayerCreationUI_BtnReset = CreateButton("PartyCreation_Clear", {527, 431}, {51, 39}, 1, 0, UIMSG_PlayerCreationClickReset, 0, InputAction::Clear, "", {ui_partycreation_buttmake2});
    pPlayerCreationUI_BtnMinus = CreateButton({523, 393}, {20, 35}, 1, 0, UIMSG_PlayerCreationClickMinus, 0, InputAction::Minus, "", {ui_partycreation_minus});
    pPlayerCreationUI_BtnPlus = CreateButton({613, 393}, {20, 35}, 1, 0, UIMSG_PlayerCreationClickPlus, 1, InputAction::Plus, "", {ui_partycreation_plus});

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
        render->BeginScene2D();
        GUI_UpdateWindows();
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
            v9 = grng->Random(32);
            if (!v20[v9]) break;
        }
        if (v8 == 10) {
            v9 = 0;
            if (v20[0]) {
                do {
                    ++v9;
                } while (v20[v9]);
            }
        }
        pParty->random_order_num_unused[i] = v9;
        v20[v9] = 1;
    }

    item.Reset();
    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].classType == PLAYER_CLASS_KNIGHT)
            pParty->pPlayers[i].sResMagicBase = 10;
        pParty->pPlayers[i].pPlayerBuffs[22].expire_time.Reset();
        int page = 0;
        for (PLAYER_SKILL_TYPE j : MagicSkills()) {
            if (pParty->pPlayers[i].pActiveSkills[j]) {
                pParty->pPlayers[i].lastOpenedSpellbookPage = page;
                break;
            }

            page++;
        }
        pItemTable->GenerateItem(ITEM_TREASURE_LEVEL_2, 40, &item);
        pParty->pPlayers[i].AddItem2(-1, &item);

        pParty->pPlayers[i].sHealth = pParty->pPlayers[i].GetMaxHealth();
        pParty->pPlayers[i].sMana = pParty->pPlayers[i].GetMaxMana();
        for (PLAYER_SKILL_TYPE j : AllSkills()) {
            if (!pParty->pPlayers[i].pActiveSkills[j]) continue;

            switch (j) {
            case PLAYER_SKILL_STAFF:
                pParty->pPlayers[i].AddItem(-1, ITEM_STAFF);
                break;
            case PLAYER_SKILL_SWORD:
                pParty->pPlayers[i].AddItem(-1, ITEM_CRUDE_LONGSWORD);
                break;
            case PLAYER_SKILL_DAGGER:
                pParty->pPlayers[i].AddItem(-1, ITEM_DAGGER);
                break;
            case PLAYER_SKILL_AXE:
                pParty->pPlayers[i].AddItem(-1, ITEM_CRUDE_AXE);
                break;
            case PLAYER_SKILL_SPEAR:
                pParty->pPlayers[i].AddItem(-1, ITEM_CRUDE_SPEAR);
                break;
            case PLAYER_SKILL_BOW:
                pParty->pPlayers[i].AddItem(-1, ITEM_CROSSBOW);
                break;
            case PLAYER_SKILL_MACE:
                pParty->pPlayers[i].AddItem(-1, ITEM_MACE);
                break;
            case PLAYER_SKILL_BLASTER:
                Error("No blasters at startup :p");
            case PLAYER_SKILL_SHIELD:
                pParty->pPlayers[i].AddItem(-1, ITEM_WOODEN_BUCKLER);
                break;
            case PLAYER_SKILL_LEATHER:
                pParty->pPlayers[i].AddItem(-1, ITEM_LEATHER_ARMOR);
                break;
            case PLAYER_SKILL_CHAIN:
                pParty->pPlayers[i].AddItem(-1, ITEM_CHAIN_MAIL);
                break;
            case PLAYER_SKILL_PLATE:
                pParty->pPlayers[i].AddItem(-1, ITEM_PLATE_ARMOR);
                break;
            case PLAYER_SKILL_FIRE:
                pParty->pPlayers[i].AddItem(-1, ITEM_SPELLBOOK_FIRE_BOLT);
                pParty->pPlayers[i]
                    .spellbook.pFireSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_AIR:
                pParty->pPlayers[i].AddItem(
                    -1, ITEM_SPELLBOOK_FEATHER_FALL);
                pParty->pPlayers[i]
                    .spellbook.pAirSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_WATER:
                pParty->pPlayers[i].AddItem(
                    -1, ITEM_SPELLBOOK_POISON_SPRAY);
                pParty->pPlayers[i]
                    .spellbook.pWaterSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_EARTH:
                pParty->pPlayers[i].AddItem(-1, ITEM_SPELLBOOK_SLOW);
                pParty->pPlayers[i]
                    .spellbook.pEarthSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_SPIRIT:
                pParty->pPlayers[i].AddItem(-1,
                                            ITEM_SPELLBOOK_BLESS);
                pParty->pPlayers[i]
                    .spellbook.pSpiritSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_MIND:
                pParty->pPlayers[i].AddItem(-1,
                                            ITEM_SPELLBOOK_MIND_BLAST);
                pParty->pPlayers[i]
                    .spellbook.pMindSpellbook.bIsSpellAvailable[0] = true;
                break;
            case PLAYER_SKILL_BODY:
                pParty->pPlayers[i].AddItem(-1,
                                            ITEM_SPELLBOOK_HEAL);
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
                pParty->pPlayers[i].AddItem(-1, grng->RandomSample(Level1Reagents()));
                break;
            case PLAYER_SKILL_DODGE:
                pParty->pPlayers[i].AddItem(-1, ITEM_LEATHER_BOOTS);
                break;
            case PLAYER_SKILL_UNARMED:
                pParty->pPlayers[i].AddItem(-1, ITEM_GAUNTLETS);
                break;
            case PLAYER_SKILL_CLUB:
                pParty->pPlayers[i].AddItem(-1, ITEM_CLUB);
                break;
            default:
                break;
            }

            for (uint k = 0; k < 138; k++) {
                if (pParty->pPlayers[i].pOwnItems[k].uItemID != ITEM_NULL)
                    pParty->pPlayers[i].pOwnItems[k].SetIdentified();
            }
        }
    }

    // pAudioPlayer->PauseSounds(-1);
    return party_not_creation_flag;
}
