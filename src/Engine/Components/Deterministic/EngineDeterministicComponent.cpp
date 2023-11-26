#include "EngineDeterministicComponent.h"

#include <cassert>

#include "Engine/Components/Random/EngineRandomComponent.h"

#include "Library/Platform/Application/PlatformApplication.h"

EngineDeterministicComponent::EngineDeterministicComponent() = default;
EngineDeterministicComponent::~EngineDeterministicComponent() = default;

void EngineDeterministicComponent::restart(int frameTimeMs, RandomEngineType rngType) {
    assert(frameTimeMs >= 1 && frameTimeMs <= 1000);

    _active = true;
    _tickCount = 0;
    _frameTimeMs = frameTimeMs;
    application()->get<EngineRandomComponent>()->reset(rngType);
}

void EngineDeterministicComponent::finish() {
    if (!isActive())
        return;

    application()->get<EngineRandomComponent>()->reset(RANDOM_ENGINE_MERSENNE_TWISTER);
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
