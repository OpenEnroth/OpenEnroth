#include "ProxyPlatform.h"

#include "Platform/Platform.h"
#include "Platform/Proxy/ProxyBase.h"
#include "Platform/PlatformEventLoop.h"
#include "Platform/PlatformWindow.h"

class PlatformEventLoop;
class PlatformGamepad;
class PlatformWindow;
enum class PlatformStorage;

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

std::vector<Recti> ProxyPlatform::displayGeometries() const {
    return nonNullBase()->displayGeometries();
}

void ProxyPlatform::showMessageBox(const std::string &title, const std::string &message) const {
    nonNullBase()->showMessageBox(title, message);
}

int64_t ProxyPlatform::tickCount() const {
    return nonNullBase()->tickCount();
}

std::string ProxyPlatform::winQueryRegistry(const std::wstring &path) const {
    return nonNullBase()->winQueryRegistry(path);
}

std::string ProxyPlatform::storagePath(const PlatformStorage type) const {
    return nonNullBase()->storagePath(type);
}
