#include "NullWindow.h"

#include <cassert>

#include "NullPlatformSharedState.h"
#include "NullOpenGLContext.h"
#include "Platform/Null/NullPlatformOptions.h"

class PlatformOpenGLContext;
struct PlatformOpenGLOptions;

NullWindow::NullWindow(NullPlatformSharedState *state): _state(state) {
    assert(state);

    _position = _state->options.defaultWindowPosition;
    _size = _state->options.defaultWindowSize;
    _systemHandle = _state->nextWinId++;
}

NullWindow::~NullWindow() = default;

void NullWindow::setTitle(const std::string &title) {
    _title = title;
}

std::string NullWindow::title() const {
    return _title;
}

void NullWindow::resize(const Sizei &size) {
    _size = size;
}

Sizei NullWindow::size() const {
    return _size;
}

void NullWindow::setPosition(const Pointi &pos) {
    _position = pos;
}

Pointi NullWindow::position() const {
    return _position;
}

void NullWindow::setVisible(bool visible) {
    _visible = visible;
}

bool NullWindow::isVisible() const {
    return _visible;
}

void NullWindow::setResizable(bool resizable) {
    _resizable = resizable;
}

bool NullWindow::isResizable() const {
    return _resizable;
}

void NullWindow::setWindowMode(PlatformWindowMode mode) {
    _mode = mode;
}

PlatformWindowMode NullWindow::windowMode() {
    return _mode;
}

void NullWindow::setGrabsMouse(bool grabsMouse) {
    _grabsMouse = grabsMouse;
}

bool NullWindow::grabsMouse() const {
    return _grabsMouse;
}

void NullWindow::setOrientations(PlatformWindowOrientations orientations) {
    _orientations = orientations;
}

PlatformWindowOrientations NullWindow::orientations() {
    return _orientations;
}

Marginsi NullWindow::frameMargins() const {
    if (_mode == WINDOW_MODE_WINDOWED || _mode == WINDOW_MODE_FULLSCREEN) {
        return _state->options.defaultFrameMargins;
    } else {
        return Marginsi(0, 0, 0, 0);
    }
}

uintptr_t NullWindow::systemHandle() const {
    return _systemHandle;
}

void NullWindow::activate() {
    // Do nothing.
}

std::unique_ptr<PlatformOpenGLContext> NullWindow::createOpenGLContext(const PlatformOpenGLOptions &options) {
    return std::make_unique<NullOpenGLContext>(_state);
}
