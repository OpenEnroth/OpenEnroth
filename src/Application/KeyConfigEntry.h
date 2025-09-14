#pragma once

#include "Io/InputAction.h"

#include "Library/Config/ConfigEntry.h"
#include "Library/Platform/Interface/PlatformEnums.h"

class KeyConfigEntry : public ConfigEntry<PlatformKey> {
    using base_type = ConfigEntry<PlatformKey>;
 public:
    KeyConfigEntry(ConfigSection *section, InputAction action, std::string_view name, PlatformKey defaultValue, std::string_view description):
        base_type(section, name, defaultValue, description),
        _inputAction(action)
    {}

    InputAction inputAction() const {
        return _inputAction;
    }

 private:
    InputAction _inputAction;
};
