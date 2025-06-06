#include "KeyboardActionMapping.h"

#include <string>
#include <map>
#include <memory>

#include "Engine/Graphics/Weather.h"
#include "Engine/Spells/CastSpellInfo.h"

using Io::InputAction;
using Io::KeyToggleType;


std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping = nullptr;

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
    CKeyListElement("KEY_FORWARD", PlatformKey::KEY_UP, Io::InputAction::MoveForward, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_BACKWARD", PlatformKey::KEY_DOWN, Io::InputAction::MoveBackwards, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_LEFT", PlatformKey::KEY_LEFT, Io::InputAction::TurnLeft, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_RIGHT", PlatformKey::KEY_RIGHT, Io::InputAction::TurnRight, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_ATTACK", PlatformKey::KEY_A, Io::InputAction::Attack, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CASTREADY", PlatformKey::KEY_S, Io::InputAction::CastReady, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_YELL", PlatformKey::KEY_Y, Io::InputAction::Yell, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_JUMP", PlatformKey::KEY_X, Io::InputAction::Jump, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_COMBAT", PlatformKey::KEY_RETURN, Io::InputAction::Combat, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_EVENTTRIGGER", PlatformKey::KEY_SPACE, Io::InputAction::EventTrigger, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CAST", PlatformKey::KEY_C, Io::InputAction::Cast, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_PASS", PlatformKey::KEY_B, Io::InputAction::Pass, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CHARCYCLE", PlatformKey::KEY_TAB, Io::InputAction::CharCycle, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUEST", PlatformKey::KEY_Q, Io::InputAction::Quest, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUICKREF", PlatformKey::KEY_Z, Io::InputAction::QuickRef, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_REST", PlatformKey::KEY_R, Io::InputAction::Rest, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TIMECAL", PlatformKey::KEY_T, Io::InputAction::TimeCal, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_AUTONOTES", PlatformKey::KEY_N, Io::InputAction::Autonotes, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_MAPBOOK", PlatformKey::KEY_M, Io::InputAction::Mapbook, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOOKUP", PlatformKey::KEY_PAGEDOWN, Io::InputAction::LookUp, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOOKDOWN", PlatformKey::KEY_DELETE, Io::InputAction::LookDown, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CENTERVIEWPT", PlatformKey::KEY_END, Io::InputAction::CenterView, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ZOOMIN", PlatformKey::KEY_ADD, Io::InputAction::ZoomIn, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ZOOMOUT", PlatformKey::KEY_SUBTRACT, Io::InputAction::ZoomOut, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_FLYUP", PlatformKey::KEY_PAGEUP, Io::InputAction::FlyUp, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_FLYDOWN", PlatformKey::KEY_INSERT, Io::InputAction::FlyDown, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_LAND", PlatformKey::KEY_HOME, Io::InputAction::Land, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ALWAYSRUN", PlatformKey::KEY_U, Io::InputAction::AlwaysRun, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_STEPLEFT", PlatformKey::KEY_LEFTBRACKET, Io::InputAction::StrafeLeft, KeyToggleType::TOGGLE_Continuously),
    CKeyListElement("KEY_STEPRIGHT", PlatformKey::KEY_RIGHTBRACKET, Io::InputAction::StrafeRight, KeyToggleType::TOGGLE_Continuously),

    CKeyListElement("KEY_QUICKSAVE", PlatformKey::KEY_F5, Io::InputAction::QuickSave, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_QUICKLOAD", PlatformKey::KEY_F9, Io::InputAction::QuickLoad, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_HISTORY", PlatformKey::KEY_H, Io::InputAction::History, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_STATS", PlatformKey::KEY_C, Io::InputAction::Stats, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SKILLS", PlatformKey::KEY_S, Io::InputAction::Skills, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_INVENTORY", PlatformKey::KEY_I, Io::InputAction::Inventory, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_AWARDS", PlatformKey::KEY_A, Io::InputAction::Awards, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_NEWGAME", PlatformKey::KEY_N, Io::InputAction::NewGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SAVEGAME", PlatformKey::KEY_S, Io::InputAction::SaveGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_LOADGAME", PlatformKey::KEY_L, Io::InputAction::LoadGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_EXITGAME", PlatformKey::KEY_Q, Io::InputAction::ExitGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_RETURNTOGAME", PlatformKey::KEY_R, Io::InputAction::ReturnToGame, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CONTROLS", PlatformKey::KEY_C, Io::InputAction::Controls, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_OPTIONS", PlatformKey::KEY_O, Io::InputAction::Options, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CREDITS", PlatformKey::KEY_C, Io::InputAction::Credits, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CLEAR", PlatformKey::KEY_C, Io::InputAction::Clear, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_RETURN", PlatformKey::KEY_RETURN, Io::InputAction::Return, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SUBTRACT", PlatformKey::KEY_SUBTRACT, Io::InputAction::Minus, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ADD", PlatformKey::KEY_ADD, Io::InputAction::Plus, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_YES", PlatformKey::KEY_Y, Io::InputAction::Yes, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_NO", PlatformKey::KEY_N, Io::InputAction::No, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_REST8HOURS", PlatformKey::KEY_R, Io::InputAction::Rest8Hours, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_WAITTILLDAWN", PlatformKey::KEY_D, Io::InputAction::WaitTillDawn, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_WAITHOUR", PlatformKey::KEY_H, Io::InputAction::WaitHour, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_WAIT5MINUTES", PlatformKey::KEY_M, Io::InputAction::Wait5Minutes, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SCREENSHOT", PlatformKey::KEY_F2, Io::InputAction::Screenshot, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CONSOLE", PlatformKey::KEY_TILDE, Io::InputAction::Console, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLEMOUSEGRAB", PlatformKey::KEY_F1, Io::InputAction::ToggleMouseGrab, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLEBORDERLESS", PlatformKey::KEY_F3, Io::InputAction::ToggleBorderless, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLEFULLSCREEN", PlatformKey::KEY_F4, Io::InputAction::ToggleFullscreen, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLERESIZABLE", PlatformKey::KEY_F6, Io::InputAction::ToggleResizable, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_CYCLEFILTER", PlatformKey::KEY_F7, Io::InputAction::CycleFilter, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_RELOADSHADERS", PlatformKey::KEY_BACKSPACE, Io::InputAction::ReloadShaders, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTCHAR1", PlatformKey::KEY_DIGIT_1, Io::InputAction::SelectChar1, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTCHAR2", PlatformKey::KEY_DIGIT_2, Io::InputAction::SelectChar2, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTCHAR3", PlatformKey::KEY_DIGIT_3, Io::InputAction::SelectChar3, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTCHAR4", PlatformKey::KEY_DIGIT_4, Io::InputAction::SelectChar4, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTNPC1", PlatformKey::KEY_DIGIT_5, Io::InputAction::SelectNPC1, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_SELECTNPC2", PlatformKey::KEY_DIGIT_6, Io::InputAction::SelectNPC2, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGUP", PlatformKey::KEY_UP, Io::InputAction::DialogUp, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGDOWN", PlatformKey::KEY_DOWN, Io::InputAction::DialogDown, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGLEFT", PlatformKey::KEY_LEFT, Io::InputAction::DialogLeft, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGRIGHT", PlatformKey::KEY_RIGHT, Io::InputAction::DialogRight, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_DIALOGSELECT", PlatformKey::KEY_PAGEDOWN, Io::InputAction::DialogSelect, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_ESCAPE", PlatformKey::KEY_ESCAPE, Io::InputAction::Escape, KeyToggleType::TOGGLE_OneTimePress),
    CKeyListElement("KEY_TOGGLEMOUSELOOK", PlatformKey::KEY_F10, Io::InputAction::ToggleMouseLook, KeyToggleType::TOGGLE_OneTimePress),
};


void Io::KeyboardActionMapping::MapKey(InputAction action, PlatformKey key) {
    actionKeyMap[action] = key;
}
//----- (00459C68) --------------------------------------------------------
void Io::KeyboardActionMapping::MapKey(InputAction action, PlatformKey key, KeyToggleType type) {
    actionKeyMap[action] = key;
    keyToggleMap[action] = type;
}

void Io::KeyboardActionMapping::MapGamepadKey(InputAction action, PlatformKey key) {
    gamepadKeyMap[action] = key;
}

PlatformKey Io::KeyboardActionMapping::MapDefaultKey(InputAction action) {
    return ConfigDefaultKey(action);
}

//----- (00459C82) --------------------------------------------------------
PlatformKey Io::KeyboardActionMapping::GetKey(InputAction action) const {
    return actionKeyMap.find(action)->second;
}

PlatformKey Io::KeyboardActionMapping::GetGamepadKey(InputAction action) const {
    return gamepadKeyMap.find(action)->second;
}

KeyToggleType Io::KeyboardActionMapping::GetToggleType(InputAction action) const {
    return keyToggleMap.find(action)->second;
}

// TODO(captainurist): maybe we need to split InputActions to sets by WindowType so guarantee of only one InputAction per key is restored.
bool Io::KeyboardActionMapping::IsKeyMatchAction(InputAction action, PlatformKey key) const {
    if (action == Io::InputAction::Invalid)
        return false;

    if (actionKeyMap.find(action)->second == key)
        return true;

    if (gamepadKeyMap.find(action)->second == key)
        return true;

    return false;
}

//----- (00459C8D) --------------------------------------------------------
Io::KeyboardActionMapping::KeyboardActionMapping(std::shared_ptr<GameConfig> config) {
    this->config = config;

    SetDefaultMapping();
    ReadMappings();
}

//----- (00459CC4) --------------------------------------------------------
void Io::KeyboardActionMapping::SetDefaultMapping() {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        //MapKey(keyMappingParams[i].m_cmdId, keyMappingParams[i].m_key, keyMappingParams[i].m_toggType);
        MapKey(keyMappingParams[i].m_cmdId, ConfigDefaultKey(keyMappingParams[i].m_cmdId), keyMappingParams[i].m_toggType);
        // MapKey(keyMappingParams[i].m_cmdId, ConfigDefaultGamepadKey(keyMappingParams[i].m_cmdId));
    }
}

//----- (00459FFC) --------------------------------------------------------
void Io::KeyboardActionMapping::ReadMappings() {
    char str[32];

    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        //GameKey commandDefaultKeyCode = keyMappingParams[i].m_key;
        InputAction commandId = keyMappingParams[i].m_cmdId;
        KeyToggleType toggType = keyMappingParams[i].m_toggType;

        //GameKey parsedKey = GameKey::None;
        //if (strcmp(str, "DEFAULT") != 0 && TryParseDisplayName(str, &parsedKey))
        //    MapKey(commandId, parsedKey);
        //else
        //    MapKey(commandId, commandDefaultKeyCode);
        PlatformKey key = ConfigGetKey(commandId);
        if (key != PlatformKey::KEY_NONE)
            MapKey(commandId, key);
        else
            MapKey(commandId, ConfigDefaultKey(commandId));

        PlatformKey gamepadKey = ConfigGetGamepadKey(commandId);
        if (gamepadKey != PlatformKey::KEY_NONE)
            MapGamepadKey(commandId, gamepadKey);
        else
            MapGamepadKey(commandId, ConfigDefaultGamepadKey(commandId));

        keyToggleMap[commandId] = toggType;
    }
}

//----- (0045A960) --------------------------------------------------------
void Io::KeyboardActionMapping::StoreMappings() {
    for (size_t i = 0; i < keyMappingParams.size(); i++) {
        //std::string display_name = GetDisplayName(GetKey(keyMappingParams[i].m_cmdId));
        //OS_SetAppString(keyMappingParams[i].m_keyName.c_str(), display_name.c_str());
        ConfigSetKey(keyMappingParams[i].m_cmdId, GetKey(keyMappingParams[i].m_cmdId));
    }
}

KeyToggleType GetToggleType(InputAction action) {
    // TODO(captainurist): TOGGLE_* values in the table above are not respected, instead toggle values from this function are used.

    if (action == Io::InputAction::StrafeLeft || action == Io::InputAction::StrafeRight
        || action == Io::InputAction::FlyUp || action == Io::InputAction::FlyDown) {
        return KeyToggleType::TOGGLE_Continuously;
    }

    if (action == Io::InputAction::Attack || action == Io::InputAction::CastReady)
        return KeyToggleType::TOGGLE_DelayContinuous;

    if (action > Io::InputAction::TurnRight)
        return KeyToggleType::TOGGLE_OneTimePress;
    else
        return KeyToggleType::TOGGLE_Continuously;
}

GameConfig::Key *Io::KeyboardActionMapping::InputActionToConfigKey(InputAction action) {
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
        case(InputAction::ToggleMouseLook): return &config->keybindings.ToggleMouseLook;
        default: break;
    }

    return nullptr;
}

PlatformKey Io::KeyboardActionMapping::ConfigDefaultKey(InputAction action) {
    ConfigEntry<PlatformKey> *val = InputActionToConfigKey(action);
    return val ? val->defaultValue() : PlatformKey::KEY_NONE;
}

PlatformKey Io::KeyboardActionMapping::ConfigGetKey(InputAction action) {
    ConfigEntry<PlatformKey> *val = InputActionToConfigKey(action);
    return val ? val->value() : PlatformKey::KEY_NONE;
}

void Io::KeyboardActionMapping::ConfigSetKey(InputAction action, PlatformKey key) {
    if (ConfigEntry<PlatformKey> *val = InputActionToConfigKey(action))
        val->setValue(key);
}

PlatformKey Io::KeyboardActionMapping::ConfigDefaultGamepadKey(InputAction action) {
    GameConfig::Key *val = InputActionToConfigGamepadKey(action);
    return val ? val->defaultValue() : PlatformKey::KEY_NONE;
}

GameConfig::Key *Io::KeyboardActionMapping::InputActionToConfigGamepadKey(InputAction action) {
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

PlatformKey Io::KeyboardActionMapping::ConfigGetGamepadKey(InputAction action) {
    GameConfig::Key *val = InputActionToConfigGamepadKey(action);
    return val ? val->value() : PlatformKey::KEY_NONE;
}

void Io::KeyboardActionMapping::ConfigSetGamepadKey(InputAction action, PlatformKey key) {
    if (GameConfig::Key *val = InputActionToConfigGamepadKey(action))
        val->setValue(key);
}
