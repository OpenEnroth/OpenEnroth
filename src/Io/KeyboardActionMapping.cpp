#include "KeyboardActionMapping.h"

#include <unordered_set>
#include <memory>

#include "Utility/MapAccess.h"

#include "InputEnumFunctions.h"

std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping = nullptr;

extern std::unordered_set<InputAction> key_map_conflicted;  // 506E6C

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

Io::Keybindings Io::KeyboardActionMapping::currentKeybindings(KeybindingsQuery query) const {
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
