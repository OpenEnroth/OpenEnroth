#pragma once

#include <cstdint>

// TODO(captainurist): this should be in some other header probably.

enum class DoorState : uint16_t {
    DOOR_CLOSED = 0,
    DOOR_OPENING = 1,
    DOOR_OPEN = 2,
    DOOR_CLOSING = 3
};
using enum DoorState;

enum class DoorAction {
    DOOR_ACTION_CLOSE = 0, // If open or opening, transitions to closing.
    DOOR_ACTION_OPEN = 1, // If closed or closing, transitions to opening.
    DOOR_ACTION_TRIGGER = 2, // Only works on fully open / closed doors. Closes / opens the door.
};
using enum DoorAction;
