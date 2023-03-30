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

using Io::InputAction;
using Io::KeyToggleType;


std::shared_ptr<KeyboardActionMapping> keyboardActionMapping = nullptr;

extern std::map<InputAction, bool> key_map_conflicted;  // 506E6C

class CKeyListElement {
 public:
    std::string m_keyName;
    PlatformKey m_key;
    InputAction m_cmdId;
    KeyToggleType m_toggType;
    CKeyListElement(std::string keyName, PlatformKey key,
                    InputAction cmdId, KeyToggleType toggType)
        : m_keyName(keyName),
          m_key(key),
          m_cmdId(cmdId),
          m_toggType(toggType) {}
};

std::array<CKeyListElement, std::to_underlying(InputAction::Count)> keyMappingParams = {
    CKeyListElement("KEY_FORWARD", PlatformKey::Up, InputAction::MoveForward, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_BACKWARD", PlatformKey::Down, InputAction::MoveBackwards, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_LEFT", PlatformKey::Left, InputAction::TurnLeft, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_RIGHT", PlatformKey::Right, InputAction::TurnRight, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_ATTACK", PlatformKey::A, InputAction::Attack, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CASTREADY", PlatformKey::S, InputAction::CastReady, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_YELL", PlatformKey::Y, InputAction::Yell, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_JUMP", PlatformKey::X, InputAction::Jump, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_COMBAT", PlatformKey::Return, InputAction::Combat, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_EVENTTRIGGER", PlatformKey::Space, InputAction::EventTrigger, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CAST", PlatformKey::C, InputAction::Cast, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_PASS", PlatformKey::B, InputAction::Pass, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CHARCYCLE", PlatformKey::Tab, InputAction::CharCycle, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUEST", PlatformKey::Q, InputAction::Quest, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUICKREF", PlatformKey::Z, InputAction::QuickRef, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_REST", PlatformKey::R, InputAction::Rest, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TIMECAL", PlatformKey::T, InputAction::TimeCal, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_AUTONOTES", PlatformKey::N, InputAction::Autonotes, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_MAPBOOK", PlatformKey::M, InputAction::Mapbook, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOOKUP", PlatformKey::PageDown, InputAction::LookUp, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOOKDOWN", PlatformKey::Delete, InputAction::LookDown, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CENTERVIEWPT", PlatformKey::End, InputAction::CenterView, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ZOOMIN", PlatformKey::Add, InputAction::ZoomIn, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ZOOMOUT", PlatformKey::Subtract, InputAction::ZoomOut, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_FLYUP", PlatformKey::PageUp, InputAction::FlyUp, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_FLYDOWN", PlatformKey::Insert, InputAction::FlyDown, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_LAND", PlatformKey::Home, InputAction::Land, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ALWAYSRUN", PlatformKey::U, InputAction::AlwaysRun, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_STEPLEFT", PlatformKey::LeftBracket, InputAction::StrafeLeft, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_STEPRIGHT", PlatformKey::RightBracket, InputAction::StrafeRight, KeyToggleType::TOGGLE_Continuously),

    CKeyListElement("KEY_QUICKSAVE", PlatformKey::F5, InputAction::QuickSave, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUICKLOAD", PlatformKey::F9, InputAction::QuickLoad, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_HISTORY", PlatformKey::H, InputAction::History, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_STATS", PlatformKey::C, InputAction::Stats, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SKILLS", PlatformKey::S, InputAction::Skills, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_INVENTORY", PlatformKey::I, InputAction::Inventory, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_AWARDS", PlatformKey::A, InputAction::Awards, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_NEWGAME", PlatformKey::N, InputAction::NewGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SAVEGAME", PlatformKey::S, InputAction::SaveGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOADGAME", PlatformKey::L, InputAction::LoadGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_EXITGAME", PlatformKey::Q, InputAction::ExitGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_RETURNTOGAME", PlatformKey::R, InputAction::ReturnToGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CONTROLS", PlatformKey::C, InputAction::Controls, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_OPTIONS", PlatformKey::O, InputAction::Options, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CREDITS", PlatformKey::C, InputAction::Credits, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CLEAR", PlatformKey::C, InputAction::Clear, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_RETURN", PlatformKey::Return, InputAction::Return, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SUBTRACT", PlatformKey::Subtract, InputAction::Minus, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ADD", PlatformKey::Add, InputAction::Plus, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_YES", PlatformKey::Y, InputAction::Yes, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_NO", PlatformKey::N, InputAction::No, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_REST8HOURS", PlatformKey::R, InputAction::Rest8Hours, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_WAITTILLDAWN", PlatformKey::D, InputAction::WaitTillDawn, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_WAITHOUR", PlatformKey::H, InputAction::WaitHour, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_WAIT5MINUTES", PlatformKey::M, InputAction::Wait5Minutes, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SCREENSHOT", PlatformKey::F2, InputAction::Screenshot, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CONSOLE", PlatformKey::Tilde, InputAction::Console, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLEMOUSEGRAB", PlatformKey::F1, InputAction::ToggleMouseGrab, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLEBORDERLESS", PlatformKey::F3, InputAction::ToggleBorderless, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLEFULLSCREEN", PlatformKey::F4, InputAction::ToggleFullscreen, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLERESIZABLE", PlatformKey::F6, InputAction::ToggleResizable, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CYCLEFILTER", PlatformKey::F7, InputAction::CycleFilter, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_RELOADSHADERS", PlatformKey::Backspace, InputAction::ReloadShaders, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTCHAR1", PlatformKey::Digit1, InputAction::SelectChar1, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTCHAR2", PlatformKey::Digit2, InputAction::SelectChar2, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTCHAR3", PlatformKey::Digit3, InputAction::SelectChar3, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTCHAR4", PlatformKey::Digit4, InputAction::SelectChar4, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTNPC1", PlatformKey::Digit5, InputAction::SelectNPC1, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTNPC2", PlatformKey::Digit6, InputAction::SelectNPC2, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGUP", PlatformKey::Up, InputAction::DialogUp, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGDOWN", PlatformKey::Down, InputAction::DialogDown, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGLEFT", PlatformKey::Left, InputAction::DialogLeft, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGRIGHT", PlatformKey::Right, InputAction::DialogRight, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGSELECT", PlatformKey::PageDown, InputAction::DialogSelect, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ESCAPE", PlatformKey::Escape, InputAction::Escape, KeyToggleType::TOGGLE_OneTimePress)};


void KeyboardActionMapping::MapKey(InputAction action, PlatformKey key) {
    actionKeyMap[action] = key;
}
//----- (00459C68) --------------------------------------------------------
void KeyboardActionMapping::MapKey(InputAction action, PlatformKey key, KeyToggleType type) {
    actionKeyMap[action] = key;
    keyToggleMap[action] = type;
}

void KeyboardActionMapping::MapGamepadKey(InputAction action, PlatformKey key) {
    gamepadKeyMap[action] = key;
}

PlatformKey KeyboardActionMapping::MapDefaultKey(InputAction action) {
    return ConfigDefaultKey(action);
}

//----- (00459C82) --------------------------------------------------------
PlatformKey KeyboardActionMapping::GetKey(InputAction action) const {
    return actionKeyMap.find(action)->second;
}

PlatformKey KeyboardActionMapping::GetGamepadKey(InputAction action) const {
    return gamepadKeyMap.find(action)->second;
}

KeyToggleType KeyboardActionMapping::GetToggleType(InputAction action) const {
    return keyToggleMap.find(action)->second;
}

// TODO: maybe we need to split InputActions to sets by WindowType so guarantee of only one InputAction per key is restored.
bool KeyboardActionMapping::IsKeyMatchAction(InputAction action, PlatformKey key) const {
    if (action == InputAction::Invalid)
        return false;

    if (actionKeyMap.find(action)->second == key)
        return true;

    if (gamepadKeyMap.find(action)->second == key)
        return true;

    return false;
}

//----- (00459C8D) --------------------------------------------------------
KeyboardActionMapping::KeyboardActionMapping(std::shared_ptr<GameConfig> config) {
    this->config = config;

    SetDefaultMapping();
    ReadMappings();
}

//----- (00459CC4) --------------------------------------------------------
void KeyboardActionMapping::SetDefaultMapping() {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        //MapKey(keyMappingParams[i].m_cmdId, keyMappingParams[i].m_key, keyMappingParams[i].m_toggType);
        MapKey(keyMappingParams[i].m_cmdId, ConfigDefaultKey(keyMappingParams[i].m_cmdId), keyMappingParams[i].m_toggType);
        // MapKey(keyMappingParams[i].m_cmdId, ConfigDefaultGamepadKey(keyMappingParams[i].m_cmdId));
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

        //GameKey parsedKey = GameKey::None;
        //if (strcmp(str, "DEFAULT") != 0 && TryParseDisplayName(str, &parsedKey))
        //    MapKey(commandId, parsedKey);
        //else
        //    MapKey(commandId, commandDefaultKeyCode);
        PlatformKey key = ConfigGetKey(commandId);
        if (key != PlatformKey::None)
            MapKey(commandId, key);
        else
            MapKey(commandId, ConfigDefaultKey(commandId));

        PlatformKey gamepadKey = ConfigGetGamepadKey(commandId);
        if (gamepadKey != PlatformKey::None)
            MapGamepadKey(commandId, gamepadKey);
        else
            MapGamepadKey(commandId, ConfigDefaultGamepadKey(commandId));

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

GameConfig::Key *KeyboardActionMapping::InputActionToConfigKey(InputAction action) {
    switch (action) {
        case(InputAction::MoveForward): return &config->keybindings.Forward;
        case(InputAction::MoveBackwards): return &config->keybindings.Backward;
        case(InputAction::TurnLeft): return &config->keybindings.Left;
        case(InputAction::TurnRight): return &config->keybindings.Right;
        case(InputAction::Attack): return &config->keybindings.Attack;
        case(InputAction::CastReady): return &config->keybindings.CastReady;
        case(InputAction::Yell): return &config->keybindings.Yell;
        case(InputAction::Jump): return &config->keybindings.Jump;
        case(InputAction::Combat): return &config->keybindings.Combat;
        case(InputAction::EventTrigger): return &config->keybindings.EventTrigger;
        case(InputAction::Cast): return &config->keybindings.Cast;
        case(InputAction::Pass): return &config->keybindings.Pass;
        case(InputAction::CharCycle): return &config->keybindings.CharCycle;
        case(InputAction::Quest): return &config->keybindings.Quest;
        case(InputAction::QuickRef): return &config->keybindings.QuickReference;
        case(InputAction::Rest): return &config->keybindings.Rest;
        case(InputAction::TimeCal): return &config->keybindings.TimeCalendar;
        case(InputAction::Autonotes): return &config->keybindings.AutoNotes;
        case(InputAction::Mapbook): return &config->keybindings.MapBook;
        case(InputAction::LookUp): return &config->keybindings.LookUp;
        case(InputAction::LookDown): return &config->keybindings.LookDown;
        case(InputAction::CenterView): return &config->keybindings.CenterView;
        case(InputAction::ZoomIn): return &config->keybindings.ZoomIn;
        case(InputAction::ZoomOut): return &config->keybindings.ZoomOut;
        case(InputAction::FlyUp): return &config->keybindings.FlyUp;
        case(InputAction::FlyDown): return &config->keybindings.FlyDown;
        case(InputAction::Land): return &config->keybindings.Land;
        case(InputAction::AlwaysRun): return &config->keybindings.AlwaysRun;
        case(InputAction::StrafeLeft): return &config->keybindings.StepLeft;
        case(InputAction::StrafeRight): return &config->keybindings.StepRight;
        case(InputAction::QuickSave): return &config->keybindings.QuickSave;
        case(InputAction::QuickLoad): return &config->keybindings.QuickLoad;
        case(InputAction::History): return &config->keybindings.History;
        case(InputAction::Stats): return &config->keybindings.Stats;
        case(InputAction::Skills): return &config->keybindings.Skills;
        case(InputAction::Inventory): return &config->keybindings.Inventory;
        case(InputAction::Awards): return &config->keybindings.Awards;
        case(InputAction::NewGame): return &config->keybindings.NewGame;
        case(InputAction::SaveGame): return &config->keybindings.SaveGame;
        case(InputAction::LoadGame): return &config->keybindings.LoadGame;
        case(InputAction::ExitGame): return &config->keybindings.ExitGame;
        case(InputAction::ReturnToGame): return &config->keybindings.ReturnToGame;
        case(InputAction::Controls): return &config->keybindings.Controls;
        case(InputAction::Options): return &config->keybindings.Options;
        case(InputAction::Credits): return &config->keybindings.Credits;
        case(InputAction::Clear): return &config->keybindings.Clear;
        case(InputAction::Return): return &config->keybindings.Return;
        case(InputAction::Minus): return &config->keybindings.Minus;
        case(InputAction::Plus): return &config->keybindings.Plus;
        case(InputAction::Yes): return &config->keybindings.Yes;
        case(InputAction::No): return &config->keybindings.No;
        case(InputAction::Rest8Hours): return &config->keybindings.Rest8Hours;
        case(InputAction::WaitTillDawn): return &config->keybindings.WaitTillDawn;
        case(InputAction::WaitHour): return &config->keybindings.WaitHour;
        case(InputAction::Wait5Minutes): return &config->keybindings.Wait5Minutes;
        case(InputAction::Screenshot): return &config->keybindings.Screenshot;
        case(InputAction::Console): return &config->keybindings.Console;
        case(InputAction::ToggleMouseGrab): return &config->keybindings.ToggleMouseGrab;
        case(InputAction::ToggleBorderless): return &config->keybindings.ToggleBorderless;
        case(InputAction::ToggleFullscreen): return &config->keybindings.ToggleFullscreen;
        case(InputAction::ToggleResizable): return &config->keybindings.ToggleResizable;
        case(InputAction::CycleFilter): return &config->keybindings.CycleFilter;
        case(InputAction::ReloadShaders): return &config->keybindings.ReloadShaders;
        case(InputAction::SelectChar1): return &config->keybindings.SelectChar1;
        case(InputAction::SelectChar2): return &config->keybindings.SelectChar2;
        case(InputAction::SelectChar3): return &config->keybindings.SelectChar3;
        case(InputAction::SelectChar4): return &config->keybindings.SelectChar4;
        case(InputAction::SelectNPC1): return &config->keybindings.SelectNPC1;
        case(InputAction::SelectNPC2): return &config->keybindings.SelectNPC2;
        case(InputAction::DialogUp): return &config->keybindings.DialogUp;
        case(InputAction::DialogDown): return &config->keybindings.DialogDown;
        case(InputAction::DialogLeft): return &config->keybindings.DialogLeft;
        case(InputAction::DialogRight): return &config->keybindings.DialogRight;
        case(InputAction::DialogSelect): return &config->keybindings.DialogSelect;
        case(InputAction::Escape): return &config->keybindings.Escape;
        default: break;
    }

    return nullptr;
}

PlatformKey KeyboardActionMapping::ConfigDefaultKey(InputAction action) {
    ConfigEntry<PlatformKey> *val = InputActionToConfigKey(action);
    return val ? val->defaultValue() : PlatformKey::None;
}

PlatformKey KeyboardActionMapping::ConfigGetKey(InputAction action) {
    ConfigEntry<PlatformKey> *val = InputActionToConfigKey(action);
    return val ? val->value() : PlatformKey::None;
}

void KeyboardActionMapping::ConfigSetKey(InputAction action, PlatformKey key) {
    if (ConfigEntry<PlatformKey> *val = InputActionToConfigKey(action))
        val->setValue(key);
}

PlatformKey KeyboardActionMapping::ConfigDefaultGamepadKey(InputAction action) {
    GameConfig::Key *val = InputActionToConfigGamepadKey(action);
    return val ? val->defaultValue() : PlatformKey::None;
}

GameConfig::Key *KeyboardActionMapping::InputActionToConfigGamepadKey(InputAction action) {
    switch (action) {
        case(InputAction::MoveForward): return &config->gamepad.Forward;
        case(InputAction::MoveBackwards): return &config->gamepad.Backward;
        case(InputAction::TurnLeft): return &config->gamepad.Left;
        case(InputAction::TurnRight): return &config->gamepad.Right;
        case(InputAction::Attack): return &config->gamepad.Attack;
        case(InputAction::CastReady): return &config->gamepad.CastReady;
        case(InputAction::Yell): return &config->gamepad.Yell;
        case(InputAction::Jump): return &config->gamepad.Jump;
        case(InputAction::Combat): return &config->gamepad.Combat;
        case(InputAction::EventTrigger): return &config->gamepad.EventTrigger;
        case(InputAction::Cast): return &config->gamepad.Cast;
        case(InputAction::Pass): return &config->gamepad.Pass;
        case(InputAction::CharCycle): return &config->gamepad.CharCycle;
        case(InputAction::Quest): return &config->gamepad.Quest;
        case(InputAction::QuickRef): return &config->gamepad.QuickReference;
        case(InputAction::Rest): return &config->gamepad.Rest;
        case(InputAction::TimeCal): return &config->gamepad.TimeCalendar;
        case(InputAction::Autonotes): return &config->gamepad.AutoNotes;
        case(InputAction::Mapbook): return &config->gamepad.MapBook;
        case(InputAction::LookUp): return &config->gamepad.LookUp;
        case(InputAction::LookDown): return &config->gamepad.LookDown;
        case(InputAction::CenterView): return &config->gamepad.CenterView;
        case(InputAction::ZoomIn): return &config->gamepad.ZoomIn;
        case(InputAction::ZoomOut): return &config->gamepad.ZoomOut;
        case(InputAction::FlyUp): return &config->gamepad.FlyUp;
        case(InputAction::FlyDown): return &config->gamepad.FlyDown;
        case(InputAction::Land): return &config->gamepad.Land;
        case(InputAction::AlwaysRun): return &config->gamepad.AlwaysRun;
        case(InputAction::StrafeLeft): return &config->gamepad.StepLeft;
        case(InputAction::StrafeRight): return &config->gamepad.StepRight;
        case(InputAction::QuickSave): return &config->gamepad.QuickSave;
        case(InputAction::QuickLoad): return &config->gamepad.QuickLoad;
        case(InputAction::History): return &config->gamepad.History;
        case(InputAction::Stats): return &config->gamepad.Stats;
        case(InputAction::Skills): return &config->gamepad.Skills;
        case(InputAction::Inventory): return &config->gamepad.Inventory;
        case(InputAction::Awards): return &config->gamepad.Awards;
        case(InputAction::NewGame): return &config->gamepad.NewGame;
        case(InputAction::SaveGame): return &config->gamepad.SaveGame;
        case(InputAction::LoadGame): return &config->gamepad.LoadGame;
        case(InputAction::ExitGame): return &config->gamepad.ExitGame;
        case(InputAction::ReturnToGame): return &config->gamepad.ReturnToGame;
        case(InputAction::Controls): return &config->gamepad.Controls;
        case(InputAction::Options): return &config->gamepad.Options;
        case(InputAction::Credits): return &config->gamepad.Credits;
        case(InputAction::Clear): return &config->gamepad.Clear;
        case(InputAction::Return): return &config->gamepad.Return;
        case(InputAction::Minus): return &config->gamepad.Minus;
        case(InputAction::Plus): return &config->gamepad.Plus;
        case(InputAction::Yes): return &config->gamepad.Yes;
        case(InputAction::No): return &config->gamepad.No;
        case(InputAction::Rest8Hours): return &config->gamepad.Rest8Hours;
        case(InputAction::WaitTillDawn): return &config->gamepad.WaitTillDawn;
        case(InputAction::WaitHour): return &config->gamepad.WaitHour;
        case(InputAction::Wait5Minutes): return &config->gamepad.Wait5Minutes;
        case(InputAction::Screenshot): return &config->gamepad.Screenshot;
        case(InputAction::Console): return &config->gamepad.Console;
        case(InputAction::ToggleMouseGrab): return &config->gamepad.ToggleMouseGrab;
        case(InputAction::ToggleBorderless): return &config->gamepad.ToggleBorderless;
        case(InputAction::ToggleFullscreen): return &config->gamepad.ToggleFullscreen;
        case(InputAction::ToggleResizable): return &config->gamepad.ToggleResizable;
        case(InputAction::CycleFilter): return &config->gamepad.CycleFilter;
        case(InputAction::ReloadShaders): return &config->gamepad.ReloadShaders;
        case(InputAction::SelectChar1): return &config->gamepad.SelectChar1;
        case(InputAction::SelectChar2): return &config->gamepad.SelectChar2;
        case(InputAction::SelectChar3): return &config->gamepad.SelectChar3;
        case(InputAction::SelectChar4): return &config->gamepad.SelectChar4;
        case(InputAction::SelectNPC1): return &config->gamepad.SelectNPC1;
        case(InputAction::SelectNPC2): return &config->gamepad.SelectNPC2;
        case(InputAction::DialogUp): return &config->gamepad.DialogUp;
        case(InputAction::DialogDown): return &config->gamepad.DialogDown;
        case(InputAction::DialogLeft): return &config->gamepad.DialogLeft;
        case(InputAction::DialogRight): return &config->gamepad.DialogRight;
        case(InputAction::DialogSelect): return &config->gamepad.DialogSelect;
        case(InputAction::Escape): return &config->gamepad.Escape;
        default: break;
    }

    return nullptr;
}

PlatformKey KeyboardActionMapping::ConfigGetGamepadKey(InputAction action) {
    GameConfig::Key *val = InputActionToConfigGamepadKey(action);
    return val ? val->value() : PlatformKey::None;
}

void KeyboardActionMapping::ConfigSetGamepadKey(InputAction action, PlatformKey key) {
    if (GameConfig::Key *val = InputActionToConfigGamepadKey(action))
        val->setValue(key);
}
