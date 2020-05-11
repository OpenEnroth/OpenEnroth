#include "Io/KeyboardInputHandler.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"

using Io::InputAction;
using Io::KeyboardInputHandler;

std::shared_ptr<KeyboardInputHandler> keyboardInputHandler = nullptr;

extern InputAction currently_selected_action_for_binding;  // 506E68

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


void KeyboardInputHandler::GeneratePausedActions() {
    for (auto action : AllInputActions()) {
        bool isTriggered = false;
        GameKey key = actionMapping->GetKey(action);
        if (GetToggleType(action) == KeyToggleType::TOGGLE_OneTimePress)
            isTriggered = controller->IsKeyPressed(key);
        else
            isTriggered = controller->IsKeyHeld(key);

        if (!isTriggered) {
            continue;
        }

        if (action == InputAction::EventTrigger) {
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

void KeyboardInputHandler::GenerateGameplayActions() {
    for (InputAction action : AllInputActions()) {
        bool isTriggered = false;
        GameKey key = actionMapping->GetKey(action);
        if (GetToggleType(action) == KeyToggleType::TOGGLE_OneTimePress)
            isTriggered = controller->IsKeyPressed(key);
        else
            isTriggered = controller->IsKeyHeld(key);

        if (!isTriggered) {
            continue;
        }

        switch (action) {
        case InputAction::MoveForward:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                PartyMove(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_RunForward : PARTY_WalkForward);
            }
            break;

        case InputAction::MoveBackwards:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                PartyMove(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_RunBackward : PARTY_WalkBackward);
            }
            break;

        case InputAction::StrafeLeft:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                PartyStrafe(PARTY_StrafeLeft);
            }
            break;

        case InputAction::StrafeRight:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                PartyStrafe(PARTY_StrafeRight);
            }
            break;

        case InputAction::TurnLeft:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                if (IsTurnStrafingToggled()) {
                    if (!PartyStrafe(PARTY_StrafeLeft)) {
                        break;
                    }
                } else {
                    pPartyActionQueue->Add(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_FastTurnLeft : PARTY_TurnLeft);
                }

                if (uCurrentlyLoadedLevelType == LEVEL_Outdoor && pWeather->bRenderSnow)
                    pWeather->OnPlayerTurn(10);
            }
            break;
        case InputAction::TurnRight:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                if (IsTurnStrafingToggled()) {
                    if (!PartyStrafe(PARTY_StrafeRight)) {
                        break;
                    }
                } else {
                    pPartyActionQueue->Add(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_FastTurnRight : PARTY_TurnRight);
                }

                if (uCurrentlyLoadedLevelType == LEVEL_Outdoor && pWeather->bRenderSnow)
                    pWeather->OnPlayerTurn(-10);
            }
            break;

        case InputAction::Jump:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME && !pParty->bTurnBasedModeOn) {
                pPartyActionQueue->Add(PARTY_Jump);
            }
            break;

        case InputAction::Yell:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME && uActiveCharacter != 0) {
                pParty->Yell();
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_Yell, 0);
            }
            break;

        case InputAction::Pass:
            if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->field_18 |= TE_FLAG_8_finished;
                break;
            }
            if (uActiveCharacter != 0) {
                if (pPlayers[uActiveCharacter]->uTimeToRecovery == 0) {
                    if (!pParty->bTurnBasedModeOn) {
                        pPlayers[uActiveCharacter]->SetRecoveryTime(
                            flt_6BE3A4_debug_recmod1 * (double)pPlayers[uActiveCharacter]->GetAttackRecoveryTime(false) * 2.133333333333333
                        );
                    }
                    CastSpellInfoHelpers::Cancel_Spell_Cast_In_Progress();
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
                } else {
                    pTurnEngine->Start();
                    pParty->bTurnBasedModeOn = true;
                }
            }
            break;

        case InputAction::CastReady: {
            if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) {
                break;
            }

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->field_18 |= TE_FLAG_8_finished;
                break;
            }

            if (uActiveCharacter == 0) {
                break;
            }

            uchar quickSpellNumber = pPlayers[uActiveCharacter]->uQuickSpell;
            int skill_level = pPlayers[uActiveCharacter]->GetActualSkillMastery(PLAYER_SKILL_TYPE(quickSpellNumber / 11 + 12));

            int uRequiredMana = 0;
            if (!engine->config->debug_all_magic) {
                uRequiredMana = pSpellDatas[quickSpellNumber].mana_per_skill[skill_level - 1];
            }

            bool enoughMana = pPlayers[uActiveCharacter]->sMana >= uRequiredMana;

            if (pPlayers[uActiveCharacter]->uQuickSpell == 0 || engine->IsUnderwater() || !enoughMana) {
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Attack, 0, 0);
            } else {
                pMessageQueue_50C9E8->AddGUIMessage(UIMSG_CastQuickSpell, 0, 0);
            }

            break;
        }

        case InputAction::Attack:
            if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) {
                break;
            }

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->field_18 |= TE_FLAG_8_finished;
            } else {
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Attack, 0, 0);
            }

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
            if (current_screen_type != CURRENT_SCREEN::SCREEN_SPELL_BOOK) {
                pMessageQueue_50C9E8->AddGUIMessage(UIMSG_CycleCharacters, 0, 0);
            }
            break;

        case InputAction::LookUp:
            pPartyActionQueue->Add(PARTY_LookUp);
            break;

        case InputAction::CenterView:
            pPartyActionQueue->Add(PARTY_CenterView);
            break;

        case InputAction::LookDown:
            pPartyActionQueue->Add(PARTY_LookDown);
            break;

        case InputAction::FlyUp:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                pPartyActionQueue->Add(PARTY_FlyUp);
            }
            break;

        case InputAction::Land:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                pPartyActionQueue->Add(PARTY_Land);
            }
            break;

        case InputAction::FlyDown:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                pPartyActionQueue->Add(PARTY_FlyDown);
            }
            break;

        case InputAction::ZoomIn:
            // pMessageQueue_50C9E8->AddGUIMessage(UIMSG_ClickZoomInBtn, 0, 0);
            break;

        case InputAction::ZoomOut:
            // pMessageQueue_50C9E8->AddGUIMessage(UIMSG_ClickZoomOutBtn, 0, 0);
            break;

        case InputAction::AlwaysRun:
            engine->ToggleAlwaysRun();
            break;

        default:
            break;
        }
    }
}

//----- (0042FC4E) --------------------------------------------------------
void KeyboardInputHandler::GenerateInputActions() {
    if (!engine->config->always_run) {
        if (IsRunKeyToggled()) {
            pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;
        } else {
            pParty->uFlags2 &= ~PARTY_FLAGS_2_RUNNING;
        }
    } else {
        if (IsRunKeyToggled()) {
            pParty->uFlags2 &= ~PARTY_FLAGS_2_RUNNING;
        } else {
            pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;
        }
    }

    if (pEventTimer->bPaused) {
        GeneratePausedActions();
    } else {
        GenerateGameplayActions();
    }
}

//----- (00459E5A) --------------------------------------------------------
void KeyboardInputHandler::StartTextInput(TextInputType type, int max_string_len, GUIWindow* window) {
    memset(pPressedKeysBuffer, 0, 0x101u);
    uNumKeysPressed = 0;
    inputType = type;

    this->max_input_string_len = max_string_len;
    this->window = window;

    if (window != nullptr) {
        window->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_IN_PROGRESS;
    }
}

void KeyboardInputHandler::EndTextInput() {
    if (window != nullptr) {
        window->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
    }
}

//----- (00459ED1) --------------------------------------------------------
void KeyboardInputHandler::SetWindowInputStatus(WindowInputStatus status) {
    inputType = TextInputType::None;
    if (window) {
        window->keyboard_input_status = status;
    }
}

//----- (00459F10) --------------------------------------------------------
bool KeyboardInputHandler::ProcessTextInput(GameKey key, int c) {
    if (currently_selected_action_for_binding == InputAction::Invalid) {
        if (inputType != TextInputType::Text && inputType != TextInputType::Number) {
            return false;
        }

        if (key == GameKey::Backspace) {
            if (uNumKeysPressed > 0) {
                pPressedKeysBuffer[--uNumKeysPressed] = 0;
            }
        } else if (key == GameKey::Return) {
            SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_CONFIRMED);
        } else if (key == GameKey::Escape) {
            SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_CANCELLED);
        } else if (key == GameKey::Char && this->uNumKeysPressed < this->max_input_string_len) {
            if (inputType == TextInputType::Text) {
                pPressedKeysBuffer[uNumKeysPressed++] = c;
                pPressedKeysBuffer[uNumKeysPressed] = 0;
            } else if (inputType == TextInputType::Number && isdigit(c)) {
                pPressedKeysBuffer[uNumKeysPressed++] = c;
            }
        }
    } else {
        if (key != GameKey::Char) {
            // we're setting key binding in options
            // pPressedKeysBuffer[uNumKeysPressed++] = c;
            // pPressedKeysBuffer[uNumKeysPressed] = 0;
            lastKeyPressed = key;
            SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_CONFIRMED);
        }
    }
    return true;
}


std::string KeyboardInputHandler::GetTextInput() const {
    return std::string(pPressedKeysBuffer);
}

void KeyboardInputHandler::SetTextInput(const std::string& text) {
    SetTextInput(text.c_str());
}

void KeyboardInputHandler::SetTextInput(const char* text) {
    strcpy(pPressedKeysBuffer, text);
    uNumKeysPressed = strlen(pPressedKeysBuffer);
}

//----- (00459E3F) --------------------------------------------------------
void KeyboardInputHandler::ResetKeys() {
    for (auto action : AllInputActions()) {
        // requesting KeyPressed will consume all the events due to how logic is designed in GetAsyncKeyState
        controller->IsKeyPressed(actionMapping->GetKey(action));
    }
}

bool KeyboardInputHandler::IsRunKeyToggled() const {
    return controller->IsKeyHeld(GameKey::Shift);
}

bool KeyboardInputHandler::IsTurnStrafingToggled() const {
    return controller->IsKeyHeld(GameKey::Control);
}

bool KeyboardInputHandler::IsKeyboardPickingOutlineToggled() const {
    return controller->IsKeyHeld(GameKey::Control);
}

bool KeyboardInputHandler::IsStealingToggled() const {
    return controller->IsKeyHeld(GameKey::Control);
}

bool KeyboardInputHandler::IsTakeAllToggled() const {
    return controller->IsKeyHeld(GameKey::Control);
}

bool KeyboardInputHandler::IsAdventurerBackcycleToggled() const {
    return controller->IsKeyHeld(GameKey::Shift);
}

bool KeyboardInputHandler::IsSpellBackcycleToggled() const {
    return controller->IsKeyHeld(GameKey::Shift);
}

bool KeyboardInputHandler::IsCastOnClickToggled() const {
    return controller->IsKeyHeld(GameKey::Shift);
}
