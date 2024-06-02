#include "UIDialogue.h"

#include <vector>
#include <string>

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Objects/Decoration.h"
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
#include "GUI/UI/NPCTopics.h"

#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/String/Ascii.h"

using Io::TextInputType;

int speakingNpcId;
Actor *currentSpeakingActor = nullptr;

const IndexedArray<std::string, PartyAlignment_Good, PartyAlignment_Evil> dialogueBackgroundResourceByAlignment = {
    {PartyAlignment_Good, "evt02-b"},
    {PartyAlignment_Neutral, "evt02"},
    {PartyAlignment_Evil, "evt02-c"}
};

void initializeNPCDialogue(int npcId, int bPlayerSaysHello, Actor *actor) {
    currentAddressingAwardBit = -1;
    pNPCStats->dword_AE336C_LastMispronouncedNameFirstLetter = -1;
    pEventTimer->setPaused(true);
    pMiscTimer->setPaused(true);
    speakingNpcId = npcId;
    currentSpeakingActor = actor;
    NPCData *pNPCInfo = getNPCData(npcId);
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
    desc.label = localization->FormatString(LSTR_FMT_CONVERSE_WITH_S, pNPCInfo->name);
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
    if (speakingNpcId < 0) v9 = 4;
#endif

    pDialogueWindow = new GUIWindow_Dialogue(DIALOG_WINDOW_FULL);

    if (bPlayerSaysHello && pParty->hasActiveCharacter() && !pNPCInfo->Hired()) {
        if (pParty->uCurrentHour < 5 || pParty->uCurrentHour > 21) {
            pParty->activeCharacter().playReaction(SPEECH_GOOD_EVENING);
        } else {
            pParty->activeCharacter().playReaction(SPEECH_GOOD_DAY);
        }
    }
}

GUIWindow_Dialogue::GUIWindow_Dialogue(DialogWindowType type) : GUIWindow(WINDOW_Dialogue, {0, 0}, render->GetRenderDimensions()) {
    prev_screen_type = current_screen_type;
    current_screen_type = SCREEN_NPC_DIALOGUE;
    pBtn_ExitCancel = CreateButton({0x1D7u, 0x1BDu}, {0xA9u, 0x23u}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
                                   localization->GetString(LSTR_DIALOGUE_EXIT), {ui_exit_cancel_button_background});

    int text_line_height = assets->pFontArrus->GetHeight() - 3;
    NPCData *speakingNPC = getNPCData(speakingNpcId);
    std::vector<DialogueId> optionList;

    if (type == DIALOG_WINDOW_FULL) {
        if (getNPCType(speakingNpcId) == NPC_TYPE_QUEST) {
            optionList = prepareScriptedNPCDialogueTopics(speakingNPC);
        } else if (speakingNPC->is_joinable) {
            optionList = {DIALOGUE_PROFESSION_DETAILS, DIALOGUE_HIRE_FIRE};
        }
        if (speakingNPC->Hired() && !speakingNPC->bHasUsedTheAbility) {
            if (speakingNPC->profession == Healer || speakingNPC->profession == ExpertHealer ||
                speakingNPC->profession == MasterHealer || speakingNPC->profession == Cook ||
                speakingNPC->profession == Chef || speakingNPC->profession == WindMaster ||
                speakingNPC->profession == WaterMaster || speakingNPC->profession == GateMaster ||
                speakingNPC->profession == Acolyte ||  // or Chaplain? mb discrepancy between game versions?
                speakingNPC->profession == Piper || speakingNPC->profession == FallenWizard) {
                optionList.push_back(DIALOGUE_USE_HIRED_NPC_ABILITY);
                // TODO(Nik-RE-dev): this is for compatability. Previously when NPC can use ability, dialogue allocated 4 buttons unconditionally.
                //                   Without it many test will fail because of changed buttons positions.
                optionList.push_back(DIALOGUE_NULL);
            }
        }
    } else {
        assert(type == DIALOG_WINDOW_HIRE_FIRE_SHORT);
        if (!pNPCStats->pProfessions[speakingNPC->profession].pBenefits.empty()) {
            optionList.push_back(DIALOGUE_PROFESSION_DETAILS);
        }
        optionList.push_back(DIALOGUE_HIRE_FIRE);
    }
    for (int i = 0; i < optionList.size(); i++) {
        CreateButton({480, 130 + i * text_line_height}, {140, text_line_height}, 1, 0, UIMSG_SelectNPCDialogueOption, std::to_underlying(optionList[i]), Io::InputAction::Invalid, "");
    }
    setKeyboardControlGroup(optionList.size(), false, 0, 1);

    CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1, "");
    CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2, "");
    CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3, "");
    CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4, "");
    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, Io::InputAction::CharCycle, "");
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
    currentSpeakingActor = nullptr;
    pParty->switchToNextActiveCharacter();
    GUIWindow::Release();
}

void GUIWindow_Dialogue::Update() {
    if (!pDialogueWindow) {
        return;
    }

    // Window title(Заголовок окна)----
    GUIWindow window = *pDialogueWindow;
    NPCData *pNPC = getNPCData(speakingNpcId);
    NpcType npcType = getNPCType(speakingNpcId);
    window.uFrameWidth -= 10;
    window.uFrameZ -= 10;
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureNew((pNPCPortraits_x[0][0] - 4) / 640.0f, (pNPCPortraits_y[0][0] - 4) / 480.0f, game_ui_evtnpc);
    render->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, houseNpcs[0].icon);

    window.DrawTitleText(assets->pFontArrus.get(), SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, ui_game_dialogue_npc_name_color, NameAndTitle(pNPC), 3);

    pParty->getPartyFame();

    std::string dialogue_string;
    switch (_displayedDialogue) {
        case DIALOGUE_13_hiring_related:
            dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->profession].pJoinText, 0, pNPC);
            break;

        case DIALOGUE_PROFESSION_DETAILS: {
            if (dialogue_show_profession_details) {
                dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->profession].pBenefits, 0, pNPC);
            } else if (pNPC->Hired()) {
                dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->profession].pDismissText, 0, pNPC);
            } else {
                dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->profession].pJoinText, 0, pNPC);
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
            if (_displayedDialogue >= DIALOGUE_SCRIPTED_LINE_1 && _displayedDialogue < DIALOGUE_SCRIPTED_LINE_6 &&
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

                if (pNPC->Hired()) {
                    dialogue_string = BuildDialogueString(prof->pDismissText, 0, pNPC);
                } else {
                    dialogue_string = BuildDialogueString(prof->pJoinText, 0, pNPC);
                }
            }
            break;
    }

    int pTextHeight = 0;

    // Message window(Окно сообщения)----
    if (!dialogue_string.empty()) {
        window.uFrameWidth = game_viewport_width;
        window.uFrameZ = 452;
        GUIFont *font = assets->pFontArrus.get();
        pTextHeight = assets->pFontArrus->CalcTextHeight(dialogue_string, window.uFrameWidth, 13) + 7;
        if (352 - pTextHeight < 8) {
            font = assets->pFontCreate.get();
            pTextHeight = assets->pFontCreate->CalcTextHeight(dialogue_string, window.uFrameWidth, 13) + 7;
        }

        if (ui_leather_mm7)
            render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);

        render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        pDialogueWindow->DrawText(font, {13, 354 - pTextHeight}, colorTable.White, font->FitTextInAWindow(dialogue_string, window.uFrameWidth, 13));
    }

    // Right panel(Правая панель)-------
    window = *pDialogueWindow;
    window.uFrameX = SIDE_TEXT_BOX_POS_X;
    window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
    for (int i = window.pStartingPosActiveItem; i < window.pStartingPosActiveItem + window.pNumPresenceButton; ++i) {
        GUIButton *pButton = window.GetControl(i);
        if (!pButton) {
            break;
        }

        DialogueId topic = (DialogueId)pButton->msg_param;
        pButton->sLabel = npcDialogueOptionString(topic, pNPC);
        if (pButton->sLabel.empty() && topic >= DIALOGUE_SCRIPTED_LINE_1 && topic <= DIALOGUE_SCRIPTED_LINE_6) {
            pButton->msg_param = 0;
        }

        if (pParty->arenaState == ARENA_STATE_FIGHTING) {
            int num_dead_actors = 0;
            for (int i = 0; i < pActors.size(); ++i) {
                if (pActors[i].aiState == Dead ||
                    pActors[i].aiState == Removed ||
                    pActors[i].aiState == Disabled) {
                    ++num_dead_actors;
                } else {
                    ObjectType sumonner_type = pActors[i].summonerId.type();
                    if (sumonner_type == OBJECT_Character)
                        ++num_dead_actors;
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
    for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton; ++i) {
        GUIButton *pButton = pDialogueWindow->GetControl(i);
        if (!pButton)
            break;
        all_text_height += assets->pFontArrus->CalcTextHeight(pButton->sLabel, window.uFrameWidth, 0);
        index++;
    }

    if (index) {
        int v45 = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / index;
        if (v45 > SIDE_TEXT_BOX_MAX_SPACING)
            v45 = SIDE_TEXT_BOX_MAX_SPACING;
        int v42 = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - v45 * index - all_text_height) / 2 - v45 / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
            GUIButton *pButton = pDialogueWindow->GetControl(i);
            if (!pButton)
                break;
            pButton->uY = (unsigned int)(v45 + v42);
            pTextHeight = assets->pFontArrus->CalcTextHeight(pButton->sLabel, window.uFrameWidth, 0);
            pButton->uHeight = pTextHeight;
            v42 = pButton->uY + pTextHeight - 1;
            pButton->uW = v42;
            Color pTextColor = ui_game_dialogue_option_normal_color;
            if (pDialogueWindow->pCurrentPosActiveItem == i) {
                pTextColor = ui_game_dialogue_option_highlight_color;
            }
            window.DrawTitleText(assets->pFontArrus.get(), 0, pButton->uY, pTextColor, pButton->sLabel, 3);
        }
    }
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
}

void BuildHireableNpcDialogue() {
    pDialogueWindow->eWindowType = WINDOW_MainMenu;
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow_Dialogue(DIALOG_WINDOW_HIRE_FIRE_SHORT);
}

void selectNPCDialogueOption(DialogueId option) {
    NPCData *speakingNPC = getNPCData(speakingNpcId);

    ((GUIWindow_Dialogue*)pDialogueWindow)->setDisplayedDialogueType(option);

    if (!speakingNPC->uFlags) {
        speakingNPC->uFlags = NPC_GREETED_FIRST;
    }

    if (option >= DIALOGUE_SCRIPTED_LINE_1 && option <= DIALOGUE_SCRIPTED_LINE_6) {
        DialogueId newTopic = handleScriptedNPCTopicSelection(option, speakingNPC);

        if (newTopic != DIALOGUE_MAIN) {
            std::vector<DialogueId> topics = listNPCDialogueOptions(newTopic);
            ((GUIWindow_Dialogue*)pDialogueWindow)->setDisplayedDialogueType(newTopic);
            pDialogueWindow->DeleteButtons();
            pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {0xA9u, 0x23u}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
                                                            localization->GetString(LSTR_DIALOGUE_EXIT), {ui_exit_cancel_button_background});

            for (int i = 0; i < topics.size(); i++) {
                pDialogueWindow->CreateButton({480, 160 + i * 30}, {140, 30}, 1, 0, UIMSG_SelectNPCDialogueOption, std::to_underlying(topics[i]), Io::InputAction::Invalid, "");
            }
            pDialogueWindow->setKeyboardControlGroup(topics.size(), false, 0, 1);

            pDialogueWindow->CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1, "");
            pDialogueWindow->CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2, "");
            pDialogueWindow->CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3, "");
            pDialogueWindow->CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4, "");
            pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, Io::InputAction::CharCycle, "");
        }
        return;
    }

    if (option == DIALOGUE_13_hiring_related) {
        if (!speakingNPC->Hired()) {
            BuildHireableNpcDialogue();
            dialogue_show_profession_details = false;
        } else {
            for (unsigned i = 0; i < (signed int)pNPCStats->uNumNewNPCs; ++i) {
                if (pNPCStats->pNPCData[i].Hired() && speakingNPC->name == pNPCStats->pNPCData[i].name)
                    pNPCStats->pNPCData[i].uFlags &= ~NPC_HIRED;
            }
            if (ascii::noCaseEquals(pParty->pHirelings[0].name, speakingNPC->name)) // TODO(captainurist): #unicode this is not ascii
                pParty->pHirelings[0] = NPCData();
            else if (ascii::noCaseEquals(pParty->pHirelings[1].name, speakingNPC->name)) // TODO(captainurist): #unicode this is not ascii
                pParty->pHirelings[1] = NPCData();
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        }
        return;
    }

    selectSpecialNPCTopicSelection(option, speakingNPC);

    if (option == DIALOGUE_HIRE_FIRE) {
        if (speakingNPC->Hired()) {
            if (currentSpeakingActor && currentSpeakingActor->npcId >= 0) {
                currentSpeakingActor->aiState = Removed;
            }
        }
    }
}
