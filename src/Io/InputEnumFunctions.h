#pragma once

#include <string>

#include "Utility/Segment.h"

#include "InputEnums.h"

inline Segment<InputAction> allInputActions() {
    return {INPUT_ACTION_FIRST_VALID, INPUT_ACTION_LAST_VALID};
}

inline Segment<InputAction> allConfigurableInputActions() {
    return {INPUT_ACTION_FIRST_CONFIGURABLE, INPUT_ACTION_LAST_CONFIGURABLE};
}

std::string GetDisplayName(InputAction action);

InputActionTriggerMode triggerModeForInputAction(InputAction action);
