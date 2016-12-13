#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Overlays.h"

#include "IO/Keyboard.h"

#include "GUI/UI/UIPartyCreation.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/AIL.h"

void CreateParty_EventLoop()
{
    auto pPlayer = pParty->pPlayers.data();
    while (pMessageQueue_50CBD0->uNumMessages)
    {
        UIMessageType msg;
        int param, param2;
        pMessageQueue_50CBD0->PopMessage(&msg, &param, &param2);

        switch (msg)
        {
            case UIMSG_PlayerCreation_SelectAttribute:
            {
                pGUIWindow_CurrentMenu->pCurrentPosActiveItem = (pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem)
                    % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
                uPlayerCreationUI_SelectedCharacter = param;
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
            }
            break;

            case UIMSG_PlayerCreation_VoicePrev:
            {
                int sex = pParty->pPlayers[param].GetSexByVoice();
                do
                {
                    if (pParty->pPlayers[param].uVoiceID == 0)
                        pParty->pPlayers[param].uVoiceID = 19;
                    else --pParty->pPlayers[param].uVoiceID;
                } while (pParty->pPlayers[param].GetSexByVoice() != sex);
                auto pButton = pCreationUI_BtnPressLeft2[param];

                new OnButtonClick(pButton->uX, pButton->uY, 0, 0, (int)pButton, (char *)1);
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
                pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            }
            break;

            case UIMSG_PlayerCreation_VoiceNext:
            {
                int sex = pParty->pPlayers[param].GetSexByVoice();
                do
                {
                    pParty->pPlayers[param].uVoiceID = (pParty->pPlayers[param].uVoiceID + 1) % 20;
                } while (pParty->pPlayers[param].GetSexByVoice() != sex);
                auto pButton = pCreationUI_BtnPressRight2[param];
                new OnButtonClick(pButton->uX, pButton->uY, 0, 0, (int)pButton, (char *)1);
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
                pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            }
            break;
        case UIMSG_PlayerCreation_FacePrev:
            //pPlayer = &pParty->pPlayers[pParam];
            if (!pParty->pPlayers[param].uCurrentFace)
                pParty->pPlayers[param].uCurrentFace = 19;
            else
                pParty->pPlayers[param].uCurrentFace -= 1;
            pParty->pPlayers[param].uVoiceID = pParty->pPlayers[param].uCurrentFace;
            pParty->pPlayers[param].SetInitialStats();
            pParty->pPlayers[param].SetSexByVoice();
            pParty->pPlayers[param].RandomizeName();
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = (pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem)
                % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            new OnButtonClick(pCreationUI_BtnPressLeft[param]->uX, pCreationUI_BtnPressLeft[param]->uY, 0, 0, (int)pCreationUI_BtnPressLeft[param], (char *)1);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0.0, 0);
            pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            break;
        case UIMSG_PlayerCreation_FaceNext:
            //pPlayer = &pParty->pPlayers[pParam];
            int v20; v20 = (char)((int)pParty->pPlayers[param].uCurrentFace + 1) % 20;
            pParty->pPlayers[param].uCurrentFace = v20;
            pParty->pPlayers[param].uVoiceID = v20;
            pParty->pPlayers[param].SetInitialStats();
            pParty->pPlayers[param].SetSexByVoice();
            pParty->pPlayers[param].RandomizeName();
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = (pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem)
                % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            new OnButtonClick(pCreationUI_BtnPressRight[param]->uX, pCreationUI_BtnPressRight[param]->uY, 0, 0, (int)pCreationUI_BtnPressRight[param], (char *)1);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
            pParty->pPlayers[param].PlaySound(SPEECH_PickMe, 0);
            break;
        case UIMSG_PlayerCreationClickPlus:
            new OnButtonClick2(613, 393, 0, 0, (int)pPlayerCreationUI_BtnPlus, (char *)1);
            pPlayer[uPlayerCreationUI_SelectedCharacter].IncreaseAttribute((pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) % 7);
            pAudioPlayer->PlaySound(SOUND_ClickMinus, 0, 0, -1, 0, 0, 0, 0);
            break;
        case UIMSG_PlayerCreationClickMinus:
            new OnButtonClick2(523, 393, 0, 0, (int)pPlayerCreationUI_BtnMinus, (char *)1);
            pPlayer[uPlayerCreationUI_SelectedCharacter].DecreaseAttribute((pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) % 7);
            pAudioPlayer->PlaySound(SOUND_ClickPlus, 0, 0, -1, 0, 0, 0, 0);
            break;
        case UIMSG_PlayerCreationSelectActiveSkill:
            if (pPlayer[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(3) == 37)
                pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].pActiveSkills[pPlayer[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(param + 4)] = 1;
            pAudioPlayer->PlaySound(SOUND_ClickSkill, 0, 0, -1, 0, 0, 0, 0);
            break;
        case UIMSG_PlayerCreationSelectClass:
            pPlayer[uPlayerCreationUI_SelectedCharacter].Reset((PLAYER_CLASS_TYPE)param);
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
            break;
        case UIMSG_PlayerCreationClickOK:
            new OnButtonClick2(580, 431, 0, 0, (int)pPlayerCreationUI_BtnOK, 0);
            if (PlayerCreation_GetUnspentAttributePointCount() || !PlayerCreation_Choose4Skills())
                game_ui_status_bar_event_string_time_left = GetTickCount() + 4000;
            else
                uGameState = GAME_STATE_STARTING_NEW_GAME;
            break;
        case UIMSG_PlayerCreationClickReset:
            new OnButtonClick2(527, 431, 0, 0, (int)pPlayerCreationUI_BtnReset, 0);
            pParty->Reset();
            break;
        case UIMSG_PlayerCreationRemoveUpSkill:
        {
            int v4; v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(2) != 37)//37 - None(Нет)
                pParty->pPlayers[param].pActiveSkills[pPlayer[param].GetSkillIdxByOrder(2)] = 0;
        }
        break;
        case UIMSG_PlayerCreationRemoveDownSkill:
        {
            int v4; v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(3) != 37)//37 - None(Нет)
                pParty->pPlayers[param].pActiveSkills[pPlayer[param].GetSkillIdxByOrder(3)] = 0;
        }
        break;
        case UIMSG_PlayerCreationChangeName:
            pAudioPlayer->PlaySound(SOUND_ClickSkill, 0, 0, -1, 0, 0, 0, 0);
            uPlayerCreationUI_SelectedCharacter = param;
            pKeyActionMap->EnterText(0, 15, pGUIWindow_CurrentMenu);
            pGUIWindow_CurrentMenu->ptr_1C = (void *)param;
            break;
        case UIMSG_Escape:
            if (pModalWindow)
            {
                pModalWindow->Release();
                pModalWindow = nullptr;
                break;
            }
            if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_4000))
                break;
            viewparams->bRedrawGameUI = true;
            viewparams->field_48 = 1;
            if (GetCurrentMenuID() == MENU_MAIN || GetCurrentMenuID() == MENU_MMT_MAIN_MENU
                || GetCurrentMenuID() == MENU_CREATEPARTY || GetCurrentMenuID() == MENU_NAMEPANELESC)
            {
                //if ( current_screen_type == SCREEN_VIDEO )
                //pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);			  
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_ChangeGameState, 0, 0);
            }
            break;
        case UIMSG_ChangeGameState:
            uGameState = GAME_FINISHED;
            break;
        }
    }
}

bool CreateParty_Loop()
{
// -------------------------------------------------------
// 00462C94 bool MM_Main(const wchar_t *pCmdLine) --- part
    extern bool use_music_folder;
    if (use_music_folder)
        alSourceStop(mSourceID);
    else
    {
        if (pAudioPlayer->hAILRedbook)
            AIL_redbook_stop(pAudioPlayer->hAILRedbook);
    }

    pParty->Reset();
    pParty->CreateDefaultParty();

    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_RED_POTION_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_SEASHELL_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_LONGBOW_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_PLATE_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_LUTE_ACTIVE, 1);
    _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_HAT_ACTIVE, 1);

    pGUIWindow_CurrentMenu = new GUIWindow_PartyCreation();
    if (PlayerCreationUI_Loop())
    {
        DeleteCCharFont();
        return false;
    }
    else
    {
        DeleteCCharFont();
        return true;
    }
}