#include "GUI/UI/UIDialogue.h"

#include "Engine/Events.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIArena.h"

#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/OSWindow.h"

using Io::TextInputType;


void GameUI_InitializeDialogue(Actor *actor, int bPlayerSaysHello) {
    dword_A74CDC = -1;
    pNPCStats->dword_AE336C_LastMispronouncedNameFirstLetter = -1;
    pEventTimer->Pause();
    pMiscTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    uDialogueType = DIALOGUE_NULL;
    sDialogue_SpeakingActorNPC_ID = actor->sNPC_ID;
    pDialogue_SpeakingActor = actor;
    NPCData *pNPCInfo = GetNPCData(actor->sNPC_ID);
    if ((pNPCInfo->uFlags & 3) != 2) {
        pNPCInfo->uFlags = pNPCInfo->uFlags + 1;
    }

    String filename;
    switch (pParty->alignment) {
    case PartyAlignment::PartyAlignment_Good:
            filename = "evt02-b";
            break;
        case PartyAlignment::PartyAlignment_Neutral:
            filename = "evt02";
            break;
        case PartyAlignment::PartyAlignment_Evil:
            filename = "evt02-c";
            break;
        default:
            Error("Invalid alignment: %u", pParty->alignment);
    }
    game_ui_dialogue_background = assets->GetImage_Solid(filename);

    pDialogueNPCCount = 0;
    uNumDialogueNPCPortraits = 1;

    filename = StringPrintf("npc%03u", pNPCInfo->uPortraitID);
    pDialogueNPCPortraits[0] = assets->GetImage_ColorKey(filename, render->teal_mask_16);

    int pNumberContacts = 0;
    int v9 = 0;
    if (!pNPCInfo->Hired() && pNPCInfo->Location2D >= 0) {
        if ((signed int)pParty->GetPartyFame() <= pNPCInfo->fame ||
            (pNumberContacts = pNPCInfo->uFlags & 0xFFFFFF7F,
             (pNumberContacts & 0x80000000u) != 0)) {
            v9 = 1;
        } else {
            if (pNumberContacts > 1) {
                if (pNumberContacts == 2) {
                    v9 = 3;
                } else {
                    if (pNumberContacts != 3) {
                        if (pNumberContacts != 4) v9 = 1;
                    } else {
                        v9 = 2;
                    }
                }
            } else if (pNPCInfo->rep) {
                v9 = 2;
            }
        }
    }
    if (sDialogue_SpeakingActorNPC_ID < 0) v9 = 4;

    pDialogueWindow = new GUIWindow_Dialogue(
        0, 0, window->GetWidth(), window->GetHeight(), (GUIButton *)3
    );
    if (pNPCInfo->Hired() && !pNPCInfo->bHasUsedTheAbility) {
        if (pNPCInfo->profession == Healer ||
            pNPCInfo->profession == ExpertHealer ||
            pNPCInfo->profession == MasterHealer ||
            pNPCInfo->profession == Cook ||
            pNPCInfo->profession == Chef ||
            pNPCInfo->profession == WindMaster ||
            pNPCInfo->profession == WaterMaster ||
            pNPCInfo->profession == GateMaster ||
            pNPCInfo->profession == Acolyte ||  // or Chaplain? mb discrepancy between game versions?
            pNPCInfo->profession == Piper ||
            pNPCInfo->profession == FallenWizard
        ) {
            pDialogueWindow->CreateButton(
                480, 250, 140, pFontArrus->GetHeight() - 3, 1, 0,
                UIMSG_SelectNPCDialogueOption, DIALOGUE_USE_HIRED_NPC_ABILITY);
            pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 1);
        }
    }

    pDialogueWindow->CreateButton(61, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 1, GameKey::Digit1);
    pDialogueWindow->CreateButton(177, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 2, GameKey::Digit2);
    pDialogueWindow->CreateButton(292, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 3, GameKey::Digit3);
    pDialogueWindow->CreateButton(407, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 4, GameKey::Digit4);

    if (bPlayerSaysHello && uActiveCharacter && !pNPCInfo->Hired()) {
        if (pParty->uCurrentHour < 5 || pParty->uCurrentHour > 21)
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_GoodEvening, 0);
        else
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_GoodDay, 0);
    }
}


GUIWindow_Dialogue::GUIWindow_Dialogue(unsigned int x, unsigned int y,
                                       unsigned int width, unsigned int height,
                                       GUIButton *button, const String &hint)
    : GUIWindow(WINDOW_Dialogue, x, y, width, height, button, hint) {
    prev_screen_type = current_screen_type;
    current_screen_type = CURRENT_SCREEN::SCREEN_NPC_DIALOGUE;
    pBtn_ExitCancel = CreateButton(
        0x1D7u, 0x1BDu, 0xA9u, 0x23u, 1, 0, UIMSG_Escape, 0, GameKey::None,
        localization->GetString(LSTR_DIALOGUE_EXIT),
        {{ui_exit_cancel_button_background}}
    );
    if (par1C != 1) {
        int num_dialugue_options = 0;
        int text_line_height = pFontArrus->GetHeight() - 3;
        NPCData *speakingNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
        if (GetGreetType(sDialogue_SpeakingActorNPC_ID) == 1) {  // QuestsNPC_greet
            if (speakingNPC->is_joinable) {
                CreateButton(
                    480, 130, 140, text_line_height, 1, 0,
                    UIMSG_SelectNPCDialogueOption,
                    DIALOGUE_13_hiring_related
                );
                num_dialugue_options = 1;
            }

            #define AddScriptedDialogueLine(DIALOGUE_EVENT_ID, MSG_PARAM) \
                if (DIALOGUE_EVENT_ID) { \
                    if (num_dialugue_options < 4) { \
                        int res = NPCDialogueEventProcessor(DIALOGUE_EVENT_ID); \
                        if (res == 1 || res == 2) \
                            CreateButton( \
                                480, 130 + num_dialugue_options++ * text_line_height, \
                                140, text_line_height, \
                                1, 0, UIMSG_SelectNPCDialogueOption, MSG_PARAM \
                            ); \
                    } \
                }

            AddScriptedDialogueLine(speakingNPC->dialogue_1_evt_id, DIALOGUE_SCRIPTED_LINE_1);
            AddScriptedDialogueLine(speakingNPC->dialogue_2_evt_id, DIALOGUE_SCRIPTED_LINE_2);
            AddScriptedDialogueLine(speakingNPC->dialogue_3_evt_id, DIALOGUE_SCRIPTED_LINE_3);
            AddScriptedDialogueLine(speakingNPC->dialogue_4_evt_id, DIALOGUE_SCRIPTED_LINE_4);
            AddScriptedDialogueLine(speakingNPC->dialogue_5_evt_id, DIALOGUE_SCRIPTED_LINE_5);
            AddScriptedDialogueLine(speakingNPC->dialogue_6_evt_id, DIALOGUE_SCRIPTED_LINE_6);
        } else {
            if (speakingNPC->is_joinable) {
                CreateButton(
                    480, 0x82u, 140, text_line_height, 1, 0,
                    UIMSG_SelectNPCDialogueOption, DIALOGUE_PROFESSION_DETAILS, GameKey::None,
                    localization->GetString(LSTR_MORE_INFORMATION)
                );
                if (speakingNPC->Hired()) {
                    CreateButton(
                        480, 130 + text_line_height, 140, text_line_height, 1, 0,
                        UIMSG_SelectNPCDialogueOption, DIALOGUE_HIRE_FIRE, GameKey::None,
                        localization->FormatString(LSTR_HIRE_RELEASE, speakingNPC->pName)
                    );
                } else {
                    CreateButton(480, 130 + text_line_height, 140, text_line_height, 1, 0,
                        UIMSG_SelectNPCDialogueOption, DIALOGUE_HIRE_FIRE, GameKey::None,
                        localization->GetString(LSTR_HIRE)
                    );
                }
                num_dialugue_options = 2;
            }
        }
        _41D08F_set_keyboard_control_group(num_dialugue_options, 1, 0, 1);
    }
}

void GUIWindow_Dialogue::Release() {
    uNumDialogueNPCPortraits = 0;

    if (game_ui_dialogue_background) {
        game_ui_dialogue_background->Release();
        game_ui_dialogue_background = nullptr;
    }

    current_screen_type = prev_screen_type;

    GUIWindow::Release();
}

void GUIWindow_Dialogue::Update() {
    if (!pDialogueWindow) {
        return;
    }

    // Window title(Заголовок окна)----
    GUIWindow window = *pDialogueWindow;
    NPCData *pNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
    int pGreetType = GetGreetType(sDialogue_SpeakingActorNPC_ID);
    window.uFrameWidth -= 10;
    window.uFrameZ -= 10;
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureAlphaNew((pNPCPortraits_x[0][0] - 4) / 640.0f,
                                (pNPCPortraits_y[0][0] - 4) / 480.0f,
                                game_ui_evtnpc);
    render->DrawTextureAlphaNew(pNPCPortraits_x[0][0] / 640.0f,
                                pNPCPortraits_y[0][0] / 480.0f,
                                pDialogueNPCPortraits[0]);

    window.DrawTitleText(
        pFontArrus, 483, 112, ui_game_dialogue_npc_name_color, NameAndTitle(pNPC), 3
    );

    pParty->GetPartyFame();

    String dialogue_string;
    switch (uDialogueType) {
        case DIALOGUE_13_hiring_related:
            dialogue_string = BuildDialogueString(
                pNPCStats->pProfessions[pNPC->profession].pJoinText,
                uActiveCharacter - 1, 0, 0, 0);
            break;

        case DIALOGUE_PROFESSION_DETAILS: {
            if (dialogue_show_profession_details) {
                dialogue_string = BuildDialogueString(
                    pNPCStats->pProfessions[pNPC->profession].pBenefits,
                    uActiveCharacter - 1, 0, 0, 0
                );
            } else if (pNPC->Hired()) {
                dialogue_string = BuildDialogueString(
                    pNPCStats->pProfessions[pNPC->profession].pDismissText,
                    uActiveCharacter - 1, 0, 0, 0
                );
            } else {
                dialogue_string = BuildDialogueString(
                    pNPCStats->pProfessions[pNPC->profession].pJoinText,
                    uActiveCharacter - 1, 0, 0, 0
                );
            }
            break;
        }

        case DIALOGUE_ARENA_WELCOME:
            dialogue_string = localization->GetString(LSTR_ARENA_WELCOME);
            break;

        case DIALOGUE_ARENA_FIGHT_NOT_OVER_YET:
            dialogue_string = localization->GetString(LSTR_ARENA_PREMATURE_EXIT);
            break;

        case DIALOGUE_ARENA_REWARD:
            dialogue_string = localization->FormatString(
                LSTR_ARENA_REWARD, gold_transaction_amount);
            break;

        case DIALOGUE_ARENA_ALREADY_WON:
            dialogue_string = localization->GetString(LSTR_ARENA_AREADY_WON);
            break;

        default:
            if (uDialogueType >= DIALOGUE_SCRIPTED_LINE_1 && uDialogueType < DIALOGUE_SCRIPTED_LINE_6 &&
                branchless_dialogue_str.empty()) {
                dialogue_string = current_npc_text;
            } else if (pGreetType == 1) {  // QuestNPC_greet
                if (pNPC->greet) {
                    if ((pNPC->uFlags & 3) == 2)
                        dialogue_string =
                            pNPCStats->pNPCGreetings[pNPC->greet].pGreeting2;
                    else
                        dialogue_string =
                            pNPCStats->pNPCGreetings[pNPC->greet].pGreeting1;
                }
            } else if (pGreetType == 2) {  // HiredNPC_greet
                NPCProfession *prof = &pNPCStats->pProfessions[pNPC->profession];

                if (pNPC->Hired())
                    dialogue_string = BuildDialogueString(
                        prof->pDismissText, uActiveCharacter - 1, 0, 0, 0);
                else
                    dialogue_string = BuildDialogueString(
                        prof->pJoinText, uActiveCharacter - 1, 0, 0, 0);
            }
            break;
    }

    int pTextHeight = 0;

    // Message window(Окно сообщения)----
    if (!dialogue_string.empty()) {
        window.uFrameWidth = game_viewport_width;
        window.uFrameZ = 452;
        GUIFont *font = pFontArrus;
        pTextHeight = pFontArrus->CalcTextHeight(dialogue_string,
                                                 window.uFrameWidth, 13) + 7;
        if (352 - pTextHeight < 8) {
            font = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(dialogue_string,
                                                      window.uFrameWidth, 13) + 7;
        }

        if (ui_leather_mm7)
            render->DrawTextureCustomHeight(8 / 640.0f,
                                            (352 - pTextHeight) / 480.0f,
                                            ui_leather_mm7, pTextHeight);

        render->DrawTextureAlphaNew(8 / 640.0f, (347 - pTextHeight) / 480.0f,
                                    _591428_endcap);
        pDialogueWindow->DrawText(
            font, 13, 354 - pTextHeight, 0,
            font->FitTextInAWindow(dialogue_string, window.uFrameWidth, 13), 0,
            0, 0);
    }

    // Right panel(Правая панель)-------
    window = *pDialogueWindow;
    window.uFrameX = 483;
    window.uFrameWidth = 148;
    window.uFrameZ = 334;
    for (int i = window.pStartingPosActiveItem;
         i < window.pStartingPosActiveItem + window.pNumPresenceButton; ++i) {
        GUIButton *pButton = window.GetControl(i);
        if (!pButton) break;

        if (pButton->msg_param > DIALOGUE_ARENA_SELECT_CHAMPION) {
            pButton->sLabel.clear();
        } else if (pButton->msg_param == DIALOGUE_ARENA_SELECT_CHAMPION) {
            pButton->sLabel = localization->GetString(LSTR_ARENA_DIFFICULTY_LORD);
        } else if (pButton->msg_param == DIALOGUE_ARENA_SELECT_KNIGHT) {
            pButton->sLabel = localization->GetString(LSTR_ARENA_DIFFICULTY_KNIGHT);
        } else if (pButton->msg_param == DIALOGUE_ARENA_SELECT_SQUIRE) {
            pButton->sLabel = localization->GetString(LSTR_ARENA_DIFFICULTY_SQUIRE);
        } else if (pButton->msg_param == DIALOGUE_ARENA_SELECT_PAGE) {
            pButton->sLabel = localization->GetString(LSTR_ARENA_DIFFICULTY_PAGE);
        } else if (pButton->msg_param == DIALOGUE_PROFESSION_DETAILS) {
            pButton->sLabel = localization->GetString(LSTR_MORE_INFORMATION);
        } else if (pButton->msg_param == DIALOGUE_HIRE_FIRE) {
            if (pNPC->Hired()) {
                pButton->sLabel = StringPrintf(
                    localization->GetString(LSTR_HIRE_RELEASE), pNPC->pName
                );
            } else {
                pButton->sLabel = localization->GetString(LSTR_HIRE);
            }
        } else if (pButton->msg_param == DIALOGUE_SCRIPTED_LINE_6) {
            if (!pNPC->dialogue_6_evt_id) {
                pButton->sLabel.clear();
                pButton->msg_param = 0;
            } else {
                pButton->sLabel = pNPCTopics[pNPC->dialogue_6_evt_id].pTopic;
            }
        } else if (pButton->msg_param == DIALOGUE_USE_HIRED_NPC_ABILITY) {
            pButton->sLabel = GetProfessionActionText(pNPC->profession);
        } else if (pButton->msg_param == DIALOGUE_SCRIPTED_LINE_1) {
            if (!pNPC->dialogue_1_evt_id) {
                pButton->sLabel.clear();
                pButton->msg_param = 0;
            } else {
                pButton->sLabel = pNPCTopics[pNPC->dialogue_1_evt_id].pTopic;
            }
        } else if (pButton->msg_param == DIALOGUE_SCRIPTED_LINE_2) {
            if (!pNPC->dialogue_2_evt_id) {
                pButton->sLabel.clear();
                pButton->msg_param = 0;
            } else {
                pButton->sLabel = pNPCTopics[pNPC->dialogue_2_evt_id].pTopic;
            }
        } else if (pButton->msg_param == DIALOGUE_SCRIPTED_LINE_3) {
            if (!pNPC->dialogue_3_evt_id) {
                pButton->sLabel.clear();
                pButton->msg_param = 0;
            } else {
                pButton->sLabel = pNPCTopics[pNPC->dialogue_3_evt_id].pTopic;
            }
        } else if (pButton->msg_param == DIALOGUE_SCRIPTED_LINE_4) {
            if (!pNPC->dialogue_4_evt_id) {
                pButton->sLabel.clear();
                pButton->msg_param = 0;
            } else {
                pButton->sLabel = pNPCTopics[pNPC->dialogue_4_evt_id].pTopic;
            }
        } else if (pButton->msg_param == DIALOGUE_SCRIPTED_LINE_5) {
            if (!pNPC->dialogue_5_evt_id) {
                pButton->sLabel.clear();
                pButton->msg_param = 0;
            } else {
                pButton->sLabel = pNPCTopics[pNPC->dialogue_5_evt_id].pTopic;
            }
        } else if (pButton->msg_param == DIALOGUE_13_hiring_related) {
            if (pNPC->Hired()) {
                pButton->sLabel = localization->FormatString(
                    LSTR_HIRE_RELEASE, pNPC->pName);
            } else {
                pButton->sLabel = localization->GetString(LSTR_JOIN);
            }
        } else {
            pButton->sLabel.clear();
        }

        if (pParty->field_7B5_in_arena_quest &&
            pParty->field_7B5_in_arena_quest != -1) {
            int num_dead_actors = 0;
            for (uint i = 0; i < uNumActors; ++i) {
                if (pActors[i].uAIState == Dead ||
                    pActors[i].uAIState == Removed ||
                    pActors[i].uAIState == Disabled) {
                    ++num_dead_actors;
                } else {
                    int sumonner_type = PID_TYPE(pActors[i].uSummonerID);
                    if (sumonner_type == OBJECT_Player) ++num_dead_actors;
                }
            }
            if (num_dead_actors == uNumActors) {
                pButton->sLabel = localization->GetString(LSTR_COLLECT_PRIZE);
            }
        }
    }

    // Install Buttons(Установка кнопок)--------
    int index = 0;
    int all_text_height = 0;
    for (int i = pDialogueWindow->pStartingPosActiveItem;
         i < pDialogueWindow->pStartingPosActiveItem +
                 pDialogueWindow->pNumPresenceButton;
         ++i) {
        GUIButton *pButton = pDialogueWindow->GetControl(i);
        if (!pButton) break;
        all_text_height +=
            pFontArrus->CalcTextHeight(pButton->sLabel, window.uFrameWidth, 0);
        index++;
    }

    if (index) {
        int v45 = (174 - all_text_height) / index;
        if (v45 > 32) v45 = 32;
        int v42 = (174 - v45 * index - all_text_height) / 2 - v45 / 2 + 138;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
             i < pDialogueWindow->pNumPresenceButton +
                     pDialogueWindow->pStartingPosActiveItem;
             ++i) {
            GUIButton *pButton = pDialogueWindow->GetControl(i);
            if (!pButton) break;
            pButton->uY = (unsigned int)(v45 + v42);
            pTextHeight = pFontArrus->CalcTextHeight(pButton->sLabel,
                                                     window.uFrameWidth, 0);
            pButton->uHeight = pTextHeight;
            v42 = pButton->uY + pTextHeight - 1;
            pButton->uW = v42;
            uint16_t pTextColor = ui_game_dialogue_option_normal_color;
            if (pDialogueWindow->pCurrentPosActiveItem == i)
                pTextColor = ui_game_dialogue_option_highlight_color;
            window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor,
                                 pButton->sLabel, 3);
        }
    }
    render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
                                ui_exit_cancel_button_background);
}

// GenericDialogue

GUIWindow_GenericDialogue::GUIWindow_GenericDialogue(
    unsigned int x, unsigned int y, unsigned int width, unsigned int height,
    GUIButton *button, const String &hint
) : GUIWindow(WINDOW_GreetingNPC, x, y, width, height, button, hint) {
    prev_screen_type = current_screen_type;
    keyboardInputHandler->StartTextInput(TextInputType::Text, 15, this);
    current_screen_type = CURRENT_SCREEN::SCREEN_BRANCHLESS_NPC_DIALOG;
}

void GUIWindow_GenericDialogue::Release() {
    current_screen_type = prev_screen_type;
    keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_CANCELLED);

    GUIWindow::Release();
}

void GUIWindow_GenericDialogue::Update() {
    int pTextHeight;                 // esi@4
    GUIWindow BranchlessDlg_window;  // [sp+D4h] [bp-58h]@4
    GUIFont *pFont;                  // [sp+128h] [bp-4h]@1

    pFont = pFontArrus;
    if (current_npc_text.length() > 0 && branchless_dialogue_str.empty())
        branchless_dialogue_str = current_npc_text.c_str();

    BranchlessDlg_window.uFrameWidth = game_viewport_width;
    BranchlessDlg_window.uFrameZ = 452;
    pTextHeight =
        pFontArrus->CalcTextHeight(branchless_dialogue_str,
                                   BranchlessDlg_window.uFrameWidth, 12) +
        7;
    if (352 - pTextHeight < 8) {
        pFont = pFontCreate;
        pTextHeight =
            pFontCreate->CalcTextHeight(branchless_dialogue_str,
                                        BranchlessDlg_window.uFrameWidth, 12) +
            7;
    }

    render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f,
                                    ui_leather_mm7, pTextHeight);
    render->DrawTextureAlphaNew(8 / 640.0f, (347 - pTextHeight) / 480.0f,
                                _591428_endcap);
    pGUIWindow2->DrawText(
        pFont, 12, 354 - pTextHeight, 0,
        pFont->FitTextInAWindow(branchless_dialogue_str,
                                BranchlessDlg_window.uFrameWidth, 12),
        0, 0, 0);
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
    if (pGUIWindow2->keyboard_input_status != WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
        if (pGUIWindow2->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
            pGUIWindow2->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
            GameUI_StatusBar_OnInput(keyboardInputHandler->GetTextInput().c_str());
            sub_4452BB();
            return;
        }
        if (pGUIWindow2->keyboard_input_status != WindowInputStatus::WINDOW_INPUT_CANCELLED)
            return;
        pGUIWindow2->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
        GameUI_StatusBar_ClearInputString();
        sub_4452BB();
        return;
    }
    if (pGUIWindow2->ptr_1C == (void *)26) {
        auto str = StringPrintf("%s %s", GameUI_StatusBar_GetInput().c_str(), keyboardInputHandler->GetTextInput().c_str());
        pGUIWindow2->DrawText(pFontLucida, 13, 357, 0, str, 0, 0, 0);
        pGUIWindow2->DrawFlashingInputCursor(pFontLucida->GetLineWidth(str) + 13, 357, pFontLucida);
        return;
    }
    if (!keyboardInputHandler->GetTextInput().empty()) {
        keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_NONE);
        GameUI_StatusBar_ClearInputString();
        sub_4452BB();
        return;
    }
}

void sub_4451A8_press_any_key(int a1, int a2, int a4) {
    if (!pGUIWindow2) {
        if (pParty->uFlags & PARTY_FLAGS_1_ForceRedraw) {
            engine->Draw();
        }
        pAudioPlayer->StopChannels(-1, -1);
        pMiscTimer->Pause();
        pEventTimer->Pause();
        dword_5C3418 = a1;
        dword_5C341C = a2;
        _591094_decoration = activeLevelDecoration;
        pGUIWindow2 = new GUIWindow_GenericDialogue(0, 0, window->GetWidth(), window->GetHeight(), (GUIButton *)a4);
        pGUIWindow2->CreateButton(61, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 1, GameKey::Digit1);
        pGUIWindow2->CreateButton(177, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 2, GameKey::Digit2);
        pGUIWindow2->CreateButton(292, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 3, GameKey::Digit3);
        pGUIWindow2->CreateButton(407, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 4, GameKey::Digit4);
    }
}


void BuildHireableNpcDialogue() {
    NPCData *v0 = GetNPCData(sDialogue_SpeakingActorNPC_ID);
    int v1 = 0;
    pDialogueWindow->eWindowType = WINDOW_MainMenu;
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow_Dialogue(0, 0, window->GetWidth(),
                                             window->GetHeight(), (GUIButton *)1);
    if (pNPCStats->pProfessions[v0->profession].pBenefits) {
        pDialogueWindow->CreateButton(
            480, 160, 140, 28, 1, 0, UIMSG_SelectNPCDialogueOption,
            DIALOGUE_PROFESSION_DETAILS, GameKey::None,
            localization->GetString(LSTR_MORE_INFORMATION));
        v1 = 1;
    }
    pDialogueWindow->CreateButton(480, 30 * v1 + 160, 140, 30, 1, 0,
        UIMSG_SelectNPCDialogueOption,
        DIALOGUE_HIRE_FIRE, GameKey::None,
        localization->GetString(LSTR_HIRE));
    pDialogueWindow->_41D08F_set_keyboard_control_group(v1 + 1, 1, 0, 1);
}

void OnSelectNPCDialogueOption(DIALOGUE_TYPE option) {
    int npc_event_id;  // ecx@10
    char *v13;         // [sp-8h] [bp-18h]@60

    NPCData *speakingNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
    uDialogueType = option;
    if (!speakingNPC->uFlags) speakingNPC->uFlags = 1;
    if (option == DIALOGUE_PROFESSION_DETAILS) {
        dialogue_show_profession_details = ~dialogue_show_profession_details;
    } else if (option == DIALOGUE_HIRE_FIRE) {
        if (speakingNPC->Hired()) {
            if ((signed int)pNPCStats->uNumNewNPCs > 0) {
                for (uint i = 0; i < (unsigned int)pNPCStats->uNumNewNPCs;
                     ++i) {
                    if (pNPCStats->pNewNPCData[i].uFlags & 0x80 &&
                        !strcmp(speakingNPC->pName,
                                pNPCStats->pNewNPCData[i].pName))
                        pNPCStats->pNewNPCData[i].uFlags &= 0x7Fu;
                }
            }
            if (pParty->pHirelings[0].pName &&
                !_stricmp(pParty->pHirelings[0].pName, speakingNPC->pName))
                memset(&pParty->pHirelings[0], 0, sizeof(NPCData));
            else if (pParty->pHirelings[1].pName &&
                     !_stricmp(pParty->pHirelings[1].pName, speakingNPC->pName))
                memset(&pParty->pHirelings[1], 0, sizeof(NPCData));
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            dword_7241C8 = 0;
            return;
        }
        if (pParty->pHirelings[0].pName && pParty->pHirelings[1].pName) {
            GameUI_SetStatusBar(LSTR_HIRE_NO_ROOM);
        } else {
            if (speakingNPC->profession != Burglar) {
                // burglars have no hiring price
                if (pParty->GetGold() < pNPCStats->pProfessions[speakingNPC->profession].uHirePrice) {
                    GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                    dialogue_show_profession_details = false;
                    uDialogueType = DIALOGUE_13_hiring_related;
                    if (uActiveCharacter)
                        pPlayers[uActiveCharacter]->PlaySound(SPEECH_NotEnoughGold, 0);
                    if (!dword_7241C8) engine->Draw();
                    dword_7241C8 = 0;
                    return;
                }
                Party::TakeGold(
                    pNPCStats->pProfessions[speakingNPC->profession].uHirePrice
                );
            }
            speakingNPC->uFlags |= 0x80u;
            if (pParty->pHirelings[0].pName) {
                memcpy(&pParty->pHirelings[1], speakingNPC, sizeof(pParty->pHirelings[1]));
                v13 = pParty->pHireling2Name;
            } else {
                memcpy(&pParty->pHirelings[0], speakingNPC, sizeof(pParty->pHirelings[0]));
                v13 = pParty->pHireling1Name;
            }
            strcpy(v13, speakingNPC->pName);
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            if (sDialogue_SpeakingActorNPC_ID >= 0)
                pDialogue_SpeakingActor->uAIState = Removed;
            if (uActiveCharacter)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_HireNPC, 0);
        }
    } else if (option >= DIALOGUE_ARENA_SELECT_PAGE && option <= DIALOGUE_ARENA_SELECT_CHAMPION) {
        ArenaFight();
        return;
    } else if (option == DIALOGUE_USE_HIRED_NPC_ABILITY) {
        if (UseNPCSkill(speakingNPC->profession) == 0) {
            if (speakingNPC->profession != GateMaster) {
                speakingNPC->bHasUsedTheAbility = 1;
            }
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
        } else {
            GameUI_SetStatusBar(LSTR_RATIONS_FULL);
        }
    } else if (option == DIALOGUE_13_hiring_related) {
        if (!speakingNPC->Hired()) {
            BuildHireableNpcDialogue();
            dialogue_show_profession_details = false;
        } else {
            for (uint i = 0; i < (signed int)pNPCStats->uNumNewNPCs; ++i) {
                if (pNPCStats->pNewNPCData[i].uFlags & 0x80 &&
                    !strcmp(speakingNPC->pName,
                            pNPCStats->pNewNPCData[i].pName))
                    pNPCStats->pNewNPCData[i].uFlags &= 0x7Fu;
            }
            if (pParty->pHirelings[0].pName &&
                !_stricmp(pParty->pHirelings[0].pName, speakingNPC->pName))
                memset(&pParty->pHirelings[0], 0, sizeof(NPCData));
            else if (pParty->pHirelings[1].pName &&
                     !_stricmp(pParty->pHirelings[1].pName, speakingNPC->pName))
                memset(&pParty->pHirelings[1], 0, sizeof(NPCData));
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            dword_7241C8 = 0;
            return;
        }
    } else if (option >= DIALOGUE_SCRIPTED_LINE_1 && option <= DIALOGUE_SCRIPTED_LINE_6) {
        switch (option) {
            case DIALOGUE_SCRIPTED_LINE_1:
                npc_event_id = speakingNPC->dialogue_1_evt_id;
                break;
            case DIALOGUE_SCRIPTED_LINE_2:
                npc_event_id = speakingNPC->dialogue_2_evt_id;
                break;
            case DIALOGUE_SCRIPTED_LINE_3:
                npc_event_id = speakingNPC->dialogue_3_evt_id;
                break;
            case DIALOGUE_SCRIPTED_LINE_4:
                npc_event_id = speakingNPC->dialogue_4_evt_id;
                break;
            case DIALOGUE_SCRIPTED_LINE_5:
                npc_event_id = speakingNPC->dialogue_5_evt_id;
                break;
            case DIALOGUE_SCRIPTED_LINE_6:
                npc_event_id = speakingNPC->dialogue_6_evt_id;
                break;
        }
        if ((npc_event_id >= 200) && (npc_event_id <= 310)) {
            _4B3FE5_training_dialogue(npc_event_id);
        } else if ((npc_event_id >= 400) && (npc_event_id <= 410)) {
            _dword_F8B1D8_last_npc_topic_menu = option;
            DrawJoinGuildWindow((GUILD_ID)(npc_event_id - 400));
        } else {
            switch (npc_event_id) {
                case 139:
                    OracleDialogue();
                    break;
                case 311:
                    CheckBountyRespawnAndAward();
                    break;
                case 399:
                    Arena_SelectionFightLevel();
                    break;
                default:
                    activeLevelDecoration = (LevelDecoration *)1;
                    current_npc_text.clear();
                    EventProcessor(npc_event_id, 0, 1);
                    activeLevelDecoration = nullptr;
                    break;
            }
        }
    }
    if (!dword_7241C8) engine->Draw();
    dword_7241C8 = 0;
}
