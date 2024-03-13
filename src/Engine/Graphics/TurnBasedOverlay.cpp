#include "TurnBasedOverlay.h"

#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/TurnEngine/TurnEngine.h"

// Opening hand animation in vanilla was 320 ticks long (that's 2.5 seconds), but it was cut short and only the first
// 64 ticks were displayed. We fixed it, but then the animation ended up being way too slow, thus we've introduced the
// acceleration factor, resulting in an animation that's ~107 ticks long (5/6 of a second).
//
// Note that turn-based combat in vanilla starts with a 0.5s monster turn - supposedly to let the monsters end their
// current actions. The same timer was used for this turn and for the opening hand animation, and this is why the
// animation was cut short. We're using separate timers in OE.
static constexpr int TURN_BASED_INITIAL_ACCELERATION = 3;

TurnBasedOverlay turnBasedOverlay;

void TurnBasedOverlay::loadIcons() {
    _initialIconId = pIconsFrameTable->FindIcon("turnstart");
    _initialAnimationLength = pIconsFrameTable->GetIcon(_initialIconId)->GetAnimLength();
    _attackIconId = pIconsFrameTable->FindIcon("turnstop");
    _waitIconId = pIconsFrameTable->FindIcon("turnhour");

    for (size_t i = 0; i < _movementIconIds.size(); ++i)
        _movementIconIds[i] = pIconsFrameTable->FindIcon(fmt::format("turn{}", i));
}

void TurnBasedOverlay::reset() {
    _state = TURN_BASED_OVERLAY_NONE;
}

void TurnBasedOverlay::update(Duration dt, TurnEngineStep newStep) {
    if (newStep == TE_NONE) {
        _state = TURN_BASED_OVERLAY_NONE;
        return;
    }

    if (_state == TURN_BASED_OVERLAY_NONE) {
        assert(newStep == TE_WAIT);
        _state = TURN_BASED_OVERLAY_INITIAL;
        _currentTime = 0_ticks;
        return;
    }

    if (_state == TURN_BASED_OVERLAY_INITIAL) {
        _currentTime += dt * TURN_BASED_INITIAL_ACCELERATION;
        if (_currentTime < _initialAnimationLength)
            return;
    }

    switch (newStep) {
    case TE_WAIT:
        if (_state != TURN_BASED_OVERLAY_WAIT) {
            _currentTime = 0_ticks;
        } else {
            _currentTime += dt;
        }
        _state = TURN_BASED_OVERLAY_WAIT;
        break;
    case TE_MOVEMENT:
        _state = TURN_BASED_OVERLAY_MOVEMENT;
        break;
    case TE_ATTACK:
        _state = TURN_BASED_OVERLAY_ATTACK;
        break;
    default:
        assert(false);
        break;
    }
}

void TurnBasedOverlay::draw() {
    if (_state == TURN_BASED_OVERLAY_NONE)
        return;

    render->DrawTextureNew(394 / 640.0f, 288 / 480.0f, currentIcon()->GetTexture());
}

Icon *TurnBasedOverlay::currentIcon() const {
    switch (_state) {
    default:
        assert(false); // TURN_BASED_OVERLAY_NONE is expected to be checked up the stack.
        return nullptr;
    case TURN_BASED_OVERLAY_INITIAL:
        return pIconsFrameTable->GetFrame(_initialIconId, _currentTime);
    case TURN_BASED_OVERLAY_ATTACK:
        return pIconsFrameTable->GetFrame(_attackIconId, 0_ticks);
    case TURN_BASED_OVERLAY_MOVEMENT:
        return pIconsFrameTable->GetFrame(_movementIconIds[5 - pTurnEngine->uActionPointsLeft / 26], 0_ticks); // TODO(captainurist): get rid of this dependency.
    case TURN_BASED_OVERLAY_WAIT:
        return pIconsFrameTable->GetFrame(_waitIconId, _currentTime);
    }
}
