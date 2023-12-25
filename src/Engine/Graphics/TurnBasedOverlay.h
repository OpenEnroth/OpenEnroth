#pragma once

#include <array>

#include "Engine/Time/Duration.h"
#include "Engine/TurnEngine/TurnEngineEnums.h"

class Icon;

enum class TurnBasedOverlayState {
    TURN_BASED_OVERLAY_NONE,
    TURN_BASED_OVERLAY_INITIAL,
    TURN_BASED_OVERLAY_ATTACK,
    TURN_BASED_OVERLAY_MOVEMENT,
    TURN_BASED_OVERLAY_WAIT,
};
using enum TurnBasedOverlayState;

class TurnBasedOverlay {
 public:
    constexpr TurnBasedOverlay() = default;

    void loadIcons();

    void reset();

    void update(Duration dt, TurnEngineStep newStep);

    /**
     * @offset 0x00441964
     */
    void draw();

 private:
    Icon *currentIcon() const;

 private:
    TurnBasedOverlayState _state = TURN_BASED_OVERLAY_NONE;
    Duration _currentTime;
    Duration _currentEnd;

    int _initialIconId = 0; // Opening hand animation.
    int _attackIconId = 0; // Open hand.
    std::array<int, 5> _movementIconIds = {{}}; // Fingers.
    int _waitIconId = 0; // Hourglass animation.
};

extern TurnBasedOverlay turnBasedOverlay;
