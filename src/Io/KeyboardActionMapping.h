#pragma once

#include <map>
#include <memory>
#include <string>

#include "Platform/PlatformEnums.h"
#include "Io/InputAction.h"

#include "Application/GameConfig.h"

namespace Io {
enum class KeyToggleType {
    TOGGLE_Continuously = 0,
    TOGGLE_OneTimePress = 1,
    TOGGLE_DelayContinuous = 2,
};

enum class TextInputType {
    None = 0,
    Text = 1,
    Number = 2,
};

class KeyboardActionMapping {
 public:
    explicit KeyboardActionMapping(std::shared_ptr<GameConfig> config);

    void MapKey(InputAction action, PlatformKey key);
    void MapKey(InputAction action, PlatformKey key, KeyToggleType type);
    void MapGamepadKey(InputAction action, PlatformKey key);
    PlatformKey MapDefaultKey(InputAction action);

    PlatformKey GetKey(InputAction action) const;
    PlatformKey GetGamepadKey(InputAction action) const;
    KeyToggleType GetToggleType(InputAction action) const;
    bool IsKeyMatchAction(InputAction action, PlatformKey key) const;

    GameConfig::Key *InputActionToConfigKey(InputAction action);
    PlatformKey ConfigDefaultKey(InputAction action);
    PlatformKey ConfigGetKey(InputAction action);
    void ConfigSetKey(InputAction action, PlatformKey key);

    GameConfig::Key *InputActionToConfigGamepadKey(InputAction action);
    PlatformKey ConfigDefaultGamepadKey(InputAction action);
    PlatformKey ConfigGetGamepadKey(InputAction action);
    void ConfigSetGamepadKey(InputAction action, PlatformKey key);

    void ReadMappings();
    void StoreMappings();
    void SetDefaultMapping();

 private:
    std::map<InputAction, PlatformKey> actionKeyMap;
    std::map<InputAction, PlatformKey> gamepadKeyMap;
    std::map<InputAction, KeyToggleType> keyToggleMap;
    std::shared_ptr<GameConfig> config = nullptr;
};
}  // namespace Io

Io::KeyToggleType GetToggleType(Io::InputAction action);

extern std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping;
