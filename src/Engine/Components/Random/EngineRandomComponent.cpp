#include "EngineRandomComponent.h"

#include <cassert>
#include <memory>

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

RandomEngineType EngineRandomComponent::type() const {
    return _type;
}

void EngineRandomComponent::setType(RandomEngineType type) {
    if (_type == type)
        return;

    _type = type;
    swizzleGlobals();
}

void EngineRandomComponent::seed(int seed) {
    for (RandomEngineType type : _grngs.indices()) {
        _vrngs[type]->seed(seed);
        _grngs[type]->seed(seed);
    }
}

void EngineRandomComponent::installNotify() {
    for (RandomEngineType type : _grngs.indices()) {
        _vrngs[type] = createRandomEngine(type);
        _grngs[type] = createRandomEngine(type);
        _tracingGrngs[type] = std::make_unique<TracingRandomEngine>(application()->platform(), _grngs[type].get());
    }
    swizzleGlobals();
}

void EngineRandomComponent::removeNotify() {
    for (RandomEngineType type : _grngs.indices()) {
        _vrngs[type].reset();
        _grngs[type].reset();
        _tracingGrngs[type].reset();
    }
    swizzleGlobals(); // Set globals to nullptr.
}

void EngineRandomComponent::swizzleGlobals() {
    vrng = _vrngs[_type].get();
    grng = (_tracing ? _tracingGrngs : _grngs)[_type].get();
}
