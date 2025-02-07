#include "NullPlatform.h"

#include <utility>
#include <memory>
#include <vector>
#include <string>

#include "Library/Logger/Logger.h"

#include "NullPlatformSharedState.h"
#include "NullWindow.h"
#include "NullEventLoop.h"

NullPlatform::NullPlatform(NullPlatformOptions options): _state(std::make_unique<NullPlatformSharedState>(std::move(options))) {
    assert(logger); // Some of NullPlatform classes log.
}

NullPlatform::~NullPlatform() = default;

std::unique_ptr<PlatformWindow> NullPlatform::createWindow() {
    return std::make_unique<NullWindow>(_state.get());
}

std::unique_ptr<PlatformEventLoop> NullPlatform::createEventLoop() {
    return std::make_unique<NullEventLoop>(_state.get());
}

std::vector<PlatformGamepad *> NullPlatform::gamepads() {
    return {}; // No gamepad support in null platform.
}

void NullPlatform::setCursorShown(bool cursorShown) {
    _state->cursorShown = cursorShown;
}

bool NullPlatform::isCursorShown() const {
    return _state->cursorShown;
}

Pointi NullPlatform::getCursorPosition() const {
    return _state->cursorPos;
}

void NullPlatform::setCursorPosition(const Pointi &position) const {
    const Recti &screen = _state->options.displayGeometries[0];

    _state->cursorPos = Pointi(std::clamp(position.x, screen.x, screen.x + screen.w - 1),
                               std::clamp(position.y, screen.y, screen.y + screen.h - 1));
}

std::vector<Recti> NullPlatform::displayGeometries() const {
    return _state->options.displayGeometries;
}

void NullPlatform::showMessageBox(const std::string &title, const std::string &message) const {
    // No GUI in null platform.
}

int64_t NullPlatform::tickCount() const {
    return 0; // Time's not flowing in null platform.
}
