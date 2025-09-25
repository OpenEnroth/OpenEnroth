#pragma once

#include <string>
#include <string_view>

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Library/Serialization/SerializationFwd.h"

#include "Utility/Segment.h"

#include "InputEnums.h"

inline Segment<InputAction> allInputActions() {
    return {INPUT_ACTION_FIRST_VALID, INPUT_ACTION_LAST_VALID};
}

inline Segment<InputAction> allConfigurableInputActions() {
    return {INPUT_ACTION_FIRST_CONFIGURABLE, INPUT_ACTION_LAST_CONFIGURABLE};
}

std::string GetDisplayName(InputAction action);

KeyToggleType toggleTypeForInputAction(InputAction action);

// TODO(captainurist): find a better place for this code
MM_DECLARE_SERIALIZATION_FUNCTIONS(PlatformKey)

std::string GetDisplayName(PlatformKey key);
bool TryParseDisplayName(std::string_view displayName, PlatformKey *outKey);
