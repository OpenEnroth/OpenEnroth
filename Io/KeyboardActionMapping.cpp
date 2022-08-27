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
/*
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        if (keyMappingParams[i].m_cmdId == action) {
            return keyMappingParams[i].m_key;
        }
    }

    return GameKey::None;
*/
    return ConfigDefaultKey(action);
}

//----- (00459C82) --------------------------------------------------------
GameKey KeyboardActionMapping::GetKey(InputAction action) const {
    return actionKeyMap.find(action)->second;
}

KeyToggleType KeyboardActionMapping::GetToggleType(InputAction action) const {
    return keyToggleMap.find(action)->second;
}

//----- (00459C8D) --------------------------------------------------------
KeyboardActionMapping::KeyboardActionMapping(std::shared_ptr<Application::GameConfig> config) {
    this->config = config;

    SetDefaultMapping();
    ReadMappings();
}

//----- (00459CC4) --------------------------------------------------------
void KeyboardActionMapping::SetDefaultMapping() {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        //MapKey(keyMappingParams[i].m_cmdId, keyMappingParams[i].m_key, keyMappingParams[i].m_toggType);
        MapKey(keyMappingParams[i].m_cmdId, ConfigDefaultKey(keyMappingParams[i].m_cmdId), keyMappingParams[i].m_toggType);
    }
}

//----- (00459FFC) --------------------------------------------------------
void KeyboardActionMapping::ReadMappings() {
    char str[32];

    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        const char *keyName = keyMappingParams[i].m_keyName.c_str();
        //GameKey commandDefaultKeyCode = keyMappingParams[i].m_key;
        InputAction commandId = keyMappingParams[i].m_cmdId;
        KeyToggleType toggType = keyMappingParams[i].m_toggType;

        //OS_GetAppString(keyName, str, 32, "DEFAULT");
        //GameKey parsedKey = GameKey::None;
        //if (strcmp(str, "DEFAULT") != 0 && TryParseDisplayName(str, &parsedKey))
        //    MapKey(commandId, parsedKey);
        //else
        //    MapKey(commandId, commandDefaultKeyCode);
        GameKey key = ConfigGetKey(commandId);
        if (key != GameKey::None)
            MapKey(commandId, key);
        else
            MapKey(commandId, ConfigDefaultKey(commandId));

        keyToggleMap[commandId] = toggType;
    }
}

//----- (0045A960) --------------------------------------------------------
void KeyboardActionMapping::StoreMappings() {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        //std::string display_name = GetDisplayName(GetKey(keyMappingParams[i].m_cmdId));
        //OS_SetAppString(keyMappingParams[i].m_keyName.c_str(), display_name.c_str());
        ConfigSetKey(keyMappingParams[i].m_cmdId, GetKey(keyMappingParams[i].m_cmdId));
    }
}

KeyToggleType GetToggleType(InputAction action) {
    // TODO: TOGGLE_* values in the table above are not respected, instead toggle values from this function are used.

    if (action == InputAction::StrafeLeft || action == InputAction::StrafeRight
        || action == InputAction::FlyUp || action == InputAction::FlyDown) {
        return KeyToggleType::TOGGLE_Continuously;
    }

    if (action == InputAction::Attack || action == InputAction::CastReady)
        return KeyToggleType::TOGGLE_DelayContinuous;

    int i = (int)action;
    if (i > 3)
        return KeyToggleType::TOGGLE_OneTimePress;
    else
        return KeyToggleType::TOGGLE_Continuously;
}

GameKey KeyboardActionMapping::ConfigDefaultKey(InputAction action) {
    GameKey key = GameKey::None;

    switch (action) {
        case(InputAction::MoveForward):
            TryParseDisplayName(config->keybindings.DefaultForward(), &key);
            break;
        case(InputAction::MoveBackwards):
            TryParseDisplayName(config->keybindings.DefaultBackward(), &key);
            break;
        case(InputAction::TurnLeft):
            TryParseDisplayName(config->keybindings.DefaultLeft(), &key);
            break;
        case(InputAction::TurnRight):
            TryParseDisplayName(config->keybindings.DefaultRight(), &key);
            break;
        case(InputAction::Attack):
            TryParseDisplayName(config->keybindings.DefaultAttack(), &key);
            break;
        case(InputAction::CastReady):
            TryParseDisplayName(config->keybindings.DefaultCastReady(), &key);
            break;
        case(InputAction::Yell):
            TryParseDisplayName(config->keybindings.DefaultYell(), &key);
            break;
        case(InputAction::Jump):
            TryParseDisplayName(config->keybindings.DefaultJump(), &key);
            break;
        case(InputAction::Combat):
            TryParseDisplayName(config->keybindings.DefaultCombat(), &key);
            break;
        case(InputAction::EventTrigger):
            TryParseDisplayName(config->keybindings.DefaultEventTrigger(), &key);
            break;
        case(InputAction::Cast):
            TryParseDisplayName(config->keybindings.DefaultCast(), &key);
            break;
        case(InputAction::Pass):
            TryParseDisplayName(config->keybindings.DefaultPass(), &key);
            break;
        case(InputAction::CharCycle):
            TryParseDisplayName(config->keybindings.DefaultCharCycle(), &key);
            break;
        case(InputAction::Quest):
            TryParseDisplayName(config->keybindings.DefaultQuest(), &key);
            break;
        case(InputAction::QuickRef):
            TryParseDisplayName(config->keybindings.DefaultQuickReference(), &key);
            break;
        case(InputAction::Rest):
            TryParseDisplayName(config->keybindings.DefaultRest(), &key);
            break;
        case(InputAction::TimeCal):
            TryParseDisplayName(config->keybindings.DefaultTimeCalendar(), &key);
            break;
        case(InputAction::Autonotes):
            TryParseDisplayName(config->keybindings.DefaultAutoNotes(), &key);
            break;
        case(InputAction::Mapbook):
            TryParseDisplayName(config->keybindings.DefaultMapBook(), &key);
            break;
        case(InputAction::LookUp):
            TryParseDisplayName(config->keybindings.DefaultLookUp(), &key);
            break;
        case(InputAction::LookDown):
            TryParseDisplayName(config->keybindings.DefaultLookDown(), &key);
            break;
        case(InputAction::CenterView):
            TryParseDisplayName(config->keybindings.DefaultCenterView(), &key);
            break;
        case(InputAction::ZoomIn):
            TryParseDisplayName(config->keybindings.DefaultZoomIn(), &key);
            break;
        case(InputAction::ZoomOut):
            TryParseDisplayName(config->keybindings.DefaultZoomOut(), &key);
            break;
        case(InputAction::FlyUp):
            TryParseDisplayName(config->keybindings.DefaultFlyUp(), &key);
            break;
        case(InputAction::FlyDown):
            TryParseDisplayName(config->keybindings.DefaultFlyDown(), &key);
            break;
        case(InputAction::Land):
            TryParseDisplayName(config->keybindings.DefaultLand(), &key);
            break;
        case(InputAction::AlwaysRun):
            TryParseDisplayName(config->keybindings.DefaultAlwaysRun(), &key);
            break;
        case(InputAction::StrafeLeft):
            TryParseDisplayName(config->keybindings.DefaultStepLeft(), &key);
            break;
        case(InputAction::StrafeRight):
            TryParseDisplayName(config->keybindings.DefaultStepRight(), &key);
            break;
    }

    return GameKey::None;
}

GameKey KeyboardActionMapping::ConfigGetKey(InputAction action) {
    GameKey key = GameKey::None;

    switch (action) {
        case(InputAction::MoveForward):
            TryParseDisplayName(config->keybindings.GetForward(), &key);
            break;
        case(InputAction::MoveBackwards):
            TryParseDisplayName(config->keybindings.GetBackward(), &key);
            break;
        case(InputAction::TurnLeft):
            TryParseDisplayName(config->keybindings.GetLeft(), &key);
            break;
        case(InputAction::TurnRight):
            TryParseDisplayName(config->keybindings.GetRight(), &key);
            break;
        case(InputAction::Attack):
            TryParseDisplayName(config->keybindings.GetAttack(), &key);
            break;
        case(InputAction::CastReady):
            TryParseDisplayName(config->keybindings.GetCastReady(), &key);
            break;
        case(InputAction::Yell):
            TryParseDisplayName(config->keybindings.GetYell(), &key);
            break;
        case(InputAction::Jump):
            TryParseDisplayName(config->keybindings.GetJump(), &key);
            break;
        case(InputAction::Combat):
            TryParseDisplayName(config->keybindings.GetCombat(), &key);
            break;
        case(InputAction::EventTrigger):
            TryParseDisplayName(config->keybindings.GetEventTrigger(), &key);
            break;
        case(InputAction::Cast):
            TryParseDisplayName(config->keybindings.GetCast(), &key);
            break;
        case(InputAction::Pass):
            TryParseDisplayName(config->keybindings.GetPass(), &key);
            break;
        case(InputAction::CharCycle):
            TryParseDisplayName(config->keybindings.GetCharCycle(), &key);
            break;
        case(InputAction::Quest):
            TryParseDisplayName(config->keybindings.GetQuest(), &key);
            break;
        case(InputAction::QuickRef):
            TryParseDisplayName(config->keybindings.GetQuickReference(), &key);
            break;
        case(InputAction::Rest):
            TryParseDisplayName(config->keybindings.GetRest(), &key);
            break;
        case(InputAction::TimeCal):
            TryParseDisplayName(config->keybindings.GetTimeCalendar(), &key);
            break;
        case(InputAction::Autonotes):
            TryParseDisplayName(config->keybindings.GetAutoNotes(), &key);
            break;
        case(InputAction::Mapbook):
            TryParseDisplayName(config->keybindings.GetMapBook(), &key);
            break;
        case(InputAction::LookUp):
            TryParseDisplayName(config->keybindings.GetLookUp(), &key);
            break;
        case(InputAction::LookDown):
            TryParseDisplayName(config->keybindings.GetLookDown(), &key);
            break;
        case(InputAction::CenterView):
            TryParseDisplayName(config->keybindings.GetCenterView(), &key);
            break;
        case(InputAction::ZoomIn):
            TryParseDisplayName(config->keybindings.GetZoomIn(), &key);
            break;
        case(InputAction::ZoomOut):
            TryParseDisplayName(config->keybindings.GetZoomOut(), &key);
            break;
        case(InputAction::FlyUp):
            TryParseDisplayName(config->keybindings.GetFlyUp(), &key);
            break;
        case(InputAction::FlyDown):
            TryParseDisplayName(config->keybindings.GetFlyDown(), &key);
            break;
        case(InputAction::Land):
            TryParseDisplayName(config->keybindings.GetLand(), &key);
            break;
        case(InputAction::AlwaysRun):
            TryParseDisplayName(config->keybindings.GetAlwaysRun(), &key);
            break;
        case(InputAction::StrafeLeft):
            TryParseDisplayName(config->keybindings.GetStepLeft(), &key);
            break;
        case(InputAction::StrafeRight):
            TryParseDisplayName(config->keybindings.GetStepRight(), &key);
            break;
    }

    return key;
}

void KeyboardActionMapping::ConfigSetKey(InputAction action, GameKey key) {
    switch (action) {
        case(InputAction::MoveForward):
            config->keybindings.SetForward(GetDisplayName(key));
            break;
        case(InputAction::MoveBackwards):
            config->keybindings.SetBackward(GetDisplayName(key));
            break;
        case(InputAction::TurnLeft):
            config->keybindings.SetLeft(GetDisplayName(key));
            break;
        case(InputAction::TurnRight):
            config->keybindings.SetRight(GetDisplayName(key));
            break;
        case(InputAction::Attack):
            config->keybindings.SetAttack(GetDisplayName(key));
            break;
        case(InputAction::CastReady):
            config->keybindings.SetCastReady(GetDisplayName(key));
            break;
        case(InputAction::Yell):
            config->keybindings.SetYell(GetDisplayName(key));
            break;
        case(InputAction::Jump):
            config->keybindings.SetJump(GetDisplayName(key));
            break;
        case(InputAction::Combat):
            config->keybindings.SetCombat(GetDisplayName(key));
            break;
        case(InputAction::EventTrigger):
            config->keybindings.SetEventTrigger(GetDisplayName(key));
            break;
        case(InputAction::Cast):
            config->keybindings.SetCast(GetDisplayName(key));
            break;
        case(InputAction::Pass):
            config->keybindings.SetPass(GetDisplayName(key));
            break;
        case(InputAction::CharCycle):
            config->keybindings.SetCharCycle(GetDisplayName(key));
            break;
        case(InputAction::Quest):
            config->keybindings.SetQuest(GetDisplayName(key));
            break;
        case(InputAction::QuickRef):
            config->keybindings.SetQuickReference(GetDisplayName(key));
            break;
        case(InputAction::Rest):
            config->keybindings.SetRest(GetDisplayName(key));
            break;
        case(InputAction::TimeCal):
            config->keybindings.SetTimeCalendar(GetDisplayName(key));
            break;
        case(InputAction::Autonotes):
            config->keybindings.SetAutoNotes(GetDisplayName(key));
            break;
        case(InputAction::Mapbook):
            config->keybindings.SetMapBook(GetDisplayName(key));
            break;
        case(InputAction::LookUp):
            config->keybindings.SetLookUp(GetDisplayName(key));
            break;
        case(InputAction::LookDown):
            config->keybindings.SetLookDown(GetDisplayName(key));
            break;
        case(InputAction::CenterView):
            config->keybindings.SetCenterView(GetDisplayName(key));
            break;
        case(InputAction::ZoomIn):
            config->keybindings.SetZoomIn(GetDisplayName(key));
            break;
        case(InputAction::ZoomOut):
            config->keybindings.SetZoomOut(GetDisplayName(key));
            break;
        case(InputAction::FlyUp):
            config->keybindings.SetFlyUp(GetDisplayName(key));
            break;
        case(InputAction::FlyDown):
            config->keybindings.SetFlyDown(GetDisplayName(key));
            break;
        case(InputAction::Land):
            config->keybindings.SetLand(GetDisplayName(key));
            break;
        case(InputAction::AlwaysRun):
            config->keybindings.SetAlwaysRun(GetDisplayName(key));
            break;
        case(InputAction::StrafeLeft):
            config->keybindings.SetStepLeft(GetDisplayName(key));
            break;
        case(InputAction::StrafeRight):
            config->keybindings.SetStepRight(GetDisplayName(key));
            break;
    }
}

