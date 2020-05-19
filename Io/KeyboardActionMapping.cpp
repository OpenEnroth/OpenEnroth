#include <string>
#include <tuple>
#include <vector>

#include "Io/KeyboardActionMapping.h"

#include "Engine/Engine.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"

#include "Platform/Api.h"

using Io::InputAction;
using Io::KeyToggleType;


std::shared_ptr<KeyboardActionMapping> keyboardActionMapping = nullptr;

extern std::map<InputAction, bool> key_map_conflicted;  // 506E6C

class CKeyListElement {
 public:
    std::string m_keyName;
    GameKey m_key;
    InputAction m_cmdId;
    KeyToggleType m_toggType;
    CKeyListElement(std::string keyName, GameKey key,
        InputAction cmdId, KeyToggleType toggType)
        : m_keyName(keyName),
          m_key(key),
          m_cmdId(cmdId),
          m_toggType(toggType) {}
};

std::array<CKeyListElement, 30> keyMappingParams = {
    CKeyListElement("KEY_FORWARD", GameKey::Up, InputAction::MoveForward, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_BACKWARD", GameKey::Down, InputAction::MoveBackwards, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_LEFT", GameKey::Left, InputAction::TurnLeft, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_RIGHT", GameKey::Right, InputAction::TurnRight, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_ATTACK", GameKey::A, InputAction::Attack, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CASTREADY", GameKey::S, InputAction::CastReady, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_YELL", GameKey::Y, InputAction::Yell, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_JUMP", GameKey::X, InputAction::Jump, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_COMBAT", GameKey::Return, InputAction::Combat, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_EVENTTRIGGER", GameKey::Space, InputAction::EventTrigger, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CAST", GameKey::C, InputAction::Cast, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_PASS", GameKey::B, InputAction::Pass, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CHARCYCLE", GameKey::Tab, InputAction::CharCycle, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUEST", GameKey::Q, InputAction::Quest, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUICKREF", GameKey::Z, InputAction::QuickRef, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_REST", GameKey::R, InputAction::Rest, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TIMECAL", GameKey::T, InputAction::TimeCal, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_AUTONOTES", GameKey::N, InputAction::Autonotes, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_MAPBOOK", GameKey::M, InputAction::Mapbook, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOOKUP", GameKey::PageDown, InputAction::LookUp, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOOKDOWN", GameKey::Delete, InputAction::LookDown, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CENTERVIEWPT", GameKey::End, InputAction::CenterView, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ZOOMIN", GameKey::Add, InputAction::ZoomIn, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ZOOMOUT", GameKey::Subtract, InputAction::ZoomOut, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_FLYUP", GameKey::PageUp, InputAction::FlyUp, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_FLYDOWN", GameKey::Insert, InputAction::FlyDown, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_LAND", GameKey::Home, InputAction::Land, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ALWAYSRUN", GameKey::U, InputAction::AlwaysRun, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_STEPLEFT", GameKey::LeftBracket, InputAction::StrafeLeft, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_STEPRIGHT", GameKey::RightBracket, InputAction::StrafeRight, KeyToggleType::TOGGLE_Continuously)};


void KeyboardActionMapping::MapKey(InputAction action, GameKey key) {
    actionKeyMap[action] = key;
}
//----- (00459C68) --------------------------------------------------------
void KeyboardActionMapping::MapKey(InputAction action, GameKey key, KeyToggleType type) {
    actionKeyMap[action] = key;
    keyToggleMap[action] = type;
}

GameKey KeyboardActionMapping::MapDefaultKey(InputAction action) {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        if (keyMappingParams[i].m_cmdId == action) {
            return keyMappingParams[i].m_key;
        }
    }
}

//----- (00459C82) --------------------------------------------------------
GameKey KeyboardActionMapping::GetKey(InputAction action) const {
    return actionKeyMap.find(action)->second;
}

KeyToggleType KeyboardActionMapping::GetToggleType(InputAction action) const {
    return keyToggleMap.find(action)->second;
}

//----- (00459C8D) --------------------------------------------------------
KeyboardActionMapping::KeyboardActionMapping() {
    SetDefaultMapping();
    ReadMappings();
}

//----- (00459CC4) --------------------------------------------------------
void KeyboardActionMapping::SetDefaultMapping() {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        MapKey(keyMappingParams[i].m_cmdId,
                      keyMappingParams[i].m_key,
                      keyMappingParams[i].m_toggType);
    }
}

//----- (00459FFC) --------------------------------------------------------
void KeyboardActionMapping::ReadMappings() {
    char str[32];

    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        const char *keyName = keyMappingParams[i].m_keyName.c_str();
        GameKey commandDefaultKeyCode = keyMappingParams[i].m_key;
        InputAction commandId = keyMappingParams[i].m_cmdId;
        KeyToggleType toggType = keyMappingParams[i].m_toggType;

        OS_GetAppString(keyName, str, 32, "DEFAULT");
        GameKey parsedKey = GameKey::None;
        if (strcmp(str, "DEFAULT") != 0 && TryParseDisplayName(str, &parsedKey))
            MapKey(commandId, parsedKey);
        else
            MapKey(commandId, commandDefaultKeyCode);
        keyToggleMap[commandId] = toggType;
    }
}

//----- (0045A960) --------------------------------------------------------
void KeyboardActionMapping::StoreMappings() {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        const char *display_name = GetDisplayName(
            GetKey(keyMappingParams[i].m_cmdId)
        ).c_str();
        OS_SetAppString(keyMappingParams[i].m_keyName.c_str(), display_name);
    }
}

KeyToggleType GetToggleType(InputAction action) {
    if (action == InputAction::StrafeLeft || action == InputAction::StrafeRight
        || action == InputAction::FlyUp || action == InputAction::FlyDown) {
        return KeyToggleType::TOGGLE_Continuously;
    }

    int i = (int)action;
    if (i > 3)
        return KeyToggleType::TOGGLE_OneTimePress;
    else
        return KeyToggleType::TOGGLE_Continuously;
}
