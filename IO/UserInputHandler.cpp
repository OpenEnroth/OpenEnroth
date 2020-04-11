#include "IO/UserInputHandler.h"

extern InputAction currently_selected_action_for_binding; // 506E68

static bool PartyDoTurnBasedAwareAction(PartyAction action) {
    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage == TE_WAIT ||
            pTurnEngine->turn_stage == TE_ATTACK ||
            pTurnEngine->uActionPointsLeft <= 0)
            return false;

        pTurnEngine->uActionPointsLeft -= 26;
    }
    pPartyActionQueue->Add(action);
    return true;
}

static bool PartyStrafe(PartyAction direction) {
    if (direction != PARTY_StrafeLeft && direction != PARTY_StrafeRight) {
        return false;
    }

    return PartyDoTurnBasedAwareAction(direction);
}

static bool PartyMove(PartyAction direction) {
    if (direction != PARTY_WalkForward && direction != PARTY_RunForward && direction != PARTY_WalkBackward && direction != PARTY_RunBackward) {
        return false;
    }

    return PartyDoTurnBasedAwareAction(direction);
}

//----- (0042FC4E) --------------------------------------------------------
void UserInputHandler::GenerateInputActions() {
    char v4;                  // al@9
    PartyAction partyAction;  // [sp-14h] [bp-1Ch]@20

    if (!engine->config->always_run) {
        if (IsRunKeyToggled())
            pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;
        else
            pParty->uFlags2 &= ~PARTY_FLAGS_2_RUNNING;
    }
    else {
        if (IsRunKeyToggled())
            pParty->uFlags2 &= ~PARTY_FLAGS_2_RUNNING;
        else
            pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;
    }

    if (pEventTimer->bPaused) {  // game paused
        for (auto action : AllInputActions()) {
            if (GetToggleType(action) == KeyToggleType::TOGGLE_OneTimePress)
                v4 = inputProvider->WasKeyPressed(GetKey(action));
            else
                v4 = inputProvider->IsKeyBeingHeld(GetKey(action));

            if (v4) {
                if (inputAction == InputAction::EventTrigger) {
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME || current_screen_type == CURRENT_SCREEN::SCREEN_CHEST) {
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Game_Action, 0, 0);
                        continue;
                    }
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE || current_screen_type == CURRENT_SCREEN::SCREEN_BRANCHLESS_NPC_DIALOG) {
                        /*v15 = pMessageQueue_50CBD0->uNumMessages;
                        if (pMessageQueue_50CBD0->uNumMessages) {
                            v15 = 0;
                            if (pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].field_8) {
                                v15 = 1;
                                pMessageQueue_50CBD0->uNumMessages = 0;
                                pMessageQueue_50CBD0->pMessages[v15].eType = UIMSG_Escape;
                                pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param = 0;
                                *(&pMessageQueue_50CBD0->uNumMessages + 3 * pMessageQueue_50CBD0->uNumMessages + 3) = 0;
                                ++pMessageQueue_50CBD0->uNumMessages;
                                continue;
                            }
                            pMessageQueue_50CBD0->uNumMessages = 0;

                        }*/
                        // pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
                    }
                }
            }
        }
    }
    if (!pEventTimer->bPaused) {  // game running
        for (uint i = 0; i < 30; ++i) {
            inputAction = (InputAction)i;
            if (GetToggleType(inputAction) == KeyToggleType::TOGGLE_OneTimePress)
                v4 = inputProvider->WasKeyPressed(GetKey(inputAction));
            else
                v4 = inputProvider->IsKeyBeingHeld(GetKey(inputAction));
            if (v4) {
                switch (inputAction) {
                case InputAction::MoveForward:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                    PartyMove(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_RunForward : PARTY_WalkForward);
                    break;

                case InputAction::MoveBackwards:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                    PartyMove(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_RunBackward : PARTY_WalkBackward);
                    break;

                case InputAction::StrafeLeft:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                    PartyStrafe(PARTY_StrafeLeft);
                    break;

                case InputAction::StrafeRight:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                    PartyStrafe(PARTY_StrafeRight);
                    break;

                case InputAction::TurnLeft:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                    if (IsTurnStrafingToggled()) {
                        if (!PartyStrafe(PARTY_StrafeLeft)) {
                            break;
                        }
                    }
                    else {
                        if (pParty->uFlags2 & PARTY_FLAGS_2_RUNNING)
                            partyAction = PARTY_FastTurnLeft;
                        else
                            partyAction = PARTY_TurnLeft;
                        pPartyActionQueue->Add(partyAction);
                    }

                    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor && pWeather->bRenderSnow)
                        pWeather->OnPlayerTurn(10);
                    break;
                case InputAction::TurnRight:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                    if (IsTurnStrafingToggled()) {
                        if (!PartyStrafe(PARTY_StrafeRight)) {
                            break;
                        }
                    }
                    else {
                        if (pParty->uFlags2 & PARTY_FLAGS_2_RUNNING)
                            partyAction = PARTY_FastTurnRight;
                        else
                            partyAction = PARTY_TurnRight;
                        pPartyActionQueue->Add(partyAction);
                    }

                    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor && pWeather->bRenderSnow)
                        pWeather->OnPlayerTurn(-10);
                    break;

                case InputAction::Jump:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME || pParty->bTurnBasedModeOn)
                        break;
                    partyAction = (PartyAction)12;
                    pPartyActionQueue->Add(partyAction);
                    break;
                case InputAction::Yell:
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME && uActiveCharacter) {
                        pParty->Yell();
                        pPlayers[uActiveCharacter]->PlaySound(SPEECH_Yell, 0);
                    }
                    break;
                case InputAction::Pass:
                    if (!(current_screen_type == CURRENT_SCREEN::SCREEN_GAME)) break;
                    if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                        pTurnEngine->field_18 |= TE_FLAG_8;
                        break;
                    }
                    if (uActiveCharacter) {
                        if (!pPlayers[uActiveCharacter]->uTimeToRecovery) {
                            if (!pParty->bTurnBasedModeOn)
                                pPlayers[uActiveCharacter]->SetRecoveryTime(
                                (signed __int64)(flt_6BE3A4_debug_recmod1 *
                                    (double)pPlayers[uActiveCharacter]->GetAttackRecoveryTime(false) *
                                    2.133333333333333));
                            CastSpellInfoHelpers::_427D48();
                            pTurnEngine->ApplyPlayerAction();
                        }
                    }
                    break;

                case InputAction::Combat:
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                        if (pParty->bTurnBasedModeOn) {
                            if (pTurnEngine->turn_stage == TE_MOVEMENT ||
                                PID_TYPE(pTurnEngine->pQueue[0].uPackedID) == OBJECT_Player) {
                                pParty->bTurnBasedModeOn = false;
                                pTurnEngine->End(true);
                            }
                        }
                        else {
                            pTurnEngine->Start();
                            pParty->bTurnBasedModeOn = true;
                        }
                    }
                    break;
                case InputAction::CastReady: {
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                    if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                        pTurnEngine->field_18 |= TE_FLAG_8;
                        break;
                    }
                    if (!uActiveCharacter) break;

                    uchar quickSpellNumber = pPlayers[uActiveCharacter]->uQuickSpell;
                    int skill_level = pPlayers[uActiveCharacter]->GetActualSkillMastery(PLAYER_SKILL_TYPE(quickSpellNumber / 11 + 12));

                    int uRequiredMana = 0;
                    if (!engine->config->debug_all_magic)
                        uRequiredMana = pSpellDatas[quickSpellNumber].mana_per_skill[skill_level - 1];

                    bool enoughMana = (pPlayers[uActiveCharacter]->sMana >= uRequiredMana);

                    if (!pPlayers[uActiveCharacter]->uQuickSpell ||
                        engine->IsUnderwater() || !enoughMana) {
                        pPartyActionQueue = pPartyActionQueue;
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Attack, 0, 0);
                        break;
                    }
                    else {
                        pMessageQueue_50C9E8->AddGUIMessage(UIMSG_CastQuickSpell, 0, 0);
                    }
                } break;
                case InputAction::Attack:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                    if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                        pTurnEngine->field_18 |= TE_FLAG_8;
                        break;
                    }
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Attack, 0, 0);
                    break;
                case InputAction::EventTrigger:
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Game_Action, 0, 0);
                        break;
                    }
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE) {
                        pMessageQueue_50CBD0->Clear();
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
                    }
                    break;
                case InputAction::CharCycle:
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_SPELL_BOOK) break;

                    pMessageQueue_50C9E8->AddGUIMessage(UIMSG_CycleCharacters, 0, 0);
                    break;
                case InputAction::LookUp:
                    if (pEventTimer->bPaused) break;
                    partyAction = (PartyAction)7;
                    pPartyActionQueue->Add(partyAction);
                    break;
                case InputAction::CenterView:
                    if (pEventTimer->bPaused) break;
                    partyAction = (PartyAction)9;
                    pPartyActionQueue->Add(partyAction);
                    break;
                case InputAction::LookDown:
                    if (pEventTimer->bPaused) break;
                    partyAction = (PartyAction)8;
                    pPartyActionQueue->Add(partyAction);
                    break;
                case InputAction::FlyUp:
                    if (!(current_screen_type == CURRENT_SCREEN::SCREEN_GAME) || pEventTimer->bPaused) break;
                    partyAction = (PartyAction)13;
                    pPartyActionQueue->Add(partyAction);
                    break;
                case InputAction::Land:
                    if (!(current_screen_type == CURRENT_SCREEN::SCREEN_GAME) || pEventTimer->bPaused) break;
                    partyAction = (PartyAction)15;
                    pPartyActionQueue->Add(partyAction);
                    break;
                case InputAction::FlyDown:
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME && !pEventTimer->bPaused) {
                        partyAction = (PartyAction)14;
                        pPartyActionQueue->Add(partyAction);
                    }
                    break;
                case InputAction::ZoomIn:
                    pMessageQueue_50C9E8->AddGUIMessage(UIMSG_ClickZoomOutBtn, 0, 0);
                    break;
                case InputAction::ZoomOut:
                    pMessageQueue_50C9E8->AddGUIMessage(UIMSG_ClickZoomInBtn, 0, 0);
                    break;
                case InputAction::AlwaysRun:
                    engine->ToggleAlwaysRun();
                    break;
                default:
                    break;
                }
            }
        }
    }
}



//----- (0045B06E) --------------------------------------------------------
bool Keyboard::IsShiftHeld() const {
    return (GetAsyncKeyState(VK_SHIFT) & 0x8001) != 0;
}

//----- (0045B0A9) --------------------------------------------------------
bool Keyboard::IsKeyBeingHeld(int vKey) {
    return (GetAsyncKeyState(vKey) & 0x8001) != 0;
}

//----- (0045B0CE) --------------------------------------------------------
bool Keyboard::WasKeyPressed(int vKey) {
    return (GetAsyncKeyState(vKey) & 1) != 0;
}

//----- (00459E5A) --------------------------------------------------------
void UserInputHandler::StartTextInput(TextInputType type, int max_string_len, GUIWindow* pWindow) {
    memset(pPressedKeysBuffer, 0, 0x101u);
    uNumKeysPressed = 0;
    inputType = type;

    this->max_input_string_len = max_string_len;
    this->pWindow = pWindow;

    pWindow->receives_keyboard_input_2 = WINDOW_INPUT_IN_PROGRESS;
}

//----- (00459ED1) --------------------------------------------------------
void UserInputHandler::SetWindowInputStatus(int a2) {
    inputType = TextInputType::None;
    if (pWindow) pWindow->receives_keyboard_input_2 = a2;
}

//----- (00459F10) --------------------------------------------------------
bool UserInputHandler::ProcessTextInput(GameKey key, int c) {
    uLastKeyPressed = c;
    if (currently_selected_action_for_binding == InputAction::Invalid) {
        if (inputType != TextInputType::Text && inputType != TextInputType::Number) {
            return false;
        }

        if (key == GameKey::Backspace) {
            if (uNumKeysPressed > 0) {
                pPressedKeysBuffer[--uNumKeysPressed] = 0;
            }
        } else if (key == GameKey::Return) {
            SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
        } else if (key == GameKey::Escape) {
            SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
        } else if (key == GameKey::Char && this->uNumKeysPressed < this->max_input_string_len) {
            if (inputType == TextInputType::Text) {
                pPressedKeysBuffer[uNumKeysPressed++] = c;
                pPressedKeysBuffer[uNumKeysPressed] = 0;
            } else if (inputType == TextInputType::Number && isdigit(c)) {
                pPressedKeysBuffer[uNumKeysPressed++] = c;
            }
        }
    } else {
        // we're setting key binding in options
        pPressedKeysBuffer[uNumKeysPressed++] = c;
        pPressedKeysBuffer[uNumKeysPressed] = 0;
        SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
    }
    return true;
}


std::string UserInputHandler::GetTextInput() const {
    return std::string(pPressedKeysBuffer);
}

void UserInputHandler::SetTextInput(const std::string& text) {
    SetTextInput(text.c_str());
}

void UserInputHandler::SetTextInput(const char* text) {
    strcpy(pPressedKeysBuffer, text);
    uNumKeysPressed = strlen(pPressedKeysBuffer);
}

//----- (00459E3F) --------------------------------------------------------
void UserInputHandler::ResetKeys() {
    for (uint i = 0; i < 30; ++i) {
        GetAsyncKeyState(pVirtualKeyCodesMapping[i]);
    }
}

bool UserInputHandler::IsRunKeyToggled() const {
    return inputProvider->IsShiftHeld();
}

bool UserInputHandler::IsTurnStrafingToggled() const {
    return inputProvider->IsKeyBeingHeld(GameKey::Control);
}

bool UserInputHandler::IsKeyboardPickingOutlineToggled() const {
    return inputProvider->IsKeyBeingHeld(GameKey::Control);
}