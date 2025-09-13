#include "KeyboardActionMapping.h"

#include <string>
#include <unordered_set>
#include <memory>

#include "Utility/IndexedArray.h"
#include "Utility/MapAccess.h"

std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping = nullptr;

extern std::unordered_set<InputAction> key_map_conflicted;  // 506E6C

static constexpr IndexedArray<KeyToggleType, INPUT_ACTION_FIRST_VALID, INPUT_ACTION_LAST_VALID> keyToggleMap = {
    {INPUT_ACTION_MOVE_FORWARD,         TOGGLE_CONTINUOUSLY},
    {INPUT_ACTION_MOVE_BACKWARDS,       TOGGLE_CONTINUOUSLY},
    {INPUT_ACTION_TURN_LEFT,            TOGGLE_CONTINUOUSLY},
    {INPUT_ACTION_TURN_RIGHT,           TOGGLE_CONTINUOUSLY},
    {INPUT_ACTION_ATTACK,               TOGGLE_CONTINUOUSLY_WITH_DELAY},
    {INPUT_ACTION_QUICK_CAST,           TOGGLE_CONTINUOUSLY_WITH_DELAY},
    {INPUT_ACTION_YELL,                 TOGGLE_CONTINUOUSLY_WITH_DELAY},
    {INPUT_ACTION_JUMP,                 TOGGLE_ONCE}, // TODO: TOGGLE_Continuously
    {INPUT_ACTION_TOGGLE_TURN_BASED,    TOGGLE_ONCE},
    {INPUT_ACTION_INTERACT,             TOGGLE_ONCE}, // TODO: TOGGLE_DelayContinuous
    {INPUT_ACTION_OPEN_SPELLBOOK,       TOGGLE_ONCE},
    {INPUT_ACTION_PASS,                 TOGGLE_CONTINUOUSLY_WITH_DELAY},
    {INPUT_ACTION_NEXT_CHAR,            TOGGLE_CONTINUOUSLY_WITH_DELAY},
    {INPUT_ACTION_OPEN_QUESTS,          TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_QUICK_REFERENCE, TOGGLE_ONCE},
    {INPUT_ACTION_REST,                 TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_CALENDAR,        TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_AUTONOTES,       TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_MAP,             TOGGLE_ONCE},
    {INPUT_ACTION_LOOK_UP,              TOGGLE_ONCE}, // TODO: TOGGLE_DelayContinuous
    {INPUT_ACTION_LOOK_DOWN,            TOGGLE_ONCE}, // TODO: TOGGLE_DelayContinuous
    {INPUT_ACTION_CENTER_VIEW,          TOGGLE_ONCE},
    {INPUT_ACTION_ZOOM_IN,              TOGGLE_CONTINUOUSLY_WITH_DELAY},
    {INPUT_ACTION_ZOOM_OUT,             TOGGLE_CONTINUOUSLY_WITH_DELAY},
    {INPUT_ACTION_FLY_UP,               TOGGLE_CONTINUOUSLY},
    {INPUT_ACTION_FLY_DOWN,             TOGGLE_CONTINUOUSLY},
    {INPUT_ACTION_FLY_LAND,             TOGGLE_ONCE},
    {INPUT_ACTION_TOGGLE_ALWAYS_RUN,    TOGGLE_ONCE},
    {INPUT_ACTION_STRAFE_LEFT,          TOGGLE_CONTINUOUSLY},
    {INPUT_ACTION_STRAFE_RIGHT,         TOGGLE_CONTINUOUSLY},
    {INPUT_ACTION_QUICK_SAVE,           TOGGLE_ONCE},
    {INPUT_ACTION_QUICK_LOAD,           TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_HISTORY,         TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_STATS,           TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_SKILLS,          TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_INVENTORY,       TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_AWARDS,          TOGGLE_ONCE},
    {INPUT_ACTION_NEW_GAME,             TOGGLE_ONCE},
    {INPUT_ACTION_SAVE_GAME,            TOGGLE_ONCE},
    {INPUT_ACTION_LOAD_GAME,            TOGGLE_ONCE},
    {INPUT_ACTION_EXIT_GAME,            TOGGLE_ONCE},
    {INPUT_ACTION_BACK_TO_GAME,         TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_CONTROLS,        TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_OPTIONS,         TOGGLE_ONCE},
    {INPUT_ACTION_SHOW_CREDITS,         TOGGLE_ONCE},
    {INPUT_ACTION_PARTY_CREATION_CLEAR, TOGGLE_ONCE},
    {INPUT_ACTION_PARTY_CREATION_DONE,  TOGGLE_ONCE},
    {INPUT_ACTION_PARTY_CREATION_DEC,   TOGGLE_ONCE},
    {INPUT_ACTION_PARTY_CREATION_INC,   TOGGLE_ONCE},
    {INPUT_ACTION_TRANSITION_YES,       TOGGLE_ONCE},
    {INPUT_ACTION_TRANSITION_NO,        TOGGLE_ONCE},
    {INPUT_ACTION_REST_HEAL,            TOGGLE_ONCE},
    {INPUT_ACTION_REST_WAIT_TILL_DAWN,  TOGGLE_ONCE},
    {INPUT_ACTION_REST_WAIT_1_HOUR,     TOGGLE_ONCE},
    {INPUT_ACTION_REST_WAIT_5_MINUTES,  TOGGLE_ONCE},
    {INPUT_ACTION_TAKE_SCREENSHOT,      TOGGLE_ONCE},
    {INPUT_ACTION_OPEN_CONSOLE,         TOGGLE_ONCE},
    {INPUT_ACTION_SELECT_CHAR_1,        TOGGLE_ONCE},
    {INPUT_ACTION_SELECT_CHAR_2,        TOGGLE_ONCE},
    {INPUT_ACTION_SELECT_CHAR_3,        TOGGLE_ONCE},
    {INPUT_ACTION_SELECT_CHAR_4,        TOGGLE_ONCE},
    {INPUT_ACTION_SELECT_NPC_1,         TOGGLE_ONCE},
    {INPUT_ACTION_SELECT_NPC_2,         TOGGLE_ONCE},
    {INPUT_ACTION_DIALOG_UP,            TOGGLE_ONCE},
    {INPUT_ACTION_DIALOG_DOWN,          TOGGLE_ONCE},
    {INPUT_ACTION_DIALOG_LEFT,          TOGGLE_ONCE},
    {INPUT_ACTION_DIALOG_RIGHT,         TOGGLE_ONCE},
    {INPUT_ACTION_DIALOG_PRESS,         TOGGLE_ONCE},
    {INPUT_ACTION_ESCAPE,               TOGGLE_ONCE},
    {INPUT_ACTION_TOGGLE_MOUSE_LOOK,    TOGGLE_ONCE},
    {INPUT_ACTION_TOGGLE_WINDOW_MODE,   TOGGLE_CONTINUOUSLY},
};

//----- (00459C8D) --------------------------------------------------------
Io::KeyboardActionMapping::KeyboardActionMapping(std::shared_ptr<GameConfig> config) : _config(config) {
    auto fill = [] (const ConfigSection &section, auto *mapping) {
        for (AnyConfigEntry *anyEntry : section.entries())
            if (KeyConfigEntry *entry = dynamic_cast<KeyConfigEntry *>(anyEntry))
                (*mapping)[entry->inputAction()] = entry;
    };

    fill(config->keybindings, &_keyboardEntryByInputAction);
    fill(config->gamepad, &_gamepadEntryByInputAction);
}

PlatformKey Io::KeyboardActionMapping::keyFor(InputAction action) const {
    KeyConfigEntry *entry = valueOr(_keyboardEntryByInputAction, action, nullptr);
    return entry ? entry->value() : PlatformKey::KEY_NONE;
}

PlatformKey Io::KeyboardActionMapping::gamepadKeyFor(InputAction action) const {
    KeyConfigEntry *entry = valueOr(_gamepadEntryByInputAction, action, nullptr);
    return entry ? entry->value() : PlatformKey::KEY_NONE;
}

KeyToggleType Io::KeyboardActionMapping::toggleTypeFor(InputAction action) const {
    return keyToggleMap[action];
}

// TODO(captainurist): maybe we need to split InputActions to sets by WindowType so guarantee of only one InputAction per key is restored.
bool Io::KeyboardActionMapping::isBound(InputAction action, PlatformKey key) const {
    if (action == INPUT_ACTION_INVALID || key == PlatformKey::KEY_NONE)
        return false;
    if (KeyConfigEntry *entry = valueOr(_keyboardEntryByInputAction, action, nullptr); entry && entry->value() == key)
        return true;
    if (KeyConfigEntry *entry = valueOr(_gamepadEntryByInputAction, action, nullptr); entry && entry->value() == key)
        return true;
    return false;
}

Io::Keybindings Io::KeyboardActionMapping::keybindings(KeybindingsQuery query) const {
    Io::Keybindings result;
    for (const auto &[inputAction, configEntry] : _keyboardEntryByInputAction)
        if (query == KEYBINDINGS_ALL || (query == KEYBINDINGS_CONFIGURABLE && allConfigurableInputActions().contains(inputAction)))
            result.emplace(inputAction, configEntry->value());
    return result;
}

Io::Keybindings Io::KeyboardActionMapping::defaultKeybindings(KeybindingsQuery query) const {
    Io::Keybindings result;
    for (const auto &[inputAction, configEntry] : _keyboardEntryByInputAction)
        if (query == KEYBINDINGS_ALL || (query == KEYBINDINGS_CONFIGURABLE && allConfigurableInputActions().contains(inputAction)))
            result.emplace(inputAction, configEntry->defaultValue());
    return result;
}

void Io::KeyboardActionMapping::applyKeybindings(const Io::Keybindings &keybindings) {
    for (const auto &[inputAction, key] : keybindings)
        _keyboardEntryByInputAction[inputAction]->setValue(key);
}
