#pragma once

#include <unordered_map>
#include <memory>
#include <string>

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Io/InputAction.h"

#include "Application/GameConfig.h"

enum class KeyToggleType {
    TOGGLE_CONTINUOUSLY = 0, // Toggle every frame as long as the key as pressed.
    TOGGLE_ONCE = 1, // Toggle once per keypress (but not more than once per frame).
    TOGGLE_CONTINUOUSLY_WITH_DELAY = 2, // Toggle once, then toggle continuously after a 500ms delay.
};
using enum KeyToggleType;

enum class KeybindingsQuery {
    KEYBINDINGS_ALL = 0,
    KEYBINDINGS_CONFIGURABLE = 1
};
using enum KeybindingsQuery;

namespace Io {
enum class TextInputType {
    None = 0,
    Text = 1,
    Number = 2,
};

using Keybindings = std::unordered_map<InputAction, PlatformKey>;

class KeyboardActionMapping {
 public:
    explicit KeyboardActionMapping(std::shared_ptr<GameConfig> config);

    PlatformKey keyFor(InputAction action) const;
    PlatformKey gamepadKeyFor(InputAction action) const;
    KeyToggleType toggleTypeFor(InputAction action) const;

    bool isBound(InputAction action, PlatformKey key) const;

    Keybindings keybindings(KeybindingsQuery query) const;
    Keybindings defaultKeybindings(KeybindingsQuery query) const;
    void applyKeybindings(const Keybindings &keybindings);

 private:
    std::shared_ptr<GameConfig> _config;
    std::unordered_map<InputAction, KeyConfigEntry *> _keyboardEntryByInputAction;
    std::unordered_map<InputAction, KeyConfigEntry *> _gamepadEntryByInputAction;
};
}  // namespace Io

extern std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping;
