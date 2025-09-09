#include "NullEventLoop.h"

#include <cassert>

#include "Library/Logger/Logger.h"

NullEventLoop::NullEventLoop(NullPlatformSharedState *state): _state(state) {
    assert(state);
}

NullEventLoop::~NullEventLoop() = default;

void NullEventLoop::exec(PlatformEventHandler *eventHandler) {
    // Null platform doesn't receive messages. In theory, we should deadlock here, but this makes no sense tbh.
}

void NullEventLoop::quit() {
    // Nothing to quit from.
}

void NullEventLoop::processMessages(PlatformEventHandler *eventHandler, int count) {
    // No messages in event queue => return immediately.
}

void NullEventLoop::waitForMessages() {
    // Null platform doesn't receive messages. In theory, we should deadlock here, but this makes no sense tbh.
    logger->error("Calls to NullEventLoop::waitForMessages should never happen. Null platform never waits.");
}
