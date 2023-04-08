#include "EngineDeterministicComponent.h"

#include <cassert>
#include <utility>

#include "Library/Random/Random.h"
#include "Library/Random/NonRandomEngine.h"

EngineDeterministicComponent::EngineDeterministicComponent() = default;
EngineDeterministicComponent::~EngineDeterministicComponent() = default;

void EngineDeterministicComponent::enterDeterministicMode() {
    _deterministicCounter++;

    if (_deterministicCounter == 1) {
        _tickCount = 0;
        _oldRandomEngine = std::move(grng);
        grng = std::make_unique<NonRandomEngine>();
    } else {
        resetDeterministicState();
    }
}

void EngineDeterministicComponent::resetDeterministicState() {
    assert(_deterministicCounter > 0);

    _tickCount = 0;
    grng->seed(0); // Equivalent to just recreating a NonRandomEngine.
}

void EngineDeterministicComponent::leaveDeterministicMode() {
    assert(_deterministicCounter > 0);

    _deterministicCounter--;

    if (_deterministicCounter == 0)
        grng = std::move(_oldRandomEngine);
}

int64_t EngineDeterministicComponent::tickCount() const {
    if (_deterministicCounter > 0) {
        return _tickCount;
    } else {
        return ProxyPlatform::tickCount();
    }
}

void EngineDeterministicComponent::swapBuffers() {
    if (_deterministicCounter > 0)
        _tickCount += FRAME_TIME_MS;

    // Tail calling is good practice - this way users can reason about the order of proxy execution.
    ProxyOpenGLContext::swapBuffers();
}
