#ifdef _WINDOWS
#include <windows.h>
#else
#include "SDL.h"
#include "IO/Mouse.h"

// remove this after enabling SDLK_* keys
inline int64_t GetAsyncKeyState(int vKey) { return (int64_t)SDL_GetKeyboardState(&vKey); }
#endif
#undef PlaySound  // conflicts with Player method

#include <string>
#include <tuple>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/TurnEngine/TurnEngine.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"

#include "Engine/Spells/CastSpellInfo.h"

#include "IO/Keyboard.h"

#include "GUI/GUIWindow.h"

#include "Platform/Api.h"

struct KeyboardActionMapping *pKeyActionMap;

//----- (0044F07B) --------------------------------------------------------
bool Engine::_44F07B() {
    if (!keyboard->IsKeyBeingHeld(VK_SHIFT) &&
            !keyboard->IsKeyBeingHeld(VK_LSHIFT) &&
            !keyboard->IsKeyBeingHeld(VK_LSHIFT) ||
        (keyboard->WasKeyPressed(VK_F11) == 0 &&
            keyboard->WasKeyPressed(VK_F11)))
        return true;
    return false;
}

class CKeyListElement {
 public:
    std::string m_keyName;
    unsigned char m_keyDefaultCode;
    unsigned short m_cmdId;
    KeyToggleType m_toggType;
    CKeyListElement(std::string keyName, unsigned char keyDefaultCode,
                    unsigned short cmdId, KeyToggleType toggType)
        : m_keyName(keyName),
          m_keyDefaultCode(keyDefaultCode),
          m_cmdId(cmdId),
          m_toggType(toggType) {}
};

std::array<CKeyListElement, 30> keyMappingParams = {
    CKeyListElement("KEY_FORWARD", VK_UP, INPUT_MoveForward,
                    KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_BACKWARD", VK_DOWN, INPUT_MoveBackwards,
                    KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_LEFT", VK_LEFT, INPUT_TurnLeft, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_RIGHT", VK_RIGHT, INPUT_TurnRight,
                    KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_ATTACK", 'A', INPUT_Attack, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CASTREADY", 'S', INPUT_CastReady, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_YELL", 'Y', INPUT_Yell, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_JUMP", 'X', INPUT_Jump, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_COMBAT", VK_RETURN, INPUT_Combat, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_EVENTTRIGGER", VK_SPACE, INPUT_EventTrigger,
                    KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CAST", 'C', INPUT_Cast, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_PASS", 'B', INPUT_Pass, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CHARCYCLE", VK_TAB, INPUT_CharCycle,
                    KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUEST", 'Q', INPUT_Quest, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUICKREF", 'Z', INPUT_QuickRef, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_REST", 'R', INPUT_Rest, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TIMECAL", 'T', INPUT_TimeCal, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_AUTONOTES", 'N', INPUT_Autonotes, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_MAPBOOK", 'M', INPUT_Mapbook, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOOKUP", VK_NEXT, INPUT_LookUp, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOOKDOWN", VK_DELETE, INPUT_LookDown,
                    KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CENTERVIEWPT", VK_END, INPUT_CenterView,
                    KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ZOOMIN", VK_ADD, INPUT_ZoomIn, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ZOOMOUT", VK_SUBTRACT, INPUT_ZoomOut,
                    KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_FLYUP", VK_PRIOR, INPUT_FlyUp, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_FLYDOWN", VK_INSERT, INPUT_FlyDown,
                    KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_LAND", VK_HOME, INPUT_Land, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ALWAYSRUN", 'U', INPUT_AlwaysRun, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_STEPLEFT", VK_OEM_4, INPUT_StrafeLeft,
                    KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_STEPRIGHT", VK_OEM_6, INPUT_StrafeRight,
                    KeyToggleType::TOGGLE_Continuously)};

std::array<std::tuple<const char *, const unsigned __int8>, 26>
    keyNameToCodeTranslationMap = {
        std::tuple<const char *, const unsigned __int8>("UP", VK_UP),
        std::tuple<const char *, const unsigned __int8>("DOWN", VK_DOWN),
        std::tuple<const char *, const unsigned __int8>("LEFT", VK_LEFT),
        std::tuple<const char *, const unsigned __int8>("ВЛЕВО", VK_LEFT),
        std::tuple<const char *, const unsigned __int8>("RIGHT", VK_RIGHT),
        std::tuple<const char *, const unsigned __int8>("ВПРАВО", VK_RIGHT),
        std::tuple<const char *, const unsigned __int8>("RETURN", VK_RETURN),
        std::tuple<const char *, const unsigned __int8>("SPACE", VK_SPACE),
        std::tuple<const char *, const unsigned __int8>("PAGE_DOWN", VK_NEXT),
        std::tuple<const char *, const unsigned __int8>("PAGE_UP", VK_PRIOR),
        std::tuple<const char *, const unsigned __int8>("TAB", VK_TAB),
        std::tuple<const char *, const unsigned __int8>("SUBTRACT",
                                                        VK_SUBTRACT),
        std::tuple<const char *, const unsigned __int8>("ADD", VK_ADD),
        std::tuple<const char *, const unsigned __int8>("END", VK_END),
        std::tuple<const char *, const unsigned __int8>("DELETE", VK_DELETE),
        std::tuple<const char *, const unsigned __int8>("HOME", VK_HOME),
        std::tuple<const char *, const unsigned __int8>("INSERT", VK_INSERT),
        std::tuple<const char *, const unsigned __int8>("COMMA", VK_OEM_COMMA),
        std::tuple<const char *, const unsigned __int8>("DECIMAL", VK_DECIMAL),
        std::tuple<const char *, const unsigned __int8>("SEMICOLON", VK_OEM_1),
        std::tuple<const char *, const unsigned __int8>("PERIOD",
                                                        VK_OEM_PERIOD),
        std::tuple<const char *, const unsigned __int8>("SLASH", VK_OEM_2),
        std::tuple<const char *, const unsigned __int8>("SQUOTE", VK_OEM_7),
        std::tuple<const char *, const unsigned __int8>("BACKSLASH", VK_OEM_5),
        std::tuple<const char *, const unsigned __int8>("BACKSPACE", VK_BACK),
        std::tuple<const char *, const unsigned __int8>("CONTROL", VK_CONTROL),
};

//----- (00459C68) --------------------------------------------------------
void KeyboardActionMapping::SetKeyMapping(int uAction, int vKey,
                                          KeyToggleType type) {
    pVirtualKeyCodesMapping[uAction] = vKey;
    pToggleTypes[uAction] = type;
}

//----- (00459C82) --------------------------------------------------------
unsigned int KeyboardActionMapping::GetActionVKey(enum InputAction eAction) {
    return this->pVirtualKeyCodesMapping[eAction];
}

//----- (00459C8D) --------------------------------------------------------
KeyboardActionMapping::KeyboardActionMapping() {
    uLastKeyPressed = 0;
    field_204 = 0;
    pWindow = nullptr;

    SetDefaultMapping();
    ReadMappings();

    ResetKeys();

    uNumKeysPressed = 0;

    uGameMenuUI_CurentlySelectedKeyIdx = -1;
}
// 506E68: using guessed type int uGameMenuUI_CurentlySelectedKeyIdx;

//----- (00459CC4) --------------------------------------------------------
void KeyboardActionMapping::SetDefaultMapping() {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        SetKeyMapping(keyMappingParams[i].m_cmdId,
                      keyMappingParams[i].m_keyDefaultCode,
                      keyMappingParams[i].m_toggType);
    }
}

//----- (00459E3F) --------------------------------------------------------
void KeyboardActionMapping::ResetKeys() {
    for (uint i = 0; i < 30; ++i) GetAsyncKeyState(pVirtualKeyCodesMapping[i]);
}

//----- (00459E5A) --------------------------------------------------------
void KeyboardActionMapping::EnterText(int a2, int max_string_len,
                                      GUIWindow *pWindow) {
    memset(this->pPressedKeysBuffer, 0, 0x101u);
    this->uNumKeysPressed = 0;
    if (a2)
        this->field_204 = 2;
    else
        this->field_204 = 1;
    this->max_input_string_len = max_string_len;
    this->pWindow = pWindow;
    pWindow->receives_keyboard_input_2 = WINDOW_INPUT_IN_PROGRESS;
}

//----- (00459ED1) --------------------------------------------------------
void KeyboardActionMapping::SetWindowInputStatus(int a2) {
    field_204 = 0;
    if (pWindow) pWindow->receives_keyboard_input_2 = a2;
}

//----- (00459F10) --------------------------------------------------------
bool KeyboardActionMapping::ProcessTextInput(unsigned int a2) {
    pKeyActionMap->uLastKeyPressed = a2;
    if (uGameMenuUI_CurentlySelectedKeyIdx == -1) {
        if (pKeyActionMap->field_204 != 1 && pKeyActionMap->field_204 != 2)
            return 0;
        if (a2 == VK_BACK) {
            if (pKeyActionMap->uNumKeysPressed > 0) {
                --pKeyActionMap->uNumKeysPressed;
                pKeyActionMap
                    ->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = 0;
            }
        } else if (a2 == VK_RETURN) {
            pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
        } else if (a2 == VK_ESCAPE) {
            pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
        } else if (this->uNumKeysPressed < this->max_input_string_len) {
            if (pKeyActionMap->field_204 == 1) {
                if (a2 != VK_TAB) {
                    pKeyActionMap
                        ->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] =
                        a2;
                    ++pKeyActionMap->uNumKeysPressed;
                    pKeyActionMap
                        ->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] =
                        0;
                }
            } else if (pKeyActionMap->field_204 == 2) {
                if (isdigit(a2)) {
                    pKeyActionMap
                        ->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] =
                        a2;
                    ++pKeyActionMap->uNumKeysPressed;
                }
            }
        }
    } else {
        pKeyActionMap->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = a2;
        ++pKeyActionMap->uNumKeysPressed;
        pKeyActionMap->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = 0;
        pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
    }
    return 1;
}
// 506E68: using guessed type int uGameMenuUI_CurentlySelectedKeyIdx;

//----- (00459FFC) --------------------------------------------------------
void KeyboardActionMapping::ReadMappings() {
    char str[32];

    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        const char *keyName = keyMappingParams[i].m_keyName.c_str();
        short commandDefaultKeyCode = keyMappingParams[i].m_keyDefaultCode;
        short commandId = keyMappingParams[i].m_cmdId;
        KeyToggleType toggType = keyMappingParams[i].m_toggType;

        OS_GetAppString(keyName, str, 32, "DEFAULT");
        if (strcmp(str, "DEFAULT") && (TranslateKeyNameToKeyCode(str) !=
                                       0xff))  // returns 0xff on error not -1
            pVirtualKeyCodesMapping[commandId] = TranslateKeyNameToKeyCode(str);
        else
            pVirtualKeyCodesMapping[commandId] = commandDefaultKeyCode;
        pToggleTypes[commandId] = toggType;
    }
}

//----- (0045A960) --------------------------------------------------------
void KeyboardActionMapping::StoreMappings() {
    const char *v2;  // eax@1

    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        v2 = GetVKeyDisplayName(
            pVirtualKeyCodesMapping[keyMappingParams[i].m_cmdId]);
        OS_SetAppString(keyMappingParams[i].m_keyName.c_str(), v2);
    }
}

//----- (0045ABCA) --------------------------------------------------------
const unsigned __int8 KeyboardActionMapping::TranslateKeyNameToKeyCode(
    const char *Str) {
    if (strlen(Str) == 1) {
        if (Str[0] >= 65 && Str[0] <= 90)
            return *Str;
        else
            return 0xFF;
    }

    for (size_t i = 0; i < keyNameToCodeTranslationMap.size(); i++) {
        if (!strcmp(Str, std::get<0>(keyNameToCodeTranslationMap[i])))
            return std::get<1>(keyNameToCodeTranslationMap[i]);
    }
    return 0xFF;
}

//----- (0045AE2C) --------------------------------------------------------
const char *KeyboardActionMapping::GetVKeyDisplayName(unsigned char a1) {
    static char static_sub_45AE2C_string_69ADE0_keyName[32];

    if (a1 >= 65 && a1 <= 90) {
        static_sub_45AE2C_string_69ADE0_keyName[0] = a1;
        static_sub_45AE2C_string_69ADE0_keyName[1] = '\0';
        return static_sub_45AE2C_string_69ADE0_keyName;
    }

    for (size_t i = 0; i < keyNameToCodeTranslationMap.size(); i++) {
        if (a1 == std::get<1>(keyNameToCodeTranslationMap[i])) {
            const char *keyName = std::get<0>(keyNameToCodeTranslationMap[i]);
            strcpy(static_sub_45AE2C_string_69ADE0_keyName, keyName);
            return static_sub_45AE2C_string_69ADE0_keyName;
        }
    }

    strcpy(static_sub_45AE2C_string_69ADE0_keyName, "-BAD KEY-");
    return static_sub_45AE2C_string_69ADE0_keyName;
}

//----- (0045B06E) --------------------------------------------------------
bool Keyboard::IsShiftHeld() {
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

//----- (0042FC4E) --------------------------------------------------------
void Keyboard::ProcessInputActions() {
    char v4;                  // al@9
    // unsigned __int16 v9;      // ax@102
    // int spell_price;          // eax@103
    PartyAction partyAction;  // [sp-14h] [bp-1Ch]@20
    InputAction inputAction;  // [sp+0h] [bp-8h]@7

    if (!engine->config->always_run) {
        if (this->IsShiftHeld())
            pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;
        else
            pParty->uFlags2 &= ~PARTY_FLAGS_2_RUNNING;
    } else {
        if (this->IsShiftHeld())
            pParty->uFlags2 &= ~PARTY_FLAGS_2_RUNNING;
        else
            pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;
    }

    // pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;

    if (pEventTimer->bPaused) {  // game paused
        for (uint i = 0; i < 30; ++i) {
            inputAction = (InputAction)i;
            if (pKeyActionMap->pToggleTypes[inputAction] == KeyToggleType::TOGGLE_OneTimePress)
                v4 = WasKeyPressed(
                    pKeyActionMap->pVirtualKeyCodesMapping[inputAction]);
            else
                v4 = IsKeyBeingHeld(
                    pKeyActionMap->pVirtualKeyCodesMapping[inputAction]);

            if (v4) {
                if (inputAction == INPUT_EventTrigger) {
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
            if (pKeyActionMap->pToggleTypes[inputAction] == KeyToggleType::TOGGLE_OneTimePress)
                v4 = this->WasKeyPressed(pKeyActionMap->pVirtualKeyCodesMapping[inputAction]);
            else
                v4 = this->IsKeyBeingHeld(pKeyActionMap->pVirtualKeyCodesMapping[inputAction]);
            if (v4) {
                switch (inputAction) {
                    case INPUT_MoveForward:
                        if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                        if (!pParty->bTurnBasedModeOn) {
                            if (pParty->uFlags2 & PARTY_FLAGS_2_RUNNING)
                                partyAction = PARTY_RunForward;
                            else
                                partyAction = PARTY_WalkForward;
                            pPartyActionQueue->Add(partyAction);
                            break;
                        }
                        if (pTurnEngine->turn_stage != TE_WAIT &&
                            pTurnEngine->turn_stage != TE_ATTACK &&
                            pTurnEngine->uActionPointsLeft > 0) {
                            pTurnEngine->uActionPointsLeft -= 26;
                            if (pParty->uFlags2 & PARTY_FLAGS_2_RUNNING)
                                partyAction = PARTY_RunForward;
                            else
                                partyAction = PARTY_WalkForward;
                            pPartyActionQueue->Add(partyAction);
                            break;
                        }
                        break;
                    case INPUT_MoveBackwards:
                        if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                        if (!pParty->bTurnBasedModeOn) {
                            if (pParty->uFlags2 & 2)
                                partyAction = PARTY_RunBackward;
                            else
                                partyAction = PARTY_WalkBackward;
                            pPartyActionQueue->Add(partyAction);
                            break;
                        }
                        if (pTurnEngine->turn_stage != TE_WAIT &&
                            pTurnEngine->turn_stage != TE_ATTACK &&
                            pTurnEngine->uActionPointsLeft > 0) {
                            pTurnEngine->uActionPointsLeft -= 26;
                            if (pParty->uFlags2 & 2)
                                partyAction = PARTY_RunBackward;
                            else
                                partyAction = PARTY_WalkBackward;
                            pPartyActionQueue->Add(partyAction);
                            break;
                        }
                        break;
                    case INPUT_StrafeLeft:
                        if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                        if (!pParty->bTurnBasedModeOn) {
                            partyAction = PARTY_StrafeLeft;
                            pPartyActionQueue->Add(partyAction);
                            break;
                        }
                        if (pTurnEngine->turn_stage == TE_WAIT ||
                            pTurnEngine->turn_stage == TE_ATTACK ||
                            pTurnEngine->uActionPointsLeft <= 0)
                            break;
                        pTurnEngine->uActionPointsLeft -= 26;
                        partyAction = PARTY_StrafeLeft;
                        pPartyActionQueue->Add(partyAction);
                        break;
                    case INPUT_StrafeRight:
                        if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                        if (!pParty->bTurnBasedModeOn) {
                            partyAction = PARTY_StrafeRight;
                            pPartyActionQueue->Add(partyAction);
                            break;
                        }
                        if (pTurnEngine->turn_stage == TE_WAIT ||
                            pTurnEngine->turn_stage == TE_ATTACK ||
                            pTurnEngine->uActionPointsLeft <= 0)
                            break;
                        pTurnEngine->uActionPointsLeft -= 26;
                        partyAction = PARTY_StrafeRight;
                        pPartyActionQueue->Add(partyAction);
                        break;
                    case INPUT_TurnLeft:
                        if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                        if (GetAsyncKeyState(VK_CONTROL)) {  // strafing
                            if (pParty->bTurnBasedModeOn) {
                                if (pTurnEngine->turn_stage == TE_WAIT ||
                                    pTurnEngine->turn_stage == TE_ATTACK ||
                                    pTurnEngine->uActionPointsLeft <= 0)
                                    break;
                                pTurnEngine->uActionPointsLeft -= 26;
                            }
                            partyAction = PARTY_StrafeLeft;
                        } else {
                            if (pParty->uFlags2 & 2)
                                partyAction = PARTY_FastTurnLeft;
                            else
                                partyAction = PARTY_TurnLeft;
                        }
                        pPartyActionQueue->Add(partyAction);
                        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor &&
                            pWeather->bRenderSnow)
                            pWeather->OnPlayerTurn(10);
                        break;
                    case INPUT_TurnRight:
                        if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                        if (GetAsyncKeyState(VK_CONTROL)) {  // strafing
                            if (pParty->bTurnBasedModeOn) {
                                if (pTurnEngine->turn_stage == TE_WAIT ||
                                    pTurnEngine->turn_stage == TE_ATTACK ||
                                    pTurnEngine->uActionPointsLeft <= 0)
                                    break;
                                pTurnEngine->uActionPointsLeft -= 26;
                            }
                            partyAction = PARTY_StrafeRight;
                        } else {
                            if (pParty->uFlags2 & 2)
                                partyAction = PARTY_FastTurnRight;
                            else
                                partyAction = PARTY_TurnRight;
                        }
                        pPartyActionQueue->Add(partyAction);
                        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor &&
                            pWeather->bRenderSnow)
                            pWeather->OnPlayerTurn(-10);
                        break;
                    case INPUT_Jump:
                        if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME ||
                            pParty->bTurnBasedModeOn)
                            break;
                        partyAction = (PartyAction)12;
                        pPartyActionQueue->Add(partyAction);
                        break;
                    case INPUT_Yell:
                        if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME && uActiveCharacter) {
                            pParty->Yell();
                            pPlayers[uActiveCharacter]->PlaySound(SPEECH_Yell,
                                                                  0);
                        }
                        break;
                    case INPUT_Pass:
                        if (!(current_screen_type == CURRENT_SCREEN::SCREEN_GAME)) break;
                        if (pParty->bTurnBasedModeOn &&
                            pTurnEngine->turn_stage == TE_MOVEMENT) {
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
                                CastSpellInfoHelpers::Cancel_Spell_Cast_In_Progress();
                                pTurnEngine->ApplyPlayerAction();
                            }
                        }
                        break;
                    case INPUT_Combat:  // if press ENTER
                        if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                            if (pParty->bTurnBasedModeOn) {
                                if (pTurnEngine->turn_stage == TE_MOVEMENT ||
                                    PID_TYPE(
                                        pTurnEngine->pQueue[0].uPackedID) ==
                                        OBJECT_Player) {
                                    pParty->bTurnBasedModeOn = false;
                                    pTurnEngine->End(true);
                                }
                            } else {
                                pTurnEngine->Start();
                                pParty->bTurnBasedModeOn = true;
                            }
                        }
                        break;
                    case INPUT_CastReady: {
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
                        } else {
                            pMessageQueue_50C9E8->AddGUIMessage(UIMSG_CastQuickSpell, 0, 0);
                        }
                    } break;
                    case INPUT_Attack:
                        if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) break;
                        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                            pTurnEngine->field_18 |= TE_FLAG_8;
                            break;
                        }
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Attack, 0, 0);
                        break;
                    case INPUT_EventTrigger:
                        if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Game_Action, 0, 0);
                            break;
                        }
                        if (current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE) {
                            pMessageQueue_50CBD0->Clear();
                            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
                        }
                        break;
                    case INPUT_CharCycle:
                        if (current_screen_type == CURRENT_SCREEN::SCREEN_SPELL_BOOK) break;

                        pMessageQueue_50C9E8->AddGUIMessage(UIMSG_CycleCharacters, 0, 0);
                        break;
                    case INPUT_LookUp:
                        if (pEventTimer->bPaused) break;
                        partyAction = (PartyAction)7;
                        pPartyActionQueue->Add(partyAction);
                        break;
                    case INPUT_CenterView:
                        if (pEventTimer->bPaused) break;
                        partyAction = (PartyAction)9;
                        pPartyActionQueue->Add(partyAction);
                        break;
                    case INPUT_LookDown:
                        if (pEventTimer->bPaused) break;
                        partyAction = (PartyAction)8;
                        pPartyActionQueue->Add(partyAction);
                        break;
                    case INPUT_FlyUp:
                        if (!(current_screen_type == CURRENT_SCREEN::SCREEN_GAME) || pEventTimer->bPaused) break;
                        partyAction = (PartyAction)13;
                        pPartyActionQueue->Add(partyAction);
                        break;
                    case INPUT_Land:
                        if (!(current_screen_type == CURRENT_SCREEN::SCREEN_GAME) || pEventTimer->bPaused) break;
                        partyAction = (PartyAction)15;
                        pPartyActionQueue->Add(partyAction);
                        break;
                    case INPUT_FlyDown:
                        if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME && !pEventTimer->bPaused) {
                            partyAction = (PartyAction)14;
                            pPartyActionQueue->Add(partyAction);
                        }
                        break;
                    case INPUT_ZoomIn:
                        pMessageQueue_50C9E8->AddGUIMessage(UIMSG_ClickZoomOutBtn, 0, 0);
                        break;
                    case INPUT_ZoomOut:
                        pMessageQueue_50C9E8->AddGUIMessage(UIMSG_ClickZoomInBtn, 0, 0);
                        break;
                    case INPUT_AlwaysRun:
                        engine->ToggleAlwaysRun();
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
