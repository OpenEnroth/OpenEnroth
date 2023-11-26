#include "EngineRandomComponent.h"

#include <cassert>

#include "Engine/Random/Random.h"

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Random/MersenneTwisterRandomEngine.h"
#include "Library/Random/SequentialRandomEngine.h"

#include "TracingRandomEngine.h"

static EngineRandomComponent *globalRandomComponent = nullptr;

static std::unique_ptr<RandomEngine> createRandomEngine(RandomEngineType type) {
    if (type == RANDOM_ENGINE_MERSENNE_TWISTER) {
        return std::make_unique<MersenneTwisterRandomEngine>();
    } else {
        assert(type == RANDOM_ENGINE_SEQUENTIAL);
        return std::make_unique<SequentialRandomEngine>();
    }
}

EngineRandomComponent::EngineRandomComponent() {
    assert(globalRandomComponent == nullptr);
    globalRandomComponent = this;
}

EngineRandomComponent::~EngineRandomComponent() {
    assert(globalRandomComponent == this);
    globalRandomComponent = nullptr;
}

bool EngineRandomComponent::isTracing() const {
    return _tracing;
}

void EngineRandomComponent::setTracing(bool tracing) {
    if (_tracing == tracing)
        return;

    _tracing = tracing;
    swizzleGlobals();
}

void EngineRandomComponent::reset(RandomEngineType withType) {
    _vrng = createRandomEngine(withType);
    _grng = createRandomEngine(withType);
    _tracingGrng = std::make_unique<TracingRandomEngine>(application()->platform(), _grng.get());
    swizzleGlobals();
}

void EngineRandomComponent::installNotify() {
    reset(RANDOM_ENGINE_MERSENNE_TWISTER);
}

void EngineRandomComponent::removeNotify() {
    _vrng.reset();
    _grng.reset();
    _tracingGrng.reset();
    swizzleGlobals(); // Set globals to nullptr.
}

void EngineRandomComponent::swizzleGlobals() {
    vrng._ptr = _vrng.get();
    grng._ptr = _tracing ? _tracingGrng.get() : _grng.get();
}
