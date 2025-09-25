#pragma once

#include <unordered_map>
#include <memory>

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Io/InputEnums.h"

#include "Application/GameConfig.h"

enum class KeybindingsQuery {
    KEYBINDINGS_ALL = 0,
    KEYBINDINGS_CONFIGURABLE = 1
};
using enum KeybindingsQuery;

namespace Io {
// TODO(captainurist): #enum
enum class TextInputType {
    None = 0,
    Text = 1,
    Number = 2,
};

using Keybindings = std::unordered_map<InputAction, PlatformKey>;

class KeyboardActionMapping {
 public:
    explicit KeyboardActionMapping(std::shared_ptr<GameConfig> config);

    [[nodiscard]] PlatformKey keyFor(InputAction action) const;
    [[nodiscard]] PlatformKey gamepadKeyFor(InputAction action) const;

    [[nodiscard]] bool isBound(InputAction action, PlatformKey key) const;

    [[nodiscard]] Keybindings currentKeybindings(KeybindingsQuery query) const;
    [[nodiscard]] Keybindings defaultKeybindings(KeybindingsQuery query) const;
    void applyKeybindings(const Keybindings &keybindings);

 private:
    std::shared_ptr<GameConfig> _config;
    std::unordered_map<InputAction, KeyConfigEntry *> _keyboardEntryByInputAction;
    std::unordered_map<InputAction, KeyConfigEntry *> _gamepadEntryByInputAction;
};
}  // namespace Io

extern std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping;
