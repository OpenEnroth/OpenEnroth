#include "ProxyPlatform.h"

#include <memory>
#include <vector>
#include <string>

#include "Library/Platform/Interface/PlatformWindow.h"
#include "Library/Platform/Interface/PlatformEventLoop.h"

ProxyPlatform::ProxyPlatform(Platform *base): ProxyBase<Platform>(base) {}

std::unique_ptr<PlatformWindow> ProxyPlatform::createWindow() {
    return nonNullBase()->createWindow();
}

std::unique_ptr<PlatformEventLoop> ProxyPlatform::createEventLoop() {
    return nonNullBase()->createEventLoop();
}

std::vector<PlatformGamepad *> ProxyPlatform::gamepads() {
    return nonNullBase()->gamepads();
}

void ProxyPlatform::setCursorShown(bool cursorShown) {
    return nonNullBase()->setCursorShown(cursorShown);
}

bool ProxyPlatform::isCursorShown() const {
    return nonNullBase()->isCursorShown();
}

Pointi ProxyPlatform::getCursorPosition() const {
	return nonNullBase()->getCursorPosition();
}

void ProxyPlatform::setCursorPosition(const Pointi& position) const {
	nonNullBase()->setCursorPosition(position);
}

std::vector<Recti> ProxyPlatform::displayGeometries() const {
    return nonNullBase()->displayGeometries();
}

void ProxyPlatform::showMessageBox(const std::string &title, const std::string &message) const {
    nonNullBase()->showMessageBox(title, message);
}

int64_t ProxyPlatform::tickCount() const {
    return nonNullBase()->tickCount();
}
