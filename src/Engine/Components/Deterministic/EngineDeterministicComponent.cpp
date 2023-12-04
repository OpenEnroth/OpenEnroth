#include "EngineDeterministicComponent.h"

#include <cassert>

#include "Engine/Components/Random/EngineRandomComponent.h"

#include "Library/Platform/Application/PlatformApplication.h"

EngineDeterministicComponent::EngineDeterministicComponent() = default;
EngineDeterministicComponent::~EngineDeterministicComponent() = default;

void EngineDeterministicComponent::restart(int frameTimeMs, RandomEngineType rngType) {
    assert(frameTimeMs >= 1);

    if (!_active)
        _oldRandomEngineType = component<EngineRandomComponent>()->type();

    _active = true;
    _tickCount = 0;
    _frameTimeMs = frameTimeMs;
    component<EngineRandomComponent>()->setType(rngType);
    component<EngineRandomComponent>()->seed(0);
}

void EngineDeterministicComponent::finish() {
    if (!isActive())
        return;

    component<EngineRandomComponent>()->setType(_oldRandomEngineType);
    _active = false;
}

int64_t EngineDeterministicComponent::tickCount() const {
    if (isActive()) {
        return _tickCount;
    } else {
        return ProxyPlatform::tickCount();
    }
}

void EngineDeterministicComponent::swapBuffers() {
    if (isActive())
        _tickCount += _frameTimeMs;

    // Tail calling is good practice - this way users can reason about the order of proxy execution.
    ProxyOpenGLContext::swapBuffers();
}

void EngineDeterministicComponent::removeNotify() {
    finish(); // Does nothing if already finished.
}
