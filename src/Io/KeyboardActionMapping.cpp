#include "KeyboardActionMapping.h"

#include <string>
#include <map>
#include <memory>

#include "Engine/Graphics/Weather.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Utility/IndexedArray.h"

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

std::array<CKeyListElement, std::to_underlying(INPUT_ACTION_LAST_VALID) + 1> keyMappingParams = {
    CKeyListElement("KEY_FORWARD", PlatformKey::KEY_UP, INPUT_ACTION_MOVE_FORWARD, TOGGLE_CONTINUOUSLY),
    CKeyListElement("KEY_BACKWARD", PlatformKey::KEY_DOWN, INPUT_ACTION_MOVE_BACKWARDS, TOGGLE_CONTINUOUSLY),
    CKeyListElement("KEY_LEFT", PlatformKey::KEY_LEFT, INPUT_ACTION_TURN_LEFT, TOGGLE_CONTINUOUSLY),
    CKeyListElement("KEY_RIGHT", PlatformKey::KEY_RIGHT, INPUT_ACTION_TURN_RIGHT, TOGGLE_CONTINUOUSLY),
    CKeyListElement("KEY_ATTACK", PlatformKey::KEY_A, INPUT_ACTION_ATTACK, TOGGLE_CONTINUOUSLY_WITH_DELAY),
    CKeyListElement("KEY_CASTREADY", PlatformKey::KEY_S, INPUT_ACTION_QUICK_CAST, TOGGLE_CONTINUOUSLY_WITH_DELAY),
    CKeyListElement("KEY_YELL", PlatformKey::KEY_Y, INPUT_ACTION_YELL, TOGGLE_CONTINUOUSLY_WITH_DELAY),
    CKeyListElement("KEY_JUMP", PlatformKey::KEY_X, INPUT_ACTION_JUMP, TOGGLE_ONCE), // TODO: TOGGLE_Continuously
    CKeyListElement("KEY_COMBAT", PlatformKey::KEY_RETURN, INPUT_ACTION_TOGGLE_TURN_BASED, TOGGLE_ONCE),
    CKeyListElement("KEY_EVENTTRIGGER", PlatformKey::KEY_SPACE, INPUT_ACTION_TRIGGER, TOGGLE_ONCE), // TODO: TOGGLE_DelayContinuous
    CKeyListElement("KEY_CAST", PlatformKey::KEY_C, INPUT_ACTION_SPELLBOOK, TOGGLE_ONCE),
    CKeyListElement("KEY_PASS", PlatformKey::KEY_B, INPUT_ACTION_PASS, TOGGLE_CONTINUOUSLY_WITH_DELAY),
    CKeyListElement("KEY_CHARCYCLE", PlatformKey::KEY_TAB, INPUT_ACTION_NEXT_CHAR, TOGGLE_CONTINUOUSLY_WITH_DELAY),
    CKeyListElement("KEY_QUEST", PlatformKey::KEY_Q, INPUT_ACTION_OPEN_QUESTS, TOGGLE_ONCE),
    CKeyListElement("KEY_QUICKREF", PlatformKey::KEY_Z, INPUT_ACTION_OPEN_QUICK_REFERENCE, TOGGLE_ONCE),
    CKeyListElement("KEY_REST", PlatformKey::KEY_R, INPUT_ACTION_REST, TOGGLE_ONCE),
    CKeyListElement("KEY_TIMECAL", PlatformKey::KEY_T, INPUT_ACTION_OPEN_CALENDAR, TOGGLE_ONCE),
    CKeyListElement("KEY_AUTONOTES", PlatformKey::KEY_N, INPUT_ACTION_OPEN_AUTONOTES, TOGGLE_ONCE),
    CKeyListElement("KEY_MAPBOOK", PlatformKey::KEY_M, INPUT_ACTION_OPEN_MAP, TOGGLE_ONCE),
    CKeyListElement("KEY_LOOKUP", PlatformKey::KEY_PAGEDOWN, INPUT_ACTION_LOOK_UP, TOGGLE_ONCE), // TODO: TOGGLE_DelayContinuous
    CKeyListElement("KEY_LOOKDOWN", PlatformKey::KEY_DELETE, INPUT_ACTION_LOOK_DOWN, TOGGLE_ONCE), // TODO: TOGGLE_DelayContinuous
    CKeyListElement("KEY_CENTERVIEWPT", PlatformKey::KEY_END, INPUT_ACTION_CENTER_VIEW, TOGGLE_ONCE),
    CKeyListElement("KEY_ZOOMIN", PlatformKey::KEY_ADD, INPUT_ACTION_ZOOM_IN, TOGGLE_CONTINUOUSLY_WITH_DELAY),
    CKeyListElement("KEY_ZOOMOUT", PlatformKey::KEY_SUBTRACT, INPUT_ACTION_ZOOM_OUT, TOGGLE_CONTINUOUSLY_WITH_DELAY),
    CKeyListElement("KEY_FLYUP", PlatformKey::KEY_PAGEUP, INPUT_ACTION_FLY_UP, TOGGLE_CONTINUOUSLY),
    CKeyListElement("KEY_FLYDOWN", PlatformKey::KEY_INSERT, INPUT_ACTION_FLY_DOWN, TOGGLE_CONTINUOUSLY),
    CKeyListElement("KEY_LAND", PlatformKey::KEY_HOME, INPUT_ACTION_FLY_LAND, TOGGLE_ONCE),
    CKeyListElement("KEY_ALWAYSRUN", PlatformKey::KEY_U, INPUT_ACTION_TOGGLE_AUTO_RUN, TOGGLE_ONCE),
    CKeyListElement("KEY_STEPLEFT", PlatformKey::KEY_LEFTBRACKET, INPUT_ACTION_STRAFE_LEFT, TOGGLE_CONTINUOUSLY),
    CKeyListElement("KEY_STEPRIGHT", PlatformKey::KEY_RIGHTBRACKET, INPUT_ACTION_STRAFE_RIGHT, TOGGLE_CONTINUOUSLY),

    CKeyListElement("KEY_QUICKSAVE", PlatformKey::KEY_F5, INPUT_ACTION_QUICK_SAVE, TOGGLE_ONCE),
    CKeyListElement("KEY_QUICKLOAD", PlatformKey::KEY_F9, INPUT_ACTION_QUICK_LOAD, TOGGLE_ONCE),
    CKeyListElement("KEY_HISTORY", PlatformKey::KEY_H, INPUT_ACTION_OPEN_HISTORY, TOGGLE_ONCE),
    CKeyListElement("KEY_STATS", PlatformKey::KEY_C, INPUT_ACTION_OPEN_STATS, TOGGLE_ONCE),
    CKeyListElement("KEY_SKILLS", PlatformKey::KEY_S, INPUT_ACTION_OPEN_SKILLS, TOGGLE_ONCE),
    CKeyListElement("KEY_INVENTORY", PlatformKey::KEY_I, INPUT_ACTION_OPEN_INVENTORY, TOGGLE_ONCE),
    CKeyListElement("KEY_AWARDS", PlatformKey::KEY_A, INPUT_ACTION_OPEN_AWARDS, TOGGLE_ONCE),
    CKeyListElement("KEY_NEWGAME", PlatformKey::KEY_N, INPUT_ACTION_NEW_GAME, TOGGLE_ONCE),
    CKeyListElement("KEY_SAVEGAME", PlatformKey::KEY_S, INPUT_ACTION_SAVE_GAME, TOGGLE_ONCE),
    CKeyListElement("KEY_LOADGAME", PlatformKey::KEY_L, INPUT_ACTION_LOAD_GAME, TOGGLE_ONCE),
    CKeyListElement("KEY_EXITGAME", PlatformKey::KEY_Q, INPUT_ACTION_EXIT_GAME, TOGGLE_ONCE),
    CKeyListElement("KEY_RETURNTOGAME", PlatformKey::KEY_R, INPUT_ACTION_BACK_TO_GAME, TOGGLE_ONCE),
    CKeyListElement("KEY_CONTROLS", PlatformKey::KEY_C, INPUT_ACTION_OPEN_CONTROLS, TOGGLE_ONCE),
    CKeyListElement("KEY_OPTIONS", PlatformKey::KEY_O, INPUT_ACTION_OPEN_OPTIONS, TOGGLE_ONCE),
    CKeyListElement("KEY_CREDITS", PlatformKey::KEY_C, INPUT_ACTION_SHOW_CREDITS, TOGGLE_ONCE),
    CKeyListElement("KEY_CLEAR", PlatformKey::KEY_C, INPUT_ACTION_PARTY_CREATION_CLEAR, TOGGLE_ONCE),
    CKeyListElement("KEY_RETURN", PlatformKey::KEY_RETURN, INPUT_ACTION_PARTY_CREATION_DONE, TOGGLE_ONCE),
    CKeyListElement("KEY_SUBTRACT", PlatformKey::KEY_SUBTRACT, INPUT_ACTION_PARTY_CREATION_DEC, TOGGLE_ONCE),
    CKeyListElement("KEY_ADD", PlatformKey::KEY_ADD, INPUT_ACTION_PARTY_CREATION_INC, TOGGLE_ONCE),
    CKeyListElement("KEY_YES", PlatformKey::KEY_Y, INPUT_ACTION_TRANSITION_YES, TOGGLE_ONCE),
    CKeyListElement("KEY_NO", PlatformKey::KEY_N, INPUT_ACTION_TRANSITION_NO, TOGGLE_ONCE),
    CKeyListElement("KEY_REST8HOURS", PlatformKey::KEY_R, INPUT_ACTION_REST_HEAL, TOGGLE_ONCE),
    CKeyListElement("KEY_WAITTILLDAWN", PlatformKey::KEY_D, INPUT_ACTION_REST_WAIT_TILL_DAWN, TOGGLE_ONCE),
    CKeyListElement("KEY_WAITHOUR", PlatformKey::KEY_H, INPUT_ACTION_REST_WAIT_1_HOUR, TOGGLE_ONCE),
    CKeyListElement("KEY_WAIT5MINUTES", PlatformKey::KEY_M, INPUT_ACTION_REST_WAIT_5_MINUTES, TOGGLE_ONCE),
    CKeyListElement("KEY_SCREENSHOT", PlatformKey::KEY_F2, INPUT_ACTION_TAKE_SCREENSHOT, TOGGLE_ONCE),
    CKeyListElement("KEY_CONSOLE", PlatformKey::KEY_TILDE, INPUT_ACTION_OPEN_CONSOLE, TOGGLE_ONCE),
    CKeyListElement("KEY_SELECTCHAR1", PlatformKey::KEY_DIGIT_1, INPUT_ACTION_SELECT_CHAR_1, TOGGLE_ONCE),
    CKeyListElement("KEY_SELECTCHAR2", PlatformKey::KEY_DIGIT_2, INPUT_ACTION_SELECT_CHAR_2, TOGGLE_ONCE),
    CKeyListElement("KEY_SELECTCHAR3", PlatformKey::KEY_DIGIT_3, INPUT_ACTION_SELECT_CHAR_3, TOGGLE_ONCE),
    CKeyListElement("KEY_SELECTCHAR4", PlatformKey::KEY_DIGIT_4, INPUT_ACTION_SELECT_CHAR_4, TOGGLE_ONCE),
    CKeyListElement("KEY_SELECTNPC1", PlatformKey::KEY_DIGIT_5, INPUT_ACTION_SELECT_NPC_1, TOGGLE_ONCE),
    CKeyListElement("KEY_SELECTNPC2", PlatformKey::KEY_DIGIT_6, INPUT_ACTION_SELECT_NPC_2, TOGGLE_ONCE),
    CKeyListElement("KEY_DIALOGUP", PlatformKey::KEY_UP, INPUT_ACTION_DIALOG_UP, TOGGLE_ONCE),
    CKeyListElement("KEY_DIALOGDOWN", PlatformKey::KEY_DOWN, INPUT_ACTION_DIALOG_DOWN, TOGGLE_ONCE),
    CKeyListElement("KEY_DIALOGLEFT", PlatformKey::KEY_LEFT, INPUT_ACTION_DIALOG_LEFT, TOGGLE_ONCE),
    CKeyListElement("KEY_DIALOGRIGHT", PlatformKey::KEY_RIGHT, INPUT_ACTION_DIALOG_RIGHT, TOGGLE_ONCE),
    CKeyListElement("KEY_DIALOGSELECT", PlatformKey::KEY_PAGEDOWN, INPUT_ACTION_DIALOG_PRESS, TOGGLE_ONCE),
    CKeyListElement("KEY_ESCAPE", PlatformKey::KEY_ESCAPE, INPUT_ACTION_ESCAPE, TOGGLE_ONCE),
    CKeyListElement("KEY_TOGGLEMOUSELOOK", PlatformKey::KEY_F10, INPUT_ACTION_TOGGLE_MOUSE_LOOK, TOGGLE_ONCE),
    CKeyListElement("KEY_TOGGLEWINDOWMODE", PlatformKey::KEY_F11, INPUT_ACTION_TOGGLE_WINDOW_MODE, TOGGLE_CONTINUOUSLY),
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
    if (action == INPUT_ACTION_INVALID)
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

GameConfig::Key *Io::KeyboardActionMapping::InputActionToConfigKey(InputAction action) {
    switch (action) {
        case(INPUT_ACTION_MOVE_FORWARD): return &config->keybindings.Forward;
        case(INPUT_ACTION_MOVE_BACKWARDS): return &config->keybindings.Backward;
        case(INPUT_ACTION_TURN_LEFT): return &config->keybindings.Left;
        case(INPUT_ACTION_TURN_RIGHT): return &config->keybindings.Right;
        case(INPUT_ACTION_ATTACK): return &config->keybindings.Attack;
        case(INPUT_ACTION_QUICK_CAST): return &config->keybindings.CastReady;
        case(INPUT_ACTION_YELL): return &config->keybindings.Yell;
        case(INPUT_ACTION_JUMP): return &config->keybindings.Jump;
        case(INPUT_ACTION_TOGGLE_TURN_BASED): return &config->keybindings.Combat;
        case(INPUT_ACTION_TRIGGER): return &config->keybindings.EventTrigger;
        case(INPUT_ACTION_SPELLBOOK): return &config->keybindings.Cast;
        case(INPUT_ACTION_PASS): return &config->keybindings.Pass;
        case(INPUT_ACTION_NEXT_CHAR): return &config->keybindings.CharCycle;
        case(INPUT_ACTION_OPEN_QUESTS): return &config->keybindings.Quest;
        case(INPUT_ACTION_OPEN_QUICK_REFERENCE): return &config->keybindings.QuickReference;
        case(INPUT_ACTION_REST): return &config->keybindings.Rest;
        case(INPUT_ACTION_OPEN_CALENDAR): return &config->keybindings.TimeCalendar;
        case(INPUT_ACTION_OPEN_AUTONOTES): return &config->keybindings.AutoNotes;
        case(INPUT_ACTION_OPEN_MAP): return &config->keybindings.MapBook;
        case(INPUT_ACTION_LOOK_UP): return &config->keybindings.LookUp;
        case(INPUT_ACTION_LOOK_DOWN): return &config->keybindings.LookDown;
        case(INPUT_ACTION_CENTER_VIEW): return &config->keybindings.CenterView;
        case(INPUT_ACTION_ZOOM_IN): return &config->keybindings.ZoomIn;
        case(INPUT_ACTION_ZOOM_OUT): return &config->keybindings.ZoomOut;
        case(INPUT_ACTION_FLY_UP): return &config->keybindings.FlyUp;
        case(INPUT_ACTION_FLY_DOWN): return &config->keybindings.FlyDown;
        case(INPUT_ACTION_FLY_LAND): return &config->keybindings.Land;
        case(INPUT_ACTION_TOGGLE_AUTO_RUN): return &config->keybindings.AlwaysRun;
        case(INPUT_ACTION_STRAFE_LEFT): return &config->keybindings.StepLeft;
        case(INPUT_ACTION_STRAFE_RIGHT): return &config->keybindings.StepRight;
        case(INPUT_ACTION_QUICK_SAVE): return &config->keybindings.QuickSave;
        case(INPUT_ACTION_QUICK_LOAD): return &config->keybindings.QuickLoad;
        case(INPUT_ACTION_OPEN_HISTORY): return &config->keybindings.History;
        case(INPUT_ACTION_OPEN_STATS): return &config->keybindings.Stats;
        case(INPUT_ACTION_OPEN_SKILLS): return &config->keybindings.Skills;
        case(INPUT_ACTION_OPEN_INVENTORY): return &config->keybindings.Inventory;
        case(INPUT_ACTION_OPEN_AWARDS): return &config->keybindings.Awards;
        case(INPUT_ACTION_NEW_GAME): return &config->keybindings.NewGame;
        case(INPUT_ACTION_SAVE_GAME): return &config->keybindings.SaveGame;
        case(INPUT_ACTION_LOAD_GAME): return &config->keybindings.LoadGame;
        case(INPUT_ACTION_EXIT_GAME): return &config->keybindings.ExitGame;
        case(INPUT_ACTION_BACK_TO_GAME): return &config->keybindings.ReturnToGame;
        case(INPUT_ACTION_OPEN_CONTROLS): return &config->keybindings.Controls;
        case(INPUT_ACTION_OPEN_OPTIONS): return &config->keybindings.Options;
        case(INPUT_ACTION_SHOW_CREDITS): return &config->keybindings.Credits;
        case(INPUT_ACTION_PARTY_CREATION_CLEAR): return &config->keybindings.Clear;
        case(INPUT_ACTION_PARTY_CREATION_DONE): return &config->keybindings.Return;
        case(INPUT_ACTION_PARTY_CREATION_DEC): return &config->keybindings.Minus;
        case(INPUT_ACTION_PARTY_CREATION_INC): return &config->keybindings.Plus;
        case(INPUT_ACTION_TRANSITION_YES): return &config->keybindings.Yes;
        case(INPUT_ACTION_TRANSITION_NO): return &config->keybindings.No;
        case(INPUT_ACTION_REST_HEAL): return &config->keybindings.Rest8Hours;
        case(INPUT_ACTION_REST_WAIT_TILL_DAWN): return &config->keybindings.WaitTillDawn;
        case(INPUT_ACTION_REST_WAIT_1_HOUR): return &config->keybindings.WaitHour;
        case(INPUT_ACTION_REST_WAIT_5_MINUTES): return &config->keybindings.Wait5Minutes;
        case(INPUT_ACTION_TAKE_SCREENSHOT): return &config->keybindings.Screenshot;
        case(INPUT_ACTION_OPEN_CONSOLE): return &config->keybindings.Console;
        case(INPUT_ACTION_SELECT_CHAR_1): return &config->keybindings.SelectChar1;
        case(INPUT_ACTION_SELECT_CHAR_2): return &config->keybindings.SelectChar2;
        case(INPUT_ACTION_SELECT_CHAR_3): return &config->keybindings.SelectChar3;
        case(INPUT_ACTION_SELECT_CHAR_4): return &config->keybindings.SelectChar4;
        case(INPUT_ACTION_SELECT_NPC_1): return &config->keybindings.SelectNPC1;
        case(INPUT_ACTION_SELECT_NPC_2): return &config->keybindings.SelectNPC2;
        case(INPUT_ACTION_DIALOG_UP): return &config->keybindings.DialogUp;
        case(INPUT_ACTION_DIALOG_DOWN): return &config->keybindings.DialogDown;
        case(INPUT_ACTION_DIALOG_LEFT): return &config->keybindings.DialogLeft;
        case(INPUT_ACTION_DIALOG_RIGHT): return &config->keybindings.DialogRight;
        case(INPUT_ACTION_DIALOG_PRESS): return &config->keybindings.DialogSelect;
        case(INPUT_ACTION_ESCAPE): return &config->keybindings.Escape;
        case(INPUT_ACTION_TOGGLE_MOUSE_LOOK): return &config->keybindings.ToggleMouseLook;
        case(INPUT_ACTION_TOGGLE_WINDOW_MODE): return &config->keybindings.ToggleWindowMode;
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
        case(INPUT_ACTION_MOVE_FORWARD): return &config->gamepad.Forward;
        case(INPUT_ACTION_MOVE_BACKWARDS): return &config->gamepad.Backward;
        case(INPUT_ACTION_TURN_LEFT): return &config->gamepad.Left;
        case(INPUT_ACTION_TURN_RIGHT): return &config->gamepad.Right;
        case(INPUT_ACTION_ATTACK): return &config->gamepad.Attack;
        case(INPUT_ACTION_QUICK_CAST): return &config->gamepad.CastReady;
        case(INPUT_ACTION_YELL): return &config->gamepad.Yell;
        case(INPUT_ACTION_JUMP): return &config->gamepad.Jump;
        case(INPUT_ACTION_TOGGLE_TURN_BASED): return &config->gamepad.Combat;
        case(INPUT_ACTION_TRIGGER): return &config->gamepad.EventTrigger;
        case(INPUT_ACTION_SPELLBOOK): return &config->gamepad.Cast;
        case(INPUT_ACTION_PASS): return &config->gamepad.Pass;
        case(INPUT_ACTION_NEXT_CHAR): return &config->gamepad.CharCycle;
        case(INPUT_ACTION_OPEN_QUESTS): return &config->gamepad.Quest;
        case(INPUT_ACTION_OPEN_QUICK_REFERENCE): return &config->gamepad.QuickReference;
        case(INPUT_ACTION_REST): return &config->gamepad.Rest;
        case(INPUT_ACTION_OPEN_CALENDAR): return &config->gamepad.TimeCalendar;
        case(INPUT_ACTION_OPEN_AUTONOTES): return &config->gamepad.AutoNotes;
        case(INPUT_ACTION_OPEN_MAP): return &config->gamepad.MapBook;
        case(INPUT_ACTION_LOOK_UP): return &config->gamepad.LookUp;
        case(INPUT_ACTION_LOOK_DOWN): return &config->gamepad.LookDown;
        case(INPUT_ACTION_CENTER_VIEW): return &config->gamepad.CenterView;
        case(INPUT_ACTION_ZOOM_IN): return &config->gamepad.ZoomIn;
        case(INPUT_ACTION_ZOOM_OUT): return &config->gamepad.ZoomOut;
        case(INPUT_ACTION_FLY_UP): return &config->gamepad.FlyUp;
        case(INPUT_ACTION_FLY_DOWN): return &config->gamepad.FlyDown;
        case(INPUT_ACTION_FLY_LAND): return &config->gamepad.Land;
        case(INPUT_ACTION_TOGGLE_AUTO_RUN): return &config->gamepad.AlwaysRun;
        case(INPUT_ACTION_STRAFE_LEFT): return &config->gamepad.StepLeft;
        case(INPUT_ACTION_STRAFE_RIGHT): return &config->gamepad.StepRight;
        case(INPUT_ACTION_QUICK_SAVE): return &config->gamepad.QuickSave;
        case(INPUT_ACTION_QUICK_LOAD): return &config->gamepad.QuickLoad;
        case(INPUT_ACTION_OPEN_HISTORY): return &config->gamepad.History;
        case(INPUT_ACTION_OPEN_STATS): return &config->gamepad.Stats;
        case(INPUT_ACTION_OPEN_SKILLS): return &config->gamepad.Skills;
        case(INPUT_ACTION_OPEN_INVENTORY): return &config->gamepad.Inventory;
        case(INPUT_ACTION_OPEN_AWARDS): return &config->gamepad.Awards;
        case(INPUT_ACTION_NEW_GAME): return &config->gamepad.NewGame;
        case(INPUT_ACTION_SAVE_GAME): return &config->gamepad.SaveGame;
        case(INPUT_ACTION_LOAD_GAME): return &config->gamepad.LoadGame;
        case(INPUT_ACTION_EXIT_GAME): return &config->gamepad.ExitGame;
        case(INPUT_ACTION_BACK_TO_GAME): return &config->gamepad.ReturnToGame;
        case(INPUT_ACTION_OPEN_CONTROLS): return &config->gamepad.Controls;
        case(INPUT_ACTION_OPEN_OPTIONS): return &config->gamepad.Options;
        case(INPUT_ACTION_SHOW_CREDITS): return &config->gamepad.Credits;
        case(INPUT_ACTION_PARTY_CREATION_CLEAR): return &config->gamepad.Clear;
        case(INPUT_ACTION_PARTY_CREATION_DONE): return &config->gamepad.Return;
        case(INPUT_ACTION_PARTY_CREATION_DEC): return &config->gamepad.Minus;
        case(INPUT_ACTION_PARTY_CREATION_INC): return &config->gamepad.Plus;
        case(INPUT_ACTION_TRANSITION_YES): return &config->gamepad.Yes;
        case(INPUT_ACTION_TRANSITION_NO): return &config->gamepad.No;
        case(INPUT_ACTION_REST_HEAL): return &config->gamepad.Rest8Hours;
        case(INPUT_ACTION_REST_WAIT_TILL_DAWN): return &config->gamepad.WaitTillDawn;
        case(INPUT_ACTION_REST_WAIT_1_HOUR): return &config->gamepad.WaitHour;
        case(INPUT_ACTION_REST_WAIT_5_MINUTES): return &config->gamepad.Wait5Minutes;
        case(INPUT_ACTION_TAKE_SCREENSHOT): return &config->gamepad.Screenshot;
        case(INPUT_ACTION_OPEN_CONSOLE): return &config->gamepad.Console;
        case(INPUT_ACTION_SELECT_CHAR_1): return &config->gamepad.SelectChar1;
        case(INPUT_ACTION_SELECT_CHAR_2): return &config->gamepad.SelectChar2;
        case(INPUT_ACTION_SELECT_CHAR_3): return &config->gamepad.SelectChar3;
        case(INPUT_ACTION_SELECT_CHAR_4): return &config->gamepad.SelectChar4;
        case(INPUT_ACTION_SELECT_NPC_1): return &config->gamepad.SelectNPC1;
        case(INPUT_ACTION_SELECT_NPC_2): return &config->gamepad.SelectNPC2;
        case(INPUT_ACTION_DIALOG_UP): return &config->gamepad.DialogUp;
        case(INPUT_ACTION_DIALOG_DOWN): return &config->gamepad.DialogDown;
        case(INPUT_ACTION_DIALOG_LEFT): return &config->gamepad.DialogLeft;
        case(INPUT_ACTION_DIALOG_RIGHT): return &config->gamepad.DialogRight;
        case(INPUT_ACTION_DIALOG_PRESS): return &config->gamepad.DialogSelect;
        case(INPUT_ACTION_ESCAPE): return &config->gamepad.Escape;
        case(INPUT_ACTION_TOGGLE_WINDOW_MODE): return &config->gamepad.ToggleWindowMode;
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
