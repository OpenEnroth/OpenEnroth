#pragma once

#include <array>

#include "Engine/Time/Duration.h"
#include "Engine/TurnEngine/TurnEngineEnums.h"

class GraphicsImage;

enum class TurnBasedOverlayState {
    TURN_BASED_OVERLAY_NONE, // No overlay.
    TURN_BASED_OVERLAY_INITIAL, // Initial phase - opening hand animation.
    TURN_BASED_OVERLAY_ATTACK, // Attack phase - open hand.
    TURN_BASED_OVERLAY_MOVEMENT, // Party movement - fist turning into an open hand, one finger at a time.
    TURN_BASED_OVERLAY_WAIT, // Monster turn - animated hourglass.
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
    GraphicsImage *currentIcon() const;

 private:
    TurnBasedOverlayState _state = TURN_BASED_OVERLAY_NONE;
    Duration _currentTime; // Current animation progress.

    int _initialIconId = 0; // Opening hand animation.
    Duration _initialAnimationLength; // Duration of the opening hand animation.
    int _attackIconId = 0; // Open hand.
    std::array<int, 5> _movementIconIds = {{}}; // Fingers.
    int _waitIconId = 0; // Hourglass animation.
};

extern TurnBasedOverlay turnBasedOverlay;
