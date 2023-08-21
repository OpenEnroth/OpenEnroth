#include "NullPlatform.h"

#include <utility>

#include "NullPlatformSharedState.h"
#include "NullWindow.h"
#include "NullEventLoop.h"
#include "Platform/Null/NullPlatformOptions.h"

class PlatformEventLoop;
class PlatformGamepad;
class PlatformWindow;
enum class PlatformStorage;

NullPlatform::NullPlatform(NullPlatformOptions options): _state(std::make_unique<NullPlatformSharedState>()) {
    _state->options = std::move(options);
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
    _cursorShown = cursorShown;
}

bool NullPlatform::isCursorShown() const {
    return _cursorShown;
}

std::vector<Recti> NullPlatform::displayGeometries() const {
    return _state->options.displayGeometries;
}

void NullPlatform::showMessageBox(const std::string &title, const std::string &message) const {
    // Okay?
}

int64_t NullPlatform::tickCount() const {
    return 0; // Time's not flowing in null platform.
}

std::string NullPlatform::winQueryRegistry(const std::wstring &path) const {
    return {};
}

std::string NullPlatform::storagePath(const PlatformStorage type) const {
    return {};
}

