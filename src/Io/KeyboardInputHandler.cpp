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
        PlatformKey key = actionMapping->GetKey(action);
        if (GetToggleType(action) == KeyToggleType::TOGGLE_OneTimePress)
            isTriggered = controller->ConsumeKeyPress(key);
        else
            isTriggered = controller->IsKeyDown(key);

        if (!isTriggered) {
            continue;
        }

        if (action == InputAction::EventTrigger) {
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME || current_screen_type == CURRENT_SCREEN::SCREEN_CHEST) {
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Game_Action, 0, 0);
                continue;
            }
            if (current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE || current_screen_type == CURRENT_SCREEN::SCREEN_BRANCHLESS_NPC_DIALOG) {
                /*v15 = pCurrentFrameMessageQueue->uNumMessages;
                if (pCurrentFrameMessageQueue->uNumMessages) {
                    v15 = 0;
                    if (pCurrentFrameMessageQueue->pMessages[pCurrentFrameMessageQueue->uNumMessages].field_8) {
                        v15 = 1;
                        pCurrentFrameMessageQueue->uNumMessages = 0;
                        pCurrentFrameMessageQueue->pMessages[v15].eType = UIMSG_Escape;
                        pCurrentFrameMessageQueue->pMessages[pCurrentFrameMessageQueue->uNumMessages].param = 0;
                        *(&pCurrentFrameMessageQueue->uNumMessages + 3 * pCurrentFrameMessageQueue->uNumMessages + 3) = 0;
                        ++pCurrentFrameMessageQueue->uNumMessages;
                        continue;
                    }
                    pCurrentFrameMessageQueue->uNumMessages = 0;

                }*/
                // pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
            }
        }
    }
}

void KeyboardInputHandler::GenerateGameplayActions() {
    // delay press timer
    bool resettimer = true;
    for (InputAction action : AllInputActions()) {
        bool isTriggered = false;
        PlatformKey key = actionMapping->GetKey(action);
        PlatformKey gamepadkey = actionMapping->GetGamepadKey(action);
        if (GetToggleType(action) == KeyToggleType::TOGGLE_OneTimePress) {
            isTriggered = controller->ConsumeKeyPress(key) || controller->ConsumeKeyPress(gamepadkey);
        } else if (GetToggleType(action) == KeyToggleType::TOGGLE_Continuously) {
            isTriggered = controller->IsKeyDown(key) || controller->IsKeyDown(gamepadkey);
        } else {
            // delay press
            if (controller->IsKeyDown(key) || controller->IsKeyDown(gamepadkey)) {
                resettimer = false;
                if (this->keydelaytimer == 0) {
                    isTriggered = true;
                    this->keydelaytimer++;
                }
                // big delay after first press then small delay
                if (this->keydelaytimer >= DELAY_TOGGLE_TIME_FIRST) {
                    isTriggered = true;
                    this->keydelaytimer -= DELAY_TOGGLE_TIME_AFTER;
                }
            }
        }

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
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME && pParty->hasActiveCharacter()) {
                pParty->yell();
                pParty->activeCharacter().playReaction(SPEECH_Yell);
            }
            break;

        case InputAction::Pass:
            if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->flags |= TE_FLAG_8_finished;
                break;
            }
            if (pParty->hasActiveCharacter()) {
                if (pParty->activeCharacter().timeToRecovery == 0) {
                    if (!pParty->bTurnBasedModeOn) {
                        pParty->activeCharacter().SetRecoveryTime(
                            debug_non_combat_recovery_mul * (double)pParty->activeCharacter().GetAttackRecoveryTime(false) * flt_debugrecmod3
                        );
                    }
                    CastSpellInfoHelpers::cancelSpellCastInProgress();
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
                pTurnEngine->flags |= TE_FLAG_8_finished;
                break;
            }

            if (!pParty->hasActiveCharacter()) {
                break;
            }

            SPELL_TYPE quickSpellNumber = pParty->activeCharacter().uQuickSpell;

            int uRequiredMana = 0;
            if (quickSpellNumber != SPELL_NONE && !engine->config->debug.AllMagic.value()) {
                PLAYER_SKILL_MASTERY skill_mastery = pParty->activeCharacter().GetActualSkillMastery(getSkillTypeForSpell(quickSpellNumber));

                uRequiredMana = pSpellDatas[quickSpellNumber].mana_per_skill[std::to_underlying(skill_mastery) - 1];
            }

            bool enoughMana = pParty->activeCharacter().mana >= uRequiredMana;

            if (quickSpellNumber == SPELL_NONE || engine->IsUnderwater() || !enoughMana) {
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Attack, 0, 0);
            } else {
                // TODO(Nik-RE-dev): why next frame?
                pNextFrameMessageQueue->AddGUIMessage(UIMSG_CastQuickSpell, 0, 0);
            }

            break;
        }

        case InputAction::Attack:
            if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) {
                break;
            }

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->flags |= TE_FLAG_8_finished;
            } else {
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Attack, 0, 0);
            }

            break;

        case InputAction::EventTrigger:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Game_Action, 0, 0);
                break;
            }

            if (current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE) {
                pCurrentFrameMessageQueue->Clear();
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
            }
            break;

        case InputAction::CharCycle:
            if (current_screen_type != CURRENT_SCREEN::SCREEN_SPELL_BOOK) {
                // TODO(Nik-RE-dev): why next frame?
                pNextFrameMessageQueue->AddGUIMessage(UIMSG_CycleCharacters, 0, 0);
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
            // pNextFrameMessageQueue->AddGUIMessage(UIMSG_ClickZoomInBtn, 0, 0);
            break;

        case InputAction::ZoomOut:
            // pNextFrameMessageQueue->AddGUIMessage(UIMSG_ClickZoomOutBtn, 0, 0);
            break;

        case InputAction::AlwaysRun:
            engine->config->settings.AlwaysRun.toggle();
            break;

        case InputAction::Escape:
            // pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, window_SpeakInHouse != 0, 0);
            break;

        case InputAction::Inventory:
            if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                pNextFrameMessageQueue->AddGUIMessage(UIMSG_OpenInventory, 0, 0);
            }
            break;

        default:
            break;
        }
    }

    if (resettimer == true) {
        this->keydelaytimer = 0;
    } else {
        // use timer so pacing is consistent across framerates
        if (this->keydelaytimer < DELAY_TOGGLE_TIME_FIRST) this->keydelaytimer += pEventTimer->uTimeElapsed;
    }
}

//----- (0042FC4E) --------------------------------------------------------
void KeyboardInputHandler::GenerateInputActions() {
    if (!engine->config->settings.AlwaysRun.value()) {
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
void KeyboardInputHandler::StartTextInput(TextInputType type, int max_string_len, GUIWindow *window) {
    memset(pPressedKeysBuffer, 0, 0x101u);
    uNumKeysPressed = 0;
    inputType = type;

    this->max_input_string_len = max_string_len;
    this->window = window;

    if (window != nullptr) {
        window->keyboard_input_status = WINDOW_INPUT_IN_PROGRESS;
    }
}

void KeyboardInputHandler::EndTextInput() {
    if (window != nullptr) {
        window->keyboard_input_status = WINDOW_INPUT_NONE;
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
bool KeyboardInputHandler::ProcessTextInput(PlatformKey key, int c) {
    if (currently_selected_action_for_binding == InputAction::Invalid) {
        if (inputType != TextInputType::Text && inputType != TextInputType::Number) {
            return false;
        }

        if (key == PlatformKey::Backspace) {
            if (uNumKeysPressed > 0) {
                pPressedKeysBuffer[--uNumKeysPressed] = 0;
            }
        } else if (key == PlatformKey::Return) {
            SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
        } else if (key == PlatformKey::Escape) {
            SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
        } else if (key == PlatformKey::Space && this->uNumKeysPressed < this->max_input_string_len) {
            if (inputType == TextInputType::Text) {
                pPressedKeysBuffer[uNumKeysPressed++] = ' ';
                pPressedKeysBuffer[uNumKeysPressed] = 0;
            }
        } else if (key == PlatformKey::Char && this->uNumKeysPressed < this->max_input_string_len) {
            if (inputType == TextInputType::Text) {
                pPressedKeysBuffer[uNumKeysPressed++] = c;
                pPressedKeysBuffer[uNumKeysPressed] = 0;
            } else if (inputType == TextInputType::Number && isdigit(c)) {
                pPressedKeysBuffer[uNumKeysPressed++] = c;
            }
        }
    } else {
        if (key != PlatformKey::Char) {
            // we're setting key binding in options
            // pPressedKeysBuffer[uNumKeysPressed++] = c;
            // pPressedKeysBuffer[uNumKeysPressed] = 0;
            lastKeyPressed = key;
            SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
        }
    }
    return true;
}


std::string KeyboardInputHandler::GetTextInput() const {
    return std::string(pPressedKeysBuffer);
}

void KeyboardInputHandler::SetTextInput(const std::string &text) {
    SetTextInput(text.c_str());
}

void KeyboardInputHandler::SetTextInput(const char *text) {
    strcpy(pPressedKeysBuffer, text);
    uNumKeysPressed = strlen(pPressedKeysBuffer);
}

//----- (00459E3F) --------------------------------------------------------
void KeyboardInputHandler::ResetKeys() {
    for (auto action : AllInputActions()) {
        // requesting KeyPressed will consume all the events due to how logic is designed in GetAsyncKeyState
        controller->ConsumeKeyPress(actionMapping->GetKey(action));
    }
}

bool KeyboardInputHandler::IsRunKeyToggled() const {
    return controller->IsKeyDown(PlatformKey::Shift);
}

bool KeyboardInputHandler::IsTurnStrafingToggled() const {
    return controller->IsKeyDown(PlatformKey::Control);
}

bool KeyboardInputHandler::IsKeyboardPickingOutlineToggled() const {
    return controller->IsKeyDown(PlatformKey::Control);
}

bool KeyboardInputHandler::IsStealingToggled() const {
    return controller->IsKeyDown(PlatformKey::Control);
}

bool KeyboardInputHandler::IsTakeAllToggled() const {
    return controller->IsKeyDown(PlatformKey::Control);
}

bool KeyboardInputHandler::IsAdventurerBackcycleToggled() const {
    return controller->IsKeyDown(PlatformKey::Shift);
}

bool KeyboardInputHandler::IsSpellBackcycleToggled() const {
    return controller->IsKeyDown(PlatformKey::Shift);
}

bool KeyboardInputHandler::IsCastOnClickToggled() const {
    return controller->IsKeyDown(PlatformKey::Shift);
}

bool KeyboardInputHandler::IsKeyHeld(PlatformKey key) const {
    return controller->IsKeyDown(key);
}
