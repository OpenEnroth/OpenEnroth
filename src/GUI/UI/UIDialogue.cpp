#include <vector>

#include "UIDialogue.h"

#include "Engine/Events/Processor.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"
#include "Engine/Pid.h"
#include "Engine/mm7_data.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/NPCTopics.h"

#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"

using Io::TextInputType;

const IndexedArray<std::string, PartyAlignment_Good, PartyAlignment_Evil> dialogueBackgroundResourceByAlignment = {
    {PartyAlignment_Good, "evt02-b"},
    {PartyAlignment_Neutral, "evt02"},
    {PartyAlignment_Evil, "evt02-c"}
};

void GameUI_InitializeDialogue(Actor *actor, int bPlayerSaysHello) {
    currentAddressingAwardBit = -1;
    pNPCStats->dword_AE336C_LastMispronouncedNameFirstLetter = -1;
    pEventTimer->Pause();
    pMiscTimer->Pause();
    uDialogueType = DIALOGUE_NULL;
    sDialogue_SpeakingActorNPC_ID = actor->npcId;
    pDialogue_SpeakingActor = actor;
    NPCData *pNPCInfo = GetNPCData(actor->npcId);
    if (!(pNPCInfo->uFlags & NPC_GREETED_SECOND)) {
        if (pNPCInfo->uFlags & NPC_GREETED_FIRST) {
            pNPCInfo->uFlags &= ~NPC_GREETED_FIRST;
            pNPCInfo->uFlags |= NPC_GREETED_SECOND;
        } else {
            pNPCInfo->uFlags |= NPC_GREETED_FIRST;
        }
    }

    game_ui_dialogue_background = assets->getImage_Solid(dialogueBackgroundResourceByAlignment[pParty->alignment]);

    currentHouseNpc = 0;

    HouseNpcDesc desc;
    desc.type = HOUSE_NPC;
    desc.label = localization->FormatString(LSTR_FMT_CONVERSE_WITH_S, pNPCInfo->pName);
    desc.icon = assets->getImage_ColorKey(fmt::format("npc{:03}", pNPCInfo->uPortraitID));
    desc.npc = pNPCInfo;

    houseNpcs.push_back(desc);

    // TODO(Nik-RE-dev): this looks like checks for NPC that only talk if party has enough fame
    //                   which is a thing only for MM8 if I remember correctly
#if 0
    int pNumberContacts = 0;
    int v9 = 0;
    if (!pNPCInfo->Hired() && pNPCInfo->Location2D >= 0) {
        if (pParty->getPartyFame() <= pNPCInfo->fame ||
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
#endif

    pDialogueWindow = new GUIWindow_Dialogue({0, 0}, render->GetRenderDimensions(), 3);
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
            pDialogueWindow->CreateButton({480, 250}, {140, pFontArrus->GetHeight() - 3}, 1, 0,
                UIMSG_SelectNPCDialogueOption, DIALOGUE_USE_HIRED_NPC_ABILITY);
            pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 1);
        }
    }

    pDialogueWindow->CreateButton({61, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1);
    pDialogueWindow->CreateButton({177, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2);
    pDialogueWindow->CreateButton({292, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3);
    pDialogueWindow->CreateButton({407, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4);

    if (bPlayerSaysHello && pParty->hasActiveCharacter() && !pNPCInfo->Hired()) {
        if (pParty->uCurrentHour < 5 || pParty->uCurrentHour > 21) {
            pParty->activeCharacter().playReaction(SPEECH_GOOD_EVENING);
        } else {
            pParty->activeCharacter().playReaction(SPEECH_GOOD_DAY);
        }
    }
}


GUIWindow_Dialogue::GUIWindow_Dialogue(Pointi position, Sizei dimensions, WindowData data, const std::string &hint)
    : GUIWindow(WINDOW_Dialogue, position, dimensions, data, hint) {
    prev_screen_type = current_screen_type;
    current_screen_type = SCREEN_NPC_DIALOGUE;
    pBtn_ExitCancel = CreateButton({0x1D7u, 0x1BDu}, {0xA9u, 0x23u}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
        localization->GetString(LSTR_DIALOGUE_EXIT),
        {ui_exit_cancel_button_background}
    );
    if (wData.val != 1) {
        int num_dialugue_options = 0;
        int text_line_height = pFontArrus->GetHeight() - 3;
        NPCData *speakingNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
        if (getNPCType(sDialogue_SpeakingActorNPC_ID) == NPC_TYPE_QUEST) {
            if (speakingNPC->is_joinable) {
                CreateButton({480, 130}, {140, text_line_height}, 1, 0, UIMSG_SelectNPCDialogueOption, DIALOGUE_13_hiring_related);
                num_dialugue_options = 1;
            }

            #define AddScriptedDialogueLine(DIALOGUE_EVENT_ID, MSG_PARAM) \
                if (DIALOGUE_EVENT_ID) { \
                    if (num_dialugue_options < 4) { \
                        int res = npcDialogueEventProcessor(DIALOGUE_EVENT_ID); \
                        if (res == 1 || res == 2) \
                            CreateButton({480, 130 + num_dialugue_options++ * text_line_height}, {140, text_line_height}, 1, 0, \
                                UIMSG_SelectNPCDialogueOption, MSG_PARAM \
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
                CreateButton({480, 0x82u}, {140, text_line_height}, 1, 0,
                    UIMSG_SelectNPCDialogueOption, DIALOGUE_PROFESSION_DETAILS, Io::InputAction::Invalid,
                    localization->GetString(LSTR_MORE_INFORMATION));
                if (speakingNPC->Hired()) {
                    CreateButton({480, 130 + text_line_height}, {140, text_line_height}, 1, 0,
                        UIMSG_SelectNPCDialogueOption, DIALOGUE_HIRE_FIRE, Io::InputAction::Invalid,
                        localization->FormatString(LSTR_HIRE_RELEASE, speakingNPC->pName)
                    );
                } else {
                    CreateButton({480, 130 + text_line_height}, {140, text_line_height}, 1, 0,
                                 UIMSG_SelectNPCDialogueOption, DIALOGUE_HIRE_FIRE, Io::InputAction::Invalid,
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
    if (houseNpcs[0].icon) {
        houseNpcs[0].icon->Release();
    }
    houseNpcs.clear();

    if (game_ui_dialogue_background) {
        game_ui_dialogue_background->Release();
        game_ui_dialogue_background = nullptr;
    }

    current_screen_type = prev_screen_type;
    pParty->switchToNextActiveCharacter();
    GUIWindow::Release();
}

void GUIWindow_Dialogue::Update() {
    if (!pDialogueWindow) {
        return;
    }

    // Window title(Заголовок окна)----
    GUIWindow window = *pDialogueWindow;
    NPCData *pNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
    NpcType npcType = getNPCType(sDialogue_SpeakingActorNPC_ID);
    window.uFrameWidth -= 10;
    window.uFrameZ -= 10;
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureNew((pNPCPortraits_x[0][0] - 4) / 640.0f,
                                (pNPCPortraits_y[0][0] - 4) / 480.0f,
                                game_ui_evtnpc);
    render->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f,
                                pNPCPortraits_y[0][0] / 480.0f,
                                houseNpcs[0].icon);

    window.DrawTitleText(
        pFontArrus, SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, ui_game_dialogue_npc_name_color, NameAndTitle(pNPC), 3
    );

    pParty->getPartyFame();

    std::string dialogue_string;
    switch (uDialogueType) {
        case DIALOGUE_13_hiring_related:
            dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->profession].pJoinText, 0, 0, HOUSE_INVALID, 0);
            break;

        case DIALOGUE_PROFESSION_DETAILS: {
            if (dialogue_show_profession_details) {
                dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->profession].pBenefits, 0, 0, HOUSE_INVALID, 0);
            } else if (pNPC->Hired()) {
                dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->profession].pDismissText, 0, 0, HOUSE_INVALID, 0);
            } else {
                dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->profession].pJoinText, 0, 0, HOUSE_INVALID, 0);
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
            dialogue_string = localization->FormatString(LSTR_ARENA_REWARD, gold_transaction_amount);
            break;

        case DIALOGUE_ARENA_ALREADY_WON:
            dialogue_string = localization->GetString(LSTR_ARENA_AREADY_WON);
            break;

        default:
            if (uDialogueType >= DIALOGUE_SCRIPTED_LINE_1 && uDialogueType < DIALOGUE_SCRIPTED_LINE_6 &&
                branchless_dialogue_str.empty()) {
                dialogue_string = current_npc_text;
            } else if (npcType == NPC_TYPE_QUEST) {
                if (pNPC->greet) {
                    if (pNPC->uFlags & NPC_GREETED_SECOND)
                        dialogue_string = pNPCStats->pNPCGreetings[pNPC->greet].pGreeting2;
                    else
                        dialogue_string = pNPCStats->pNPCGreetings[pNPC->greet].pGreeting1;
                }
            } else if (npcType == NPC_TYPE_HIREABLE) {
                NPCProfession *prof = &pNPCStats->pProfessions[pNPC->profession];

                if (pNPC->Hired())
                    dialogue_string = BuildDialogueString(prof->pDismissText, 0, 0, HOUSE_INVALID, 0);
                else
                    dialogue_string = BuildDialogueString(prof->pJoinText, 0, 0, HOUSE_INVALID, 0);
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

        render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f,
                                    _591428_endcap);
        pDialogueWindow->DrawText(font, {13, 354 - pTextHeight}, colorTable.White, font->FitTextInAWindow(dialogue_string, window.uFrameWidth, 13));
    }

    // Right panel(Правая панель)-------
    window = *pDialogueWindow;
    window.uFrameX = SIDE_TEXT_BOX_POS_X;
    window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
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
                // TODO(captainurist): what if fmt throws?
                pButton->sLabel = fmt::sprintf(localization->GetString(LSTR_HIRE_RELEASE), pNPC->pName);
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
                pButton->sLabel = localization->FormatString(LSTR_HIRE_RELEASE, pNPC->pName);
            } else {
                pButton->sLabel = localization->GetString(LSTR_JOIN);
            }
        } else {
            pButton->sLabel.clear();
        }

        if (pParty->field_7B5_in_arena_quest &&
            pParty->field_7B5_in_arena_quest != -1) {
            int num_dead_actors = 0;
            for (uint i = 0; i < pActors.size(); ++i) {
                if (pActors[i].aiState == Dead ||
                    pActors[i].aiState == Removed ||
                    pActors[i].aiState == Disabled) {
                    ++num_dead_actors;
                } else {
                    ObjectType sumonner_type = pActors[i].summonerId.type();
                    if (sumonner_type == OBJECT_Character) ++num_dead_actors;
                }
            }
            if (num_dead_actors == pActors.size()) {
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
        int v45 = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / index;
        if (v45 > SIDE_TEXT_BOX_MAX_SPACING) v45 = SIDE_TEXT_BOX_MAX_SPACING;
        int v42 = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - v45 * index - all_text_height) / 2 - v45 / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
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
            Color pTextColor = ui_game_dialogue_option_normal_color;
            if (pDialogueWindow->pCurrentPosActiveItem == i)
                pTextColor = ui_game_dialogue_option_highlight_color;
            window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor,
                                 pButton->sLabel, 3);
        }
    }
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f,
                                ui_exit_cancel_button_background);
}

// GenericDialogue

GUIWindow_GenericDialogue::GUIWindow_GenericDialogue(Pointi position, Sizei dimensions, WindowData data, const std::string &hint)
: GUIWindow(WINDOW_GreetingNPC, position, dimensions, data, hint) {
    prev_screen_type = current_screen_type;
    keyboardInputHandler->StartTextInput(TextInputType::Text, 15, this);
    current_screen_type = SCREEN_BRANCHLESS_NPC_DIALOG;
}

void GUIWindow_GenericDialogue::Release() {
    current_screen_type = prev_screen_type;
    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);

    GUIWindow::Release();
}

void GUIWindow_GenericDialogue::Update() {
    GUIFont *pFont = pFontArrus;

    if (current_npc_text.length() > 0 && branchless_dialogue_str.empty())
        branchless_dialogue_str = current_npc_text;

    GUIWindow BranchlessDlg_window;
    BranchlessDlg_window.uFrameWidth = game_viewport_width;
    BranchlessDlg_window.uFrameZ = 452;
    int pTextHeight =
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
    render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f,
                                _591428_endcap);
    pGUIWindow_BranchlessDialogue->DrawText(pFont, {12, 354 - pTextHeight}, colorTable.White,
        pFont->FitTextInAWindow(branchless_dialogue_str, BranchlessDlg_window.uFrameWidth, 12));
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);

    // TODO(Nik-RE-dev): this code related to text input in MM6/MM8, revisit
    // this functionality when it's time to support it.
#if 0
    if (pGUIWindow_BranchlessDialogue->keyboard_input_status != WINDOW_INPUT_IN_PROGRESS) {
        if (pGUIWindow_BranchlessDialogue->keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
            pGUIWindow_BranchlessDialogue->keyboard_input_status = WINDOW_INPUT_NONE;
            GameUI_StatusBar_OnInput(keyboardInputHandler->GetTextInput());
            ReleaseBranchlessDialogue();
            return;
        }
        if (pGUIWindow_BranchlessDialogue->keyboard_input_status != WINDOW_INPUT_CANCELLED)
            return;
        pGUIWindow_BranchlessDialogue->keyboard_input_status = WINDOW_INPUT_NONE;
        GameUI_StatusBar_ClearInputString();
        ReleaseBranchlessDialogue();
        return;
    }

    if (pGUIWindow_BranchlessDialogue->wData.val == (int)EVENT_InputString) {
        auto str = fmt::format("{} {}", GameUI_StatusBar_GetInput(), keyboardInputHandler->GetTextInput());
        pGUIWindow_BranchlessDialogue->DrawText(pFontLucida, {13, 357}, colorTable.White, str);
        pGUIWindow_BranchlessDialogue->DrawFlashingInputCursor(pFontLucida->GetLineWidth(str) + 13, 357, pFontLucida);
        return;
    }
#endif

    // Close branchless dialog on any keypress
    if (!keyboardInputHandler->GetTextInput().empty()) {
        keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_NONE);
        ReleaseBranchlessDialogue();
        return;
    }
}

void StartBranchlessDialogue(int eventid, int entryline, int event) {
    if (!pGUIWindow_BranchlessDialogue) {
        if (pParty->uFlags & PARTY_FLAGS_1_ForceRedraw) {
            engine->Draw();
        }
        pMiscTimer->Pause();
        pEventTimer->Pause();
        savedEventID = eventid;
        savedEventStep = entryline;
        savedDecoration = activeLevelDecoration;
        pGUIWindow_BranchlessDialogue = new GUIWindow_GenericDialogue({0, 0}, render->GetRenderDimensions(), event);
        pGUIWindow_BranchlessDialogue->CreateButton({61, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1);
        pGUIWindow_BranchlessDialogue->CreateButton({177, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2);
        pGUIWindow_BranchlessDialogue->CreateButton({292, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3);
        pGUIWindow_BranchlessDialogue->CreateButton({407, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4);
    }
}

void ReleaseBranchlessDialogue() {
    pGUIWindow_BranchlessDialogue->Release();
    pGUIWindow_BranchlessDialogue = nullptr;
    if (savedEventID) {
        // Do not run event engine whith no event, it may happen when you close talk window
        // with NPC that only say catch phrases
        activeLevelDecoration = savedDecoration;
        eventProcessor(savedEventID, Pid(), 1, savedEventStep);
    }
    activeLevelDecoration = nullptr;
    pEventTimer->Resume();
}

void BuildHireableNpcDialogue() {
    NPCData *v0 = GetNPCData(sDialogue_SpeakingActorNPC_ID);
    int v1 = 0;
    pDialogueWindow->eWindowType = WINDOW_MainMenu;
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow_Dialogue({0, 0}, render->GetRenderDimensions(), 1);
    if (!pNPCStats->pProfessions[v0->profession].pBenefits.empty()) {
        pDialogueWindow->CreateButton({480, 160}, {140, 28}, 1, 0,
            UIMSG_SelectNPCDialogueOption, DIALOGUE_PROFESSION_DETAILS, Io::InputAction::Invalid,
            localization->GetString(LSTR_MORE_INFORMATION));
        v1 = 1;
    }
    pDialogueWindow->CreateButton({480, 30 * v1 + 160}, {140, 30}, 1, 0,
        UIMSG_SelectNPCDialogueOption, DIALOGUE_HIRE_FIRE, Io::InputAction::Invalid,
        localization->GetString(LSTR_HIRE));
    pDialogueWindow->_41D08F_set_keyboard_control_group(v1 + 1, 1, 0, 1);
}

void OnSelectNPCDialogueOption(DIALOGUE_TYPE option) {
    NPCData *speakingNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
    uDialogueType = option;
    if (!speakingNPC->uFlags)
        speakingNPC->uFlags = NPC_GREETED_FIRST;
    if (option == DIALOGUE_PROFESSION_DETAILS) {
        dialogue_show_profession_details = ~dialogue_show_profession_details;
    } else if (option == DIALOGUE_HIRE_FIRE) {
        if (speakingNPC->Hired()) {
            if ((signed int)pNPCStats->uNumNewNPCs > 0) {
                for (uint i = 0; i < (unsigned int)pNPCStats->uNumNewNPCs; ++i) {
                    if (pNPCStats->pNewNPCData[i].Hired() && speakingNPC->pName == pNPCStats->pNewNPCData[i].pName)
                        pNPCStats->pNewNPCData[i].uFlags &= ~NPC_HIRED;
                }
            }
            if (iequals(pParty->pHirelings[0].pName, speakingNPC->pName))
                pParty->pHirelings[0] = NPCData();
            else if (iequals(pParty->pHirelings[1].pName, speakingNPC->pName))
                pParty->pHirelings[1] = NPCData();
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            return;
        }
        if (!pParty->pHirelings[0].pName.empty() && !pParty->pHirelings[1].pName.empty()) {
            engine->_statusBar->setEvent(LSTR_HIRE_NO_ROOM);
        } else {
            if (speakingNPC->profession != Burglar) {
                // burglars have no hiring price
                if (pParty->GetGold() < pNPCStats->pProfessions[speakingNPC->profession].uHirePrice) {
                    engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
                    dialogue_show_profession_details = false;
                    uDialogueType = DIALOGUE_13_hiring_related;
                    if (pParty->hasActiveCharacter()) {
                        pParty->activeCharacter().playReaction(SPEECH_NOT_ENOUGH_GOLD);
                    }
                    engine->Draw();
                    return;
                }
                pParty->TakeGold(pNPCStats->pProfessions[speakingNPC->profession].uHirePrice);
            }
            speakingNPC->uFlags |= NPC_HIRED;
            if (!pParty->pHirelings[0].pName.empty()) {
                pParty->pHirelings[1] = *speakingNPC;
                pParty->pHireling2Name = speakingNPC->pName;
            } else {
                pParty->pHirelings[0] = *speakingNPC;
                pParty->pHireling1Name = speakingNPC->pName;
            }
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            if (sDialogue_SpeakingActorNPC_ID >= 0)
                pDialogue_SpeakingActor->aiState = Removed;
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_HIRE_NPC);
            }
        }
    } else if (option >= DIALOGUE_ARENA_SELECT_PAGE && option <= DIALOGUE_ARENA_SELECT_CHAMPION) {
        ArenaFight();
        return;
    } else if (option == DIALOGUE_USE_HIRED_NPC_ABILITY) {
        int hirelingId;
        for (hirelingId = 0; hirelingId < pParty->pHirelings.size(); hirelingId++) {
            if (iequals(pParty->pHirelings[hirelingId].pName, speakingNPC->pName)) {
                break;
            }
        }
        assert(hirelingId < pParty->pHirelings.size());
        if (UseNPCSkill(speakingNPC->profession, hirelingId) == 0) {
            if (speakingNPC->profession != GateMaster) {
                speakingNPC->bHasUsedTheAbility = 1;
            }
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        } else {
            engine->_statusBar->setEvent(LSTR_RATIONS_FULL);
        }
    } else if (option == DIALOGUE_13_hiring_related) {
        if (!speakingNPC->Hired()) {
            BuildHireableNpcDialogue();
            dialogue_show_profession_details = false;
        } else {
            for (uint i = 0; i < (signed int)pNPCStats->uNumNewNPCs; ++i) {
                if (pNPCStats->pNewNPCData[i].Hired() && speakingNPC->pName == pNPCStats->pNewNPCData[i].pName)
                    pNPCStats->pNewNPCData[i].uFlags &= ~NPC_HIRED;
            }
            if (iequals(pParty->pHirelings[0].pName, speakingNPC->pName))
                pParty->pHirelings[0] = NPCData();
            else if (iequals(pParty->pHirelings[1].pName, speakingNPC->pName))
                pParty->pHirelings[1] = NPCData();
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            return;
        }
    } else if (option >= DIALOGUE_SCRIPTED_LINE_1 && option <= DIALOGUE_SCRIPTED_LINE_6) {
        int npc_event_id;
        if (option == DIALOGUE_SCRIPTED_LINE_1) {
            npc_event_id = speakingNPC->dialogue_1_evt_id;
        } else if (option == DIALOGUE_SCRIPTED_LINE_2) {
            npc_event_id = speakingNPC->dialogue_2_evt_id;
        } else if (option == DIALOGUE_SCRIPTED_LINE_3) {
            npc_event_id = speakingNPC->dialogue_3_evt_id;
        } else if (option == DIALOGUE_SCRIPTED_LINE_4) {
            npc_event_id = speakingNPC->dialogue_4_evt_id;
        } else if (option == DIALOGUE_SCRIPTED_LINE_5) {
            npc_event_id = speakingNPC->dialogue_5_evt_id;
        } else {
            assert(option == DIALOGUE_SCRIPTED_LINE_6);
            npc_event_id = speakingNPC->dialogue_6_evt_id;
        }

        std::vector<DIALOGUE_TYPE> topics = handleScriptedNPCTopicSelection(option, npc_event_id);

        // TODO(Nik-RE-dev): must create buttons when overworld NPC topics will be supported
        assert(topics.size() == 0);
    }
    engine->Draw();
}
