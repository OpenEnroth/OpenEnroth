#include <cstdlib>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Time/Timer.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIPartyCreation.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"

using Io::TextInputType;


GraphicsImage *ui_partycreation_top = nullptr;
GraphicsImage *ui_partycreation_sky_scroller = nullptr;

GraphicsImage *ui_partycreation_left = nullptr;
GraphicsImage *ui_partycreation_right = nullptr;
GraphicsImage *ui_partycreation_minus = nullptr;
GraphicsImage *ui_partycreation_plus = nullptr;
GraphicsImage *ui_partycreation_buttmake2 = nullptr;
GraphicsImage *ui_partycreation_buttmake = nullptr;

GraphicsImage *ui_partycreation_character_frame = nullptr;

std::array<GraphicsImage *, 9> ui_partycreation_class_icons;
std::array<GraphicsImage *, 22> ui_partycreation_portraits;

std::array<GraphicsImage *, 19> ui_partycreation_arrow_r;
std::array<GraphicsImage *, 19> ui_partycreation_arrow_l;

static Duration errorMessageExpireTime; // expiration time (misc timer) of error message

static const int ARROW_SPIN_PERIOD_MS = 475;

bool PartyCreationUI_LoopInternal();

bool PlayerCreation_Choose4Skills() {
    for (const auto& character : pParty->pCharacters) {
        int skills_count = 0;
        for (CharacterSkillType i : allVisibleSkills()) {
            if (character.pActiveSkills[i])
                ++skills_count;
        }

        if (skills_count != 4) return false;
    }

    return true;
}

void CreateParty_EventLoop() {
    auto pPlayer = pParty->pCharacters.data();
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType msg;
        int param, param2;
        engine->_messageQueue->popMessage(&msg, &param, &param2);

        switch (msg) {
        case UIMSG_PlayerCreation_SelectAttribute:
        {
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            break;
        }
        case UIMSG_PlayerCreation_VoicePrev:
        {
            CharacterSex sex = pParty->pCharacters[param].GetSexByVoice();
            do {
                if (pParty->pCharacters[param].uVoiceID == 0)
                    pParty->pCharacters[param].uVoiceID = 19;
                else
                    --pParty->pCharacters[param].uVoiceID;
            } while (pParty->pCharacters[param].GetSexByVoice() != sex);
            auto pButton = pCreationUI_BtnPressLeft2[param];

            new OnButtonClick({pButton->uX, pButton->uY}, {0, 0}, pButton, std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[param].playReaction(SPEECH_PICK_ME);
            break;
        }
        case UIMSG_PlayerCreation_VoiceNext:
        {
            CharacterSex sex = pParty->pCharacters[param].GetSexByVoice();
            do {
                pParty->pCharacters[param].uVoiceID =
                    (pParty->pCharacters[param].uVoiceID + 1) % 20;
            } while (pParty->pCharacters[param].GetSexByVoice() != sex);
            auto pButton = pCreationUI_BtnPressRight2[param];
            new OnButtonClick({pButton->uX, pButton->uY}, {0, 0}, pButton, std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[param].playReaction(SPEECH_PICK_ME);
            break;
        }
        case UIMSG_PlayerCreation_FacePrev:
            // pPlayer = &pParty->pCharacters[pParam];
            if (!pParty->pCharacters[param].uCurrentFace)
                pParty->pCharacters[param].uCurrentFace = 19;
            else
                pParty->pCharacters[param].uCurrentFace -= 1;
            pParty->pCharacters[param].uVoiceID =
                pParty->pCharacters[param].uCurrentFace;
            pParty->pCharacters[param].SetInitialStats();
            pParty->pCharacters[param].SetSexByVoice();
            pParty->pCharacters[param].RandomizeName();
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            new OnButtonClick({pCreationUI_BtnPressLeft[param]->uX, pCreationUI_BtnPressLeft[param]->uY}, {0, 0},
                pCreationUI_BtnPressLeft[param], std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[param].playReaction(SPEECH_PICK_ME);
            break;
        case UIMSG_PlayerCreation_FaceNext:
            // pPlayer = &pParty->pCharacters[pParam];
            int v20;
            v20 =
                (char)((int)pParty->pCharacters[param].uCurrentFace + 1) % 20;
            pParty->pCharacters[param].uCurrentFace = v20;
            pParty->pCharacters[param].uVoiceID = v20;
            pParty->pCharacters[param].SetInitialStats();
            pParty->pCharacters[param].SetSexByVoice();
            pParty->pCharacters[param].RandomizeName();
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            new OnButtonClick({pCreationUI_BtnPressRight[param]->uX, pCreationUI_BtnPressRight[param]->uY}, {0, 0},
                pCreationUI_BtnPressRight[param], std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[param].playReaction(SPEECH_PICK_ME);
            break;
        case UIMSG_PlayerCreationClickPlus:
            new OnButtonClick2({613, 393}, {0, 0}, pPlayerCreationUI_BtnPlus, std::string(), false);
            pPlayer[uPlayerCreationUI_SelectedCharacter].IncreaseAttribute(
                static_cast<CharacterAttribute>((pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) % 7));
            pAudioPlayer->playUISound(SOUND_ClickMinus);
            break;
        case UIMSG_PlayerCreationClickMinus:
            new OnButtonClick2({523, 393}, {0, 0}, pPlayerCreationUI_BtnMinus, std::string(), false);
            pPlayer[uPlayerCreationUI_SelectedCharacter].DecreaseAttribute(
                static_cast<CharacterAttribute>((pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) % 7));
            pAudioPlayer->playUISound(SOUND_ClickPlus);
            break;
        case UIMSG_PlayerCreationSelectActiveSkill:
            if (pPlayer[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(3) == CHARACTER_SKILL_INVALID)
                pParty->pCharacters[uPlayerCreationUI_SelectedCharacter].pActiveSkills[pPlayer[uPlayerCreationUI_SelectedCharacter]
                    .GetSkillIdxByOrder(param + 4)] = CombinedSkillValue::novice();
            pAudioPlayer->playUISound(SOUND_ClickSkill);
            break;
        case UIMSG_PlayerCreationSelectClass:
            pPlayer[uPlayerCreationUI_SelectedCharacter].ChangeClass((CharacterClass)param);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            break;
        case UIMSG_PlayerCreationClickOK:
            new OnButtonClick2({580, 431}, {0, 0}, pPlayerCreationUI_BtnOK);
            if (CharacterCreation_GetUnspentAttributePointCount() || !PlayerCreation_Choose4Skills()) {
                errorMessageExpireTime = pMiscTimer->time() + Duration::fromRealtimeSeconds(4); // show message for 4 seconds
            } else {
                uGameState = GAME_STATE_STARTING_NEW_GAME;
            }
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
            if (pPlayer[param].GetSkillIdxByOrder(2) != CHARACTER_SKILL_INVALID) {
                pParty->pCharacters[param].pActiveSkills[pPlayer[param].GetSkillIdxByOrder(2)] = CombinedSkillValue::none();
            }
            break;
        }
        case UIMSG_PlayerCreationRemoveDownSkill:
        {
            int v4;
            v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(3) != CHARACTER_SKILL_INVALID)
                pParty->pCharacters[param].pActiveSkills[pPlayer[param].GetSkillIdxByOrder(3)] = CombinedSkillValue::none();
        } break;
        case UIMSG_PlayerCreationChangeName:
            pAudioPlayer->playUISound(SOUND_ClickSkill);
            uPlayerCreationUI_NameEditCharacter = param;
            keyboardInputHandler->StartTextInput(TextInputType::Text, 15, pGUIWindow_CurrentMenu);
            break;
        case UIMSG_Escape:
            if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_4000)) break;
            if (GetCurrentMenuID() == MENU_MAIN ||
                GetCurrentMenuID() == MENU_MMT_MAIN_MENU ||
                GetCurrentMenuID() == MENU_CREATEPARTY ||
                GetCurrentMenuID() == MENU_NAMEPANELESC) {
                // if ( current_screen_type == SCREEN_VIDEO )
                // pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_ChangeGameState,
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
    pEventTimer->setPaused(true);

    // This call is here b/c otherwise Character::timeToRecovery will be overwritten in the main loop from the
    // turn-based queue if we're currently in turn-based combat.
    pTurnEngine->End(false);

    pParty->Reset();
    pParty->createDefaultParty();

    pNPCStats->pNPCData = pNPCStats->pOriginalNPCData;
    pNPCStats->pGroups = pNPCStats->pOriginalGroups;
    pNPCStats->pNPCData[3].uFlags |= NPC_HIRED; // Lady Margaret.

    pGUIWindow_CurrentMenu = new GUIWindow_PartyCreation();
    return !PartyCreationUI_LoopInternal();
}

//----- (00495B39) --------------------------------------------------------
// void PlayerCreationUI_Draw()
void GUIWindow_PartyCreation::Update() {
    int pTextCenter;                // eax@3
    int pX;                         // ecx@7
    GUIButton *uPosActiveItem;      // edi@12
    int v17;                        // eax@33
    Color pStatColor;        // eax@44
    CharacterSkillType pSkillsType;  // eax@44
    CharacterClass uClassType;   // edi@53
    Color pColorText;                 // eax@53
    CharacterSkillType pSkillId;     // edi@72
    size_t pLenText;                // eax@72
    signed int v104;                // ecx@72
    signed int pBonusNum;           // edi@82
    std::string pText;                // [sp+10h] [bp-160h]@14
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
    int sky_slider_anim_timer = std::fmod(pMiscTimer->time().realtimeMillisecondsFloat() * oldDims.w / 20, oldDims.w);
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
            assert(false);
            pX = 0;
            break;
    }

    pTextCenter = ui_partycreation_font->AlignText_Center(
        640, localization->GetString(LSTR_C_R_E_A_T_E_P_A_R_T_Y));
    pGUIWindow_CurrentMenu->DrawText(ui_partycreation_font.get(), {pTextCenter + 1, 0}, colorTable.White,
        localization->GetString(LSTR_C_R_E_A_T_E_P_A_R_T_Y));

    render->DrawTextureNew(17 / oldDims.w, 35 / oldDims.h, ui_partycreation_portraits[pParty->pCharacters[0].uCurrentFace]);
    render->DrawTextureNew(176 / oldDims.w, 35 / oldDims.h, ui_partycreation_portraits[pParty->pCharacters[1].uCurrentFace]);
    render->DrawTextureNew(335 / oldDims.w, 35 / oldDims.h, ui_partycreation_portraits[pParty->pCharacters[2].uCurrentFace]);
    render->DrawTextureNew(494 / oldDims.w, 35 / oldDims.h, ui_partycreation_portraits[pParty->pCharacters[3].uCurrentFace]);

    // arrows
    render->DrawTextureNew(pX / oldDims.w, 29 / oldDims.h, ui_partycreation_character_frame);
    uPosActiveItem = pGUIWindow_CurrentMenu->GetControl(pGUIWindow_CurrentMenu->pCurrentPosActiveItem);
    // cycle arrows backwards
    int arrowAnimTextureNum = ui_partycreation_arrow_l.size() - 1 - (pMiscTimer->time().realtimeMilliseconds() % ARROW_SPIN_PERIOD_MS) / (ARROW_SPIN_PERIOD_MS / ui_partycreation_arrow_l.size());
    render->DrawTextureNew((uPosActiveItem->uZ - 4) / oldDims.w, uPosActiveItem->uY / oldDims.h, ui_partycreation_arrow_l[arrowAnimTextureNum]);
    render->DrawTextureNew((uPosActiveItem->uX - 12) / oldDims.w, uPosActiveItem->uY / oldDims.h, ui_partycreation_arrow_r[arrowAnimTextureNum]);

    pText = localization->GetString(LSTR_SKILLS);
    for (int i = pText.size() - 1; i >= 0; i--)
        pText[i] = toupper(pText[i]); // TODO(captainurist): #unicode this won't work with a Russian localization.

    pIntervalX = 18;
    pIntervalY = assets->pFontCreate->GetHeight() - 2;
    uX = 32;
    pX_Numbers = oldDims.w - 147;  // 493;

    for (int i = 0; i < 4; ++i) {
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pIntervalX + 73, 100}, colorTable.White,
            localization->GetClassName(pParty->pCharacters[i].classType));
        render->DrawTextureNew((pIntervalX + 77) / oldDims.w, 50 / oldDims.h, ui_partycreation_class_icons[std::to_underlying(pParty->pCharacters[i].classType) / 4]);

        if (pGUIWindow_CurrentMenu->keyboard_input_status != WINDOW_INPUT_NONE && uPlayerCreationUI_NameEditCharacter == i) {
            switch (pGUIWindow_CurrentMenu->keyboard_input_status) {
            case WINDOW_INPUT_IN_PROGRESS:  // press name panel
                v17 = pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {159 * uPlayerCreationUI_NameEditCharacter + 18, 124}, colorTable.White,
                    keyboardInputHandler->GetTextInput(), 120, 1);
                pGUIWindow_CurrentMenu->DrawFlashingInputCursor(159 * uPlayerCreationUI_NameEditCharacter + v17 + 20, 124, assets->pFontCreate.get());
                break;
            case WINDOW_INPUT_CONFIRMED:  // press enter
                pGUIWindow_CurrentMenu->keyboard_input_status = WINDOW_INPUT_NONE;
                v126 = 0;
                for (int j = 0; j < keyboardInputHandler->GetTextInput().size(); ++j) {  // edit name
                    if (keyboardInputHandler->GetTextInput()[j] == ' ')
                        ++v126;
                }
                if (keyboardInputHandler->GetTextInput().size() > 0 && v126 != keyboardInputHandler->GetTextInput().size())
                    pParty->pCharacters[i].name = keyboardInputHandler->GetTextInput();
                pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {pIntervalX, 124}, colorTable.White, pParty->pCharacters[i].name, 130, 0);
                break;
            case WINDOW_INPUT_CANCELLED:  // press escape
                pGUIWindow_CurrentMenu->keyboard_input_status = WINDOW_INPUT_NONE;
                pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {pIntervalX, 124}, colorTable.White, pParty->pCharacters[i].name, 130, 0);
                SetCurrentMenuID(MENU_NAMEPANELESC);
                break;
            default:
                break;
            }
        } else {
            pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {pIntervalX, 124}, colorTable.White, pParty->pCharacters[i].name, 130, 0);
        }

        std::string pRaceName = pParty->pCharacters[i].GetRaceName();
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {pIntervalX + 72, pIntervalY + 12}, colorTable.White, pRaceName, 130, 0);

        pTextCenter = assets->pFontCreate->AlignText_Center(150, pText);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + uX - 24, 291}, colorTable.Tacha, pText);  // Skills

        int posY = 169;

        auto str1 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_MIGHT), pX_Numbers, pParty->pCharacters[i].GetActualMight());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_MIGHT);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX, posY}, pStatColor, str1);

        auto str2 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_INTELLECT), pX_Numbers, pParty->pCharacters[i].GetActualIntelligence());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_INTELLIGENCE);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX, pIntervalY + posY}, pStatColor, str2);

        auto str3 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_PERSONALITY), pX_Numbers, pParty->pCharacters[i].GetActualPersonality());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_PERSONALITY);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX, 2 * pIntervalY + posY}, pStatColor, str3);

        auto str4 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_ENDURANCE), pX_Numbers, pParty->pCharacters[i].GetActualEndurance());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_ENDURANCE);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX, 3 * pIntervalY + posY}, pStatColor, str4);

        auto str5 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_ACCURACY), pX_Numbers, pParty->pCharacters[i].GetActualAccuracy());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_ACCURACY);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX, 4 * pIntervalY + posY}, pStatColor, str5);

        auto str6 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_SPEED), pX_Numbers, pParty->pCharacters[i].GetActualSpeed());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_SPEED);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX, 5 * pIntervalY + posY}, pStatColor, str6);

        auto str7 = fmt::format("{}\r{:03}{}", localization->GetString(LSTR_LUCK), pX_Numbers, pParty->pCharacters[i].GetActualLuck());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_LUCK);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX, 6 * pIntervalY + posY}, pStatColor, str7);

        posY = 311;

        pSkillsType = pParty->pCharacters[i].GetSkillIdxByOrder(0);
        pTextCenter = assets->pFontCreate->AlignText_Center(150, localization->GetSkillName(pSkillsType));
        auto str8 = fmt::format("\t{:03}{}", pTextCenter, localization->GetSkillName(pSkillsType));
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX - 24, posY}, colorTable.White, str8);

        pSkillsType = pParty->pCharacters[i].GetSkillIdxByOrder(1);
        pTextCenter = assets->pFontCreate->AlignText_Center(150, localization->GetSkillName(pSkillsType));
        auto str9 = fmt::format("\t{:03}{}", pTextCenter, localization->GetSkillName(pSkillsType));
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX - 24, pIntervalY + posY}, colorTable.White, str9);

        pSkillsType = pParty->pCharacters[i].GetSkillIdxByOrder(2);
        pTextCenter = assets->pFontCreate->AlignText_Center(150, localization->GetSkillName(pSkillsType));
        auto str10 = fmt::format("\t{:03}{}", pTextCenter, localization->GetSkillName(pSkillsType));
        pColorText = colorTable.Green;
        if (pSkillsType == CHARACTER_SKILL_INVALID)
            pColorText = colorTable.Aqua;
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX - 24, 2 * pIntervalY + posY}, pColorText, str10);

        pSkillsType = pParty->pCharacters[i].GetSkillIdxByOrder(3);
        pTextCenter = assets->pFontCreate->AlignText_Center(150, localization->GetSkillName(pSkillsType));
        auto str11 = fmt::format("\t{:03}{}", pTextCenter, localization->GetSkillName(pSkillsType));
        pColorText = colorTable.Green;
        if (pSkillsType == CHARACTER_SKILL_INVALID)
            pColorText = colorTable.Aqua;
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {uX - 24, 3 * pIntervalY + posY}, pColorText, str11);

        pIntervalX += 159;
        pX_Numbers -= 158;
        uX += 158;
    }

    pText = localization->GetString(LSTR_CLASS);
    for (int i = pText.size() - 1; i >= 0; i--)
        pText[i] = toupper(pText[i]); // TODO(captainurist): #unicode this won't work for Russian localization.

    uClassType = pParty->pCharacters[uPlayerCreationUI_SelectedCharacter].classType;
    pTextCenter = assets->pFontCreate->AlignText_Center(193, pText);
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 324, 395}, colorTable.Tacha, pText);  // Classes

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_KNIGHT)
        pColorText = colorTable.White;
    pTextCenter = assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_KNIGHT));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 323, 417}, pColorText, localization->GetClassName(CLASS_KNIGHT));

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_PALADIN)
        pColorText = colorTable.White;
    pTextCenter =
        assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_PALADIN));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 323, pIntervalY + 417}, pColorText, localization->GetClassName(CLASS_PALADIN));

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_RANGER)
        pColorText = colorTable.White;
    pTextCenter =
        assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_RANGER));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 323, 2 * pIntervalY + 417}, pColorText, localization->GetClassName(CLASS_RANGER));

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_CLERIC)
        pColorText = colorTable.White;
    pTextCenter =
        assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_CLERIC));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 388, 417}, pColorText, localization->GetClassName(CLASS_CLERIC));

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_DRUID)
        pColorText = colorTable.White;
    pTextCenter =
        assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_DRUID));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 388, pIntervalY + 417}, pColorText, localization->GetClassName(CLASS_DRUID));

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_SORCERER)
        pColorText = colorTable.White;
    pTextCenter =
        assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_SORCERER));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 388, 2 * pIntervalY + 417}, pColorText, localization->GetClassName(CLASS_SORCERER));

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_ARCHER)
        pColorText = colorTable.White;
    pTextCenter =
        assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_ARCHER));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 453, 417}, pColorText, localization->GetClassName(CLASS_ARCHER));

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_MONK)
        pColorText = colorTable.White;
    pTextCenter =
        assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_MONK));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 453, pIntervalY + 417}, pColorText, localization->GetClassName(CLASS_MONK));

    pColorText = colorTable.Aqua;
    if (uClassType != CLASS_THIEF)
        pColorText = colorTable.White;
    pTextCenter = assets->pFontCreate->AlignText_Center(65, localization->GetClassName(CLASS_THIEF));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 453, 2 * pIntervalY + 417}, pColorText, localization->GetClassName(CLASS_THIEF));

    pTextCenter = assets->pFontCreate->AlignText_Center(
        236, localization->GetString(LSTR_AVAILABLE_SKILLS));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 37, 395}, colorTable.Tacha, localization->GetString(LSTR_AVAILABLE_SKILLS));
    for (int i = 0; i < 9; ++i) {
        pSkillId = pParty->pCharacters[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(i + 4);
        pText = localization->GetSkillName(pSkillId);
        // trim skills that are too long
        if (pText.size() > 13)
            pText.resize(12);
        if (pText == "Body Building")
            pText = "Body Build";
        if (pText.starts_with(' '))
            pText.clear();

        pCorrective = -10;             // -5
        //if ((signed int)pLenText < 8)  // if ( (signed int)v124 > 2 )
        //    pCorrective = 0;
        pColorText = colorTable.Aqua;
        if (!pParty->pCharacters[uPlayerCreationUI_SelectedCharacter].pActiveSkills[pSkillId])
            pColorText = colorTable.White;

        pTextCenter = assets->pFontCreate->AlignText_Center(100, pText);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {100 * (i / 3) + pTextCenter + pCorrective + 17, pIntervalY * (i % 3) + 417}, pColorText, pText);
    }

    pTextCenter = assets->pFontCreate->AlignText_Center(
                0x5C, localization->GetString(LSTR_BONUS_1));
        pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 533, 394}, colorTable.Tacha, localization->GetString(LSTR_BONUS_1));

    // force draw so overlays dont get muddled
    render->DrawTwodVerts();
    render->EndTextNew();

    pBonusNum = CharacterCreation_GetUnspentAttributePointCount();

    auto unspent_attribute_bonus_label = fmt::format("{}", pBonusNum);
    pTextCenter = assets->pFontCreate->AlignText_Center(84, unspent_attribute_bonus_label);
    pGUIWindow_CurrentMenu->DrawText(assets->pFontCreate.get(), {pTextCenter + 530, 410}, colorTable.White, unspent_attribute_bonus_label);

    if (errorMessageExpireTime > pMiscTimer->time()) {
        GUIWindow message_window;
        message_window.sHint = localization->GetString(LSTR_CREATE_PARTY_CANNOT_BE_COMPLETED_UNLESS);
        if (pBonusNum < 0)
            message_window.sHint = localization->GetString(LSTR_YOU_CANT_SPEND_MORE_THAN_50_POINTS);
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
    GUIWindow(WINDOW_CharacterCreation, {0, 0}, render->GetRenderDimensions()) {
    engine->_messageQueue->clear();

    main_menu_background = assets->getImage_PCXFromIconsLOD("makeme.pcx");

    current_screen_type = SCREEN_PARTY_CREATION;
    uPlayerCreationUI_SelectedCharacter = 0;
    int v0 = assets->pFontCreate->GetHeight() - 2;

    ui_partycreation_class_icons[0] = assets->getImage_ColorKey("IC_KNIGHT");
    ui_partycreation_class_icons[1] = assets->getImage_ColorKey("IC_THIEF");
    ui_partycreation_class_icons[2] = assets->getImage_ColorKey("IC_MONK");
    ui_partycreation_class_icons[3] = assets->getImage_ColorKey("IC_PALAD");
    ui_partycreation_class_icons[4] = assets->getImage_ColorKey("IC_ARCH");
    ui_partycreation_class_icons[5] = assets->getImage_ColorKey("IC_RANGER");
    ui_partycreation_class_icons[6] = assets->getImage_ColorKey("IC_CLER");
    ui_partycreation_class_icons[7] = assets->getImage_ColorKey("IC_DRUID");
    ui_partycreation_class_icons[8] = assets->getImage_ColorKey("IC_SORC");

    ui_partycreation_top = assets->getImage_Alpha("MAKETOP");
    ui_partycreation_sky_scroller = assets->getImage_Solid("MAKESKY");

    ui_partycreation_character_frame = assets->getImage_Solid("aframe1");

    for (int uX = 0; uX < 22; ++uX) {
        ui_partycreation_portraits[uX] = assets->getImage_ColorKey(fmt::format("{}01", pPlayerPortraitsNames[uX]));
    }

    ui_partycreation_minus = assets->getImage_ColorKey("buttminu");
    ui_partycreation_plus = assets->getImage_ColorKey("buttplus");
    ui_partycreation_right = assets->getImage_ColorKey("presrigh");
    ui_partycreation_left = assets->getImage_ColorKey("presleft");

    // sprites number go from (1 to 19)
    assert(ui_partycreation_arrow_l.size() == 19);
    assert(ui_partycreation_arrow_r.size() == 19);
    for (int i = 0; i < ui_partycreation_arrow_l.size(); ++i) {
        ui_partycreation_arrow_l[i] = assets->getImage_Alpha(fmt::format("arrowl{}", i + 1));
        ui_partycreation_arrow_r[i] = assets->getImage_Alpha(fmt::format("arrowr{}", i + 1));
    }

    // pGUIWindow_CurrentMenu = new GUIWindow(0, 0, window->GetWidth(), window->GetHeight());
    int uX = 8;
    for (int characterIndex = 0; characterIndex < 4; characterIndex++) {
        CreateButton({uX, 120}, {145, 25}, 1, 0, UIMSG_PlayerCreationChangeName, characterIndex);
        uX += 158;
    }

    pCreationUI_BtnPressLeft[0] = CreateButton({10, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FacePrev, 0, Io::InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[1] = CreateButton({169, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FacePrev, 1, Io::InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[2] = CreateButton({327, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FacePrev, 2, Io::InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[3] = CreateButton({486, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FacePrev, 3, Io::InputAction::Invalid, "", {ui_partycreation_left});

    pCreationUI_BtnPressRight[0] = CreateButton({74, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FaceNext, 0, Io::InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[1] = CreateButton({233, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FaceNext, 1, Io::InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[2] = CreateButton({391, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FaceNext, 2, Io::InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[3] = CreateButton({549, 32}, {11, 13}, 1, 0, UIMSG_PlayerCreation_FaceNext, 3, Io::InputAction::Invalid, "", {ui_partycreation_right});

    pCreationUI_BtnPressLeft2[0] = CreateButton({10, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoicePrev, 0, Io::InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[1] = CreateButton({169, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoicePrev, 1, Io::InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[2] = CreateButton({327, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoicePrev, 2, Io::InputAction::Invalid, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[3] = CreateButton({486, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoicePrev, 3, Io::InputAction::Invalid, "", {ui_partycreation_left});

    pCreationUI_BtnPressRight2[0] = CreateButton({74, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoiceNext, 0, Io::InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[1] = CreateButton({233, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoiceNext, 1, Io::InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[2] = CreateButton({391, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoiceNext, 2, Io::InputAction::Invalid, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[3] = CreateButton({549, 103}, {11, 13}, 1, 0, UIMSG_PlayerCreation_VoiceNext, 3, Io::InputAction::Invalid, "", {ui_partycreation_right});

    uX = 8;
    for (int characterIndex = 0 ; characterIndex < 4; characterIndex++) {
        CreateButton({uX, 308}, {150, v0}, 1, 0, UIMSG_48, characterIndex);
        CreateButton({uX, v0 + 308}, {150, v0}, 1, 0, UIMSG_49, characterIndex);
        CreateButton({uX, 2 * v0 + 308}, {150, v0}, 1, 0, UIMSG_PlayerCreationRemoveUpSkill, characterIndex);
        CreateButton({uX, 3 * v0 + 308}, {150, v0}, 1, 0, UIMSG_PlayerCreationRemoveDownSkill, characterIndex);
        uX += 158;
    }

    CreateButton({5, 21}, {153, 365}, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 0, Io::InputAction::SelectChar1);
    CreateButton({163, 21}, {153, 365}, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 1, Io::InputAction::SelectChar2);
    CreateButton({321, 21}, {153, 365}, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 2, Io::InputAction::SelectChar3);
    CreateButton({479, 21}, {153, 365}, 1, 0, UIMSG_PlayerCreation_SelectAttribute, 3, Io::InputAction::SelectChar4);

    uX = 23;
    int uControlParam = 2;
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

    setKeyboardControlGroup(28, true, 7, 40);

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

    ui_partycreation_buttmake = assets->getImage_Solid("BUTTMAKE");
    ui_partycreation_buttmake2 = assets->getImage_Solid("BUTTMAKE2");

    pPlayerCreationUI_BtnOK = CreateButton("PartyCreation_OK", {580, 431}, {51, 39}, 1, 0, UIMSG_PlayerCreationClickOK, 0, Io::InputAction::Return, "", {ui_partycreation_buttmake});
    pPlayerCreationUI_BtnReset = CreateButton("PartyCreation_Clear", {527, 431}, {51, 39}, 1, 0, UIMSG_PlayerCreationClickReset, 0, Io::InputAction::Clear, "", {ui_partycreation_buttmake2});
    pPlayerCreationUI_BtnMinus = CreateButton({523, 393}, {20, 35}, 1, 0, UIMSG_PlayerCreationClickMinus, 0, Io::InputAction::Minus, "", {ui_partycreation_minus});
    pPlayerCreationUI_BtnPlus = CreateButton({613, 393}, {20, 35}, 1, 0, UIMSG_PlayerCreationClickPlus, 1, Io::InputAction::Plus, "", {ui_partycreation_plus});

    ui_partycreation_font = GUIFont::LoadFont("cchar.fnt", "FONTPAL");
}

GUIWindow_PartyCreation::~GUIWindow_PartyCreation() {
    main_menu_background->Release();
    main_menu_background = nullptr;
}

//----- (00497526) --------------------------------------------------------
bool PartyCreationUI_LoopInternal() {
    Item item;
    bool party_not_creation_flag;

    party_not_creation_flag = false;

    pGUIWindow_CurrentMenu->keyboard_input_status = WINDOW_INPUT_NONE;
    SetCurrentMenuID(MENU_CREATEPARTY);
    while (GetCurrentMenuID() == MENU_CREATEPARTY) {
        MessageLoopWithWait();

        pMiscTimer->tick(); // This one is used for animations.

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
    delete pGUIWindow_CurrentMenu;
    pGUIWindow_CurrentMenu = nullptr;

    item.Reset();
    for (unsigned i = 0; i < 4; ++i) {
        if (pParty->pCharacters[i].classType == CLASS_KNIGHT)
            pParty->pCharacters[i].sResMagicBase = 10;
        // TODO(pskelton): why just CHARACTER_BUFF_RESIST_WATER?
        pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_RESIST_WATER].Reset();
        for (MagicSchool page : allMagicSchools()) {
            if (pParty->pCharacters[i].pActiveSkills[skillForMagicSchool(page)]) {
                pParty->pCharacters[i].lastOpenedSpellbookPage = page;
                break;
            }
        }
        pItemTable->generateItem(ITEM_TREASURE_LEVEL_2, RANDOM_ITEM_RING, &item);
        pParty->pCharacters[i].AddItem2(-1, &item);

        pParty->pCharacters[i].health = pParty->pCharacters[i].GetMaxHealth();
        pParty->pCharacters[i].mana = pParty->pCharacters[i].GetMaxMana();
        for (CharacterSkillType j : allSkills()) {
            if (!pParty->pCharacters[i].pActiveSkills[j]) continue;

            switch (j) {
            case CHARACTER_SKILL_STAFF:
                pParty->pCharacters[i].AddItem(-1, ITEM_STAFF);
                break;
            case CHARACTER_SKILL_SWORD:
                pParty->pCharacters[i].AddItem(-1, ITEM_CRUDE_LONGSWORD);
                break;
            case CHARACTER_SKILL_DAGGER:
                pParty->pCharacters[i].AddItem(-1, ITEM_DAGGER);
                break;
            case CHARACTER_SKILL_AXE:
                pParty->pCharacters[i].AddItem(-1, ITEM_CRUDE_AXE);
                break;
            case CHARACTER_SKILL_SPEAR:
                pParty->pCharacters[i].AddItem(-1, ITEM_CRUDE_SPEAR);
                break;
            case CHARACTER_SKILL_BOW:
                pParty->pCharacters[i].AddItem(-1, ITEM_CROSSBOW);
                break;
            case CHARACTER_SKILL_MACE:
                pParty->pCharacters[i].AddItem(-1, ITEM_MACE);
                break;
            case CHARACTER_SKILL_BLASTER:
                logger->error("No blasters at startup :p");
                break;
            case CHARACTER_SKILL_SHIELD:
                pParty->pCharacters[i].AddItem(-1, ITEM_WOODEN_BUCKLER);
                break;
            case CHARACTER_SKILL_LEATHER:
                pParty->pCharacters[i].AddItem(-1, ITEM_LEATHER_ARMOR);
                break;
            case CHARACTER_SKILL_CHAIN:
                pParty->pCharacters[i].AddItem(-1, ITEM_CHAIN_MAIL);
                break;
            case CHARACTER_SKILL_PLATE:
                pParty->pCharacters[i].AddItem(-1, ITEM_PLATE_ARMOR);
                break;
            case CHARACTER_SKILL_FIRE:
                pParty->pCharacters[i].AddItem(-1, ITEM_SPELLBOOK_FIRE_BOLT);
                pParty->pCharacters[i].bHaveSpell[SPELL_FIRE_TORCH_LIGHT] = true;
                break;
            case CHARACTER_SKILL_AIR:
                pParty->pCharacters[i].AddItem(-1, ITEM_SPELLBOOK_FEATHER_FALL);
                pParty->pCharacters[i].bHaveSpell[SPELL_AIR_WIZARD_EYE] = true;
                break;
            case CHARACTER_SKILL_WATER:
                pParty->pCharacters[i].AddItem(-1, ITEM_SPELLBOOK_POISON_SPRAY);
                pParty->pCharacters[i].bHaveSpell[SPELL_WATER_AWAKEN] = true;
                break;
            case CHARACTER_SKILL_EARTH:
                pParty->pCharacters[i].AddItem(-1, ITEM_SPELLBOOK_SLOW);
                pParty->pCharacters[i].bHaveSpell[SPELL_EARTH_STUN] = true;
                break;
            case CHARACTER_SKILL_SPIRIT:
                pParty->pCharacters[i].AddItem(-1, ITEM_SPELLBOOK_BLESS);
                pParty->pCharacters[i].bHaveSpell[SPELL_SPIRIT_DETECT_LIFE] = true;
                break;
            case CHARACTER_SKILL_MIND:
                pParty->pCharacters[i].AddItem(-1, ITEM_SPELLBOOK_MIND_BLAST);
                pParty->pCharacters[i].bHaveSpell[SPELL_MIND_REMOVE_FEAR] = true;
                break;
            case CHARACTER_SKILL_BODY:
                pParty->pCharacters[i].AddItem(-1, ITEM_SPELLBOOK_HEAL);
                pParty->pCharacters[i].bHaveSpell[SPELL_BODY_CURE_WEAKNESS] = true;
                break;
            case CHARACTER_SKILL_LIGHT:
            case CHARACTER_SKILL_DARK:
                logger->error("No light/dark magic at startup");
                break;
            case CHARACTER_SKILL_DIPLOMACY:
                logger->error("No diplomacy in mm7 (yet)");
                break;
            case CHARACTER_SKILL_ITEM_ID:
            case CHARACTER_SKILL_REPAIR:
            case CHARACTER_SKILL_MEDITATION:
            case CHARACTER_SKILL_PERCEPTION:
            case CHARACTER_SKILL_TRAP_DISARM:
            case CHARACTER_SKILL_LEARNING:
                pParty->pCharacters[i].AddItem(-1, ITEM_POTION_BOTTLE);
                pParty->pCharacters[i].AddItem(-1, grng->randomSample(allLevel1Reagents()));
                break;
            case CHARACTER_SKILL_DODGE:
                pParty->pCharacters[i].AddItem(-1, ITEM_LEATHER_BOOTS);
                break;
            case CHARACTER_SKILL_UNARMED:
                pParty->pCharacters[i].AddItem(-1, ITEM_GAUNTLETS);
                break;
            case CHARACTER_SKILL_CLUB:
                // pParty->pCharacters[i].AddItem(-1, ITEM_CLUB);
                break;
            default:
                break;
            }

            for (Item &inventoryItem : pParty->pCharacters[i].pInventoryItemList) {
                if (inventoryItem.itemId != ITEM_NULL)
                    inventoryItem.SetIdentified();
            }
        }
    }

    pAudioPlayer->stopSounds();
    return party_not_creation_flag;
}
