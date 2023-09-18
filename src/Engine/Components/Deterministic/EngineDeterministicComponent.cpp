#include "EngineDeterministicComponent.h"

#include <cassert>
#include <utility>

#include "Library/Random/Random.h"
#include "Library/Random/RandomEngine.h"

EngineDeterministicComponent::EngineDeterministicComponent() = default;
EngineDeterministicComponent::~EngineDeterministicComponent() = default;

void EngineDeterministicComponent::restart(int frameTimeMs, RandomEngineType rngType) {
    assert(frameTimeMs >= 1 && frameTimeMs <= 1000);

    if (!isActive())
        _oldRandomEngine = std::move(grng);

    _tickCount = 0;
    _frameTimeMs = frameTimeMs;
    grng = RandomEngine::create(rngType);
    assert(isActive());
}

void EngineDeterministicComponent::finish() {
    if (!isActive())
        return;

    grng = std::move(_oldRandomEngine);
    assert(!isActive());
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
