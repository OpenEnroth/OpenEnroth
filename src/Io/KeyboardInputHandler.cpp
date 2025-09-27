#include "KeyboardInputHandler.h"

#include <memory>
#include <string>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Party.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Spells/Spells.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Engine.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"

#include "InputEnumFunctions.h"

// Delayed keyrepeat registers after 500ms.
static constexpr Duration DELAY_TOGGLE_TIME_FIRST = Duration::fromRealtimeMilliseconds(500);

// Further keyrepeats happen every 1/15th of a second.
static constexpr Duration DELAY_TOGGLE_TIME_PERIOD = Duration::fromRealtimeMilliseconds(67);

std::shared_ptr<Io::KeyboardInputHandler> keyboardInputHandler = nullptr;

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


void Io::KeyboardInputHandler::GeneratePausedActions() {
    for (auto action : allInputActions()) {
        bool isTriggered = false;
        PlatformKey key = actionMapping->keyFor(action);
        if (toggleTypeForInputAction(action) == TOGGLE_ONCE)
            isTriggered = controller->ConsumeKeyPress(key);
        else
            isTriggered = controller->IsKeyDown(key);

        if (!isTriggered) {
            continue;
        }

        if (action == INPUT_ACTION_INTERACT) {
            if (current_screen_type == SCREEN_GAME || current_screen_type == SCREEN_CHEST) {
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Game_Action, 0, 0);
                continue;
            }
            if (current_screen_type == SCREEN_NPC_DIALOGUE || current_screen_type == SCREEN_BRANCHLESS_NPC_DIALOG) {
                // engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
            }
        }
    }
}

void Io::KeyboardInputHandler::GenerateGameplayActions() {
    // delay press timer
    bool resettimer = true;
    for (InputAction action : allInputActions()) {
        bool isTriggered = false;
        PlatformKey key = actionMapping->keyFor(action);
        PlatformKey gamepadkey = actionMapping->gamepadKeyFor(action);

        switch (toggleTypeForInputAction(action)) {
        default: assert(false); [[fallthrough]];
        case TOGGLE_ONCE:
            isTriggered = controller->ConsumeKeyPress(key) || controller->ConsumeKeyPress(gamepadkey);
            break;
        case TOGGLE_CONTINUOUSLY:
            isTriggered = controller->IsKeyDown(key) || controller->IsKeyDown(gamepadkey);
            break;
        case TOGGLE_CONTINUOUSLY_WITH_DELAY:
            // TODO(captainurist): This logic breaks down if we press & release a key every frame.
            //                     Better way to implement this would be to generate the input actions from inside
            //                     the event handler.
            if (controller->IsKeyDown(key) || controller->IsKeyDown(gamepadkey)) {
                resettimer = false;
                if (!this->keydelaytimer) {
                    isTriggered = true;
                }
                // big delay after first press then small delay
                if (this->keydelaytimer >= DELAY_TOGGLE_TIME_FIRST) {
                    isTriggered = true;
                    this->keydelaytimer -= DELAY_TOGGLE_TIME_PERIOD;
                }
            }
            break;
        }

        if (!isTriggered) {
            continue;
        }

        switch (action) {
        case INPUT_ACTION_MOVE_FORWARD:
            if (current_screen_type == SCREEN_GAME) {
                PartyMove(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_RunForward : PARTY_WalkForward);
            }
            break;

        case INPUT_ACTION_MOVE_BACKWARDS:
            if (current_screen_type == SCREEN_GAME) {
                PartyMove(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_RunBackward : PARTY_WalkBackward);
            }
            break;

        case INPUT_ACTION_STRAFE_LEFT:
            if (current_screen_type == SCREEN_GAME) {
                PartyStrafe(PARTY_StrafeLeft);
            }
            break;

        case INPUT_ACTION_STRAFE_RIGHT:
            if (current_screen_type == SCREEN_GAME) {
                PartyStrafe(PARTY_StrafeRight);
            }
            break;

        case INPUT_ACTION_TURN_LEFT:
            if (current_screen_type == SCREEN_GAME) {
                if (IsTurnStrafingToggled()) {
                    if (!PartyStrafe(PARTY_StrafeLeft)) {
                        break;
                    }
                } else {
                    pPartyActionQueue->Add(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_FastTurnLeft : PARTY_TurnLeft);
                }

                if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR && pWeather->bRenderSnow)
                    pWeather->OnPlayerTurn(10);
            }
            break;
        case INPUT_ACTION_TURN_RIGHT:
            if (current_screen_type == SCREEN_GAME) {
                if (IsTurnStrafingToggled()) {
                    if (!PartyStrafe(PARTY_StrafeRight)) {
                        break;
                    }
                } else {
                    pPartyActionQueue->Add(pParty->uFlags2 & PARTY_FLAGS_2_RUNNING ? PARTY_FastTurnRight : PARTY_TurnRight);
                }

                if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR && pWeather->bRenderSnow)
                    pWeather->OnPlayerTurn(-10);
            }
            break;

        case INPUT_ACTION_JUMP:
            if (current_screen_type == SCREEN_GAME && !pParty->bTurnBasedModeOn) {
                pPartyActionQueue->Add(PARTY_Jump);
            }
            break;

        case INPUT_ACTION_YELL:
            if (current_screen_type == SCREEN_GAME && pParty->hasActiveCharacter()) {
                pParty->yell();
                pParty->activeCharacter().playReaction(SPEECH_YELL);
            }
            break;

        case INPUT_ACTION_PASS:
            if (current_screen_type != SCREEN_GAME) break;

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->flags |= TE_FLAG_8_finished;
                break;
            }
            if (pParty->hasActiveCharacter()) {
                if (!pParty->activeCharacter().timeToRecovery) {
                    if (!pParty->bTurnBasedModeOn) {
                        pParty->activeCharacter().SetRecoveryTime(
                            debug_non_combat_recovery_mul * flt_debugrecmod3 * pParty->activeCharacter().GetAttackRecoveryTime(false)
                        );
                    }
                    CastSpellInfoHelpers::cancelSpellCastInProgress();
                    pTurnEngine->ApplyPlayerAction();
                }
            }
            break;

        case INPUT_ACTION_TOGGLE_TURN_BASED:
            if (current_screen_type == SCREEN_GAME) {
                if (pParty->bTurnBasedModeOn) {
                    if (pTurnEngine->turn_stage == TE_MOVEMENT ||
                        pTurnEngine->pQueue[0].uPackedID.type() == OBJECT_Character) {
                        pParty->bTurnBasedModeOn = false;
                        pTurnEngine->End(true);
                    }
                } else {
                    pTurnEngine->Start();
                    pParty->bTurnBasedModeOn = true;
                }
            }
            break;

        case INPUT_ACTION_QUICK_CAST: {
            if (current_screen_type != SCREEN_GAME) {
                break;
            }

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->flags |= TE_FLAG_8_finished;
                break;
            }

            if (!pParty->hasActiveCharacter()) {
                break;
            }

            SpellId quickSpellNumber = pParty->activeCharacter().uQuickSpell;
            int uRequiredMana = 0;

            if (!engine->config->debug.AllMagic.value()) {
                if (quickSpellNumber != SPELL_NONE && !pParty->activeCharacter().bHaveSpell[quickSpellNumber])
                    quickSpellNumber = SPELL_NONE; // Can end up here after setting the quick spell in all magic mode.

                if (quickSpellNumber != SPELL_NONE) {
                    Mastery skill_mastery = pParty->activeCharacter().getActualSkillValue(skillForSpell(quickSpellNumber)).mastery();
                    uRequiredMana = pSpellDatas[quickSpellNumber].mana_per_skill[skill_mastery];
                }
            }

            bool enoughMana = pParty->activeCharacter().mana >= uRequiredMana;

            if (quickSpellNumber == SPELL_NONE || engine->IsUnderwater() || !enoughMana) {
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Attack, 0, 0);
            } else {
                // TODO(Nik-RE-dev): why next frame?
                engine->_messageQueue->addMessageNextFrame(UIMSG_CastQuickSpell, 0, 0);
            }

            break;
        }

        case INPUT_ACTION_ATTACK:
            if (current_screen_type != SCREEN_GAME) {
                break;
            }

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->flags |= TE_FLAG_8_finished;
            } else {
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Attack, 0, 0);
            }

            break;

        case INPUT_ACTION_INTERACT:
            if (current_screen_type == SCREEN_GAME) {
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Game_Action, 0, 0);
                break;
            }

            if (current_screen_type == SCREEN_NPC_DIALOGUE) {
                engine->_messageQueue->clear();
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
            }
            break;

        case INPUT_ACTION_NEXT_CHAR:
            if (current_screen_type != SCREEN_SPELL_BOOK) {
                // TODO(Nik-RE-dev): why next frame?
                engine->_messageQueue->addMessageNextFrame(UIMSG_CycleCharacters, 0, 0);
            }
            break;

        case INPUT_ACTION_LOOK_UP:
            pPartyActionQueue->Add(PARTY_LookUp);
            break;

        case INPUT_ACTION_CENTER_VIEW:
            pPartyActionQueue->Add(PARTY_CenterView);
            break;

        case INPUT_ACTION_LOOK_DOWN:
            pPartyActionQueue->Add(PARTY_LookDown);
            break;

        case INPUT_ACTION_FLY_UP:
            if (current_screen_type == SCREEN_GAME) {
                pPartyActionQueue->Add(PARTY_FlyUp);
            }
            break;

        case INPUT_ACTION_FLY_LAND:
            if (current_screen_type == SCREEN_GAME) {
                pPartyActionQueue->Add(PARTY_Land);
            }
            break;

        case INPUT_ACTION_FLY_DOWN:
            if (current_screen_type == SCREEN_GAME) {
                pPartyActionQueue->Add(PARTY_FlyDown);
            }
            break;

        case INPUT_ACTION_ZOOM_IN:
            // engine->_messageQueue->addMessageNextFrame(UIMSG_ClickZoomInBtn, 0, 0);
            break;

        case INPUT_ACTION_ZOOM_OUT:
            // engine->_messageQueue->addMessageNextFrame(UIMSG_ClickZoomOutBtn, 0, 0);
            break;

        case INPUT_ACTION_TOGGLE_ALWAYS_RUN:
            engine->config->settings.AlwaysRun.toggle();
            break;

        case INPUT_ACTION_ESCAPE:
            // engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, window_SpeakInHouse != 0, 0);
            break;

        case INPUT_ACTION_OPEN_INVENTORY:
            if (current_screen_type == SCREEN_GAME) {
                engine->_messageQueue->addMessageNextFrame(UIMSG_OpenInventory, 0, 0);
            }
            break;

        default:
            break;
        }
    }

    if (resettimer == true) {
        this->keydelaytimer = 0_ticks;
    } else {
        // use timer so pacing is consistent across framerates
        if (this->keydelaytimer < DELAY_TOGGLE_TIME_FIRST) this->keydelaytimer += pEventTimer->dt();
    }
}

//----- (0042FC4E) --------------------------------------------------------
void Io::KeyboardInputHandler::GenerateInputActions() {
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

    if (pEventTimer->isPaused()) {
        GeneratePausedActions();
    } else {
        GenerateGameplayActions();
    }

    controller->NextFrame();
}

//----- (00459E5A) --------------------------------------------------------
void Io::KeyboardInputHandler::StartTextInput(TextInputType type, int max_string_len, GUIWindow *window) {
    pPressedKeysBuffer.clear();
    inputType = type;

    this->max_input_string_len = max_string_len;
    this->window = window;

    if (window != nullptr) {
        window->keyboard_input_status = WINDOW_INPUT_IN_PROGRESS;
    }
}

void Io::KeyboardInputHandler::EndTextInput() {
    if (window != nullptr) {
        window->keyboard_input_status = WINDOW_INPUT_NONE;
    }
}

//----- (00459ED1) --------------------------------------------------------
void Io::KeyboardInputHandler::SetWindowInputStatus(WindowInputStatus status) {
    inputType = TextInputType::None;
    if (window) {
        window->keyboard_input_status = status;
    }
}

//----- (00459F10) --------------------------------------------------------
bool Io::KeyboardInputHandler::ProcessTextInput(PlatformKey key, int c) {
    if (currently_selected_action_for_binding == INPUT_ACTION_INVALID) {
        if (inputType != TextInputType::Text && inputType != TextInputType::Number) {
            return false;
        }

        if (key == PlatformKey::KEY_BACKSPACE) {
            if (!pPressedKeysBuffer.empty()) {
                pPressedKeysBuffer.pop_back();
            }
        } else if (key == PlatformKey::KEY_RETURN) {
            SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
        } else if (key == PlatformKey::KEY_ESCAPE) {
            SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
        } else if (key == PlatformKey::KEY_SPACE && pPressedKeysBuffer.size() < this->max_input_string_len) {
            if (inputType == TextInputType::Text) {
                pPressedKeysBuffer.push_back(' ');
            }
        } else if (key == PlatformKey::KEY_CHAR && pPressedKeysBuffer.size() < this->max_input_string_len) {
            if (inputType == TextInputType::Text) {
                pPressedKeysBuffer.push_back(c);
            } else if (inputType == TextInputType::Number && isdigit(c)) {
                pPressedKeysBuffer.push_back(c);
            }
        }
    } else {
        if (key != PlatformKey::KEY_CHAR) {
            // we're setting key binding in options
            // pPressedKeysBuffer[uNumKeysPressed++] = c;
            // pPressedKeysBuffer[uNumKeysPressed] = 0;
            lastKeyPressed = key;
            SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
        }
    }
    return true;
}


const std::string &Io::KeyboardInputHandler::GetTextInput() const {
    return pPressedKeysBuffer;
}

void Io::KeyboardInputHandler::SetTextInput(std::string_view text) {
    pPressedKeysBuffer = text;
}

//----- (00459E3F) --------------------------------------------------------
void Io::KeyboardInputHandler::ResetKeys() {
    for (auto action : allInputActions()) {
        // requesting KeyPressed will consume all the events due to how logic is designed in GetAsyncKeyState
        controller->ConsumeKeyPress(actionMapping->keyFor(action));
    }
}

bool Io::KeyboardInputHandler::IsRunKeyToggled() const {
    return controller->IsKeyDown(PlatformKey::KEY_SHIFT);
}

bool Io::KeyboardInputHandler::IsTurnStrafingToggled() const {
    return controller->IsKeyDown(PlatformKey::KEY_CONTROL);
}

bool Io::KeyboardInputHandler::IsStealingToggled() const {
    return controller->IsKeyDown(PlatformKey::KEY_CONTROL);
}

bool Io::KeyboardInputHandler::IsTakeAllToggled() const {
    return controller->IsKeyDown(PlatformKey::KEY_CONTROL);
}

bool Io::KeyboardInputHandler::IsAdventurerBackcycleToggled() const {
    return controller->IsKeyDown(PlatformKey::KEY_SHIFT);
}

bool Io::KeyboardInputHandler::IsSpellBackcycleToggled() const {
    return controller->IsKeyDown(PlatformKey::KEY_SHIFT);
}

bool Io::KeyboardInputHandler::IsCastOnClickToggled() const {
    return controller->IsKeyDown(PlatformKey::KEY_SHIFT);
}
