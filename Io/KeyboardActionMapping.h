#pragma once

#include <map>
#include <memory>
#include <string>

#include "Io/GameKey.h"
#include "Io/InputAction.h"

namespace Io {
    enum class KeyToggleType {
        TOGGLE_Continuously = 0,
        TOGGLE_OneTimePress = 1,
    };

    enum class TextInputType {
        None = 0,
        Text = 1,
        Number = 2,
    };

    struct KeyboardActionMapping {
        KeyboardActionMapping();

        void MapKey(InputAction action, GameKey key);
        void MapKey(InputAction action, GameKey key, KeyToggleType type);
        GameKey MapDefaultKey(InputAction action);

        GameKey GetKey(InputAction action) const;
        KeyToggleType GetToggleType(InputAction action) const;

        void ReadMappings();
        void StoreMappings();
        void SetDefaultMapping();

     private:
        std::map<InputAction, GameKey> actionKeyMap;
        std::map<InputAction, KeyToggleType> keyToggleMap;
    };
}  // namespace Io

void OnPressSpace();
Io::KeyToggleType GetToggleType(Io::InputAction action);

extern std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping;
