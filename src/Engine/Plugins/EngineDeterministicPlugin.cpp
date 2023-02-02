#include "EngineDeterministicPlugin.h"

#include <cassert>
#include <utility>

#include "Library/Random/Random.h"
#include "Library/Random/NonRandomEngine.h"

EngineDeterministicPlugin::EngineDeterministicPlugin() = default;
EngineDeterministicPlugin::~EngineDeterministicPlugin() = default;

void EngineDeterministicPlugin::enterDeterministicMode() {
    _deterministicCounter++;

    if (_deterministicCounter == 1) {
        _tickCount = 0;
        _oldRandomEngine = std::move(grng);
        grng = std::make_unique<NonRandomEngine>();
    } else {
        resetDeterministicState();
    }
}

void EngineDeterministicPlugin::resetDeterministicState() {
    assert(_deterministicCounter > 0);

    _tickCount = 0;
    grng->Seed(0); // Equivalent to just recreating a NonRandomEngine.
}

void EngineDeterministicPlugin::leaveDeterministicMode() {
    assert(_deterministicCounter > 0);

    _deterministicCounter--;

    if (_deterministicCounter == 0)
        grng = std::move(_oldRandomEngine);
}

int64_t EngineDeterministicPlugin::TickCount() const {
    if (_deterministicCounter > 0) {
        return _tickCount;
    } else {
        return ProxyPlatform::TickCount();
    }
}

void EngineDeterministicPlugin::SwapBuffers() {
    if (_deterministicCounter > 0)
        _tickCount += FRAME_TIME_MS;

    // Tail calling is good practice - this way users can reason about the order of proxy execution.
    ProxyOpenGLContext::SwapBuffers();
}
