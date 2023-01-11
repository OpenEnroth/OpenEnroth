#include "ProxyPlatform.h"

#include <cassert>

#include "Platform/PlatformGamepad.h"
#include "Platform/PlatformWindow.h"
#include "Platform/PlatformEventLoop.h"

ProxyPlatform::ProxyPlatform(Platform *base): ProxyBase<Platform>(base) {}

std::unique_ptr<PlatformWindow> ProxyPlatform::CreateWindow() {
    return NonNullBase()->CreateWindow();
}

std::unique_ptr<PlatformEventLoop> ProxyPlatform::CreateEventLoop() {
    return NonNullBase()->CreateEventLoop();
}

std::unique_ptr<PlatformGamepad> ProxyPlatform::CreateGamepad(uint32_t id) {
    return NonNullBase()->CreateGamepad(id);
}

void ProxyPlatform::SetCursorShown(bool cursorShown) {
    return NonNullBase()->SetCursorShown(cursorShown);
}

bool ProxyPlatform::IsCursorShown() const {
    return NonNullBase()->IsCursorShown();
}

std::vector<Recti> ProxyPlatform::DisplayGeometries() const {
    return NonNullBase()->DisplayGeometries();
}

void ProxyPlatform::ShowMessageBox(const std::string &message, const std::string& title) const {
    NonNullBase()->ShowMessageBox(message, title);
}

int64_t ProxyPlatform::TickCount() const {
    return NonNullBase()->TickCount();
}

std::string ProxyPlatform::WinQueryRegistry(const std::wstring &path) const {
    return NonNullBase()->WinQueryRegistry(path);
}

std::string ProxyPlatform::StoragePath(const PLATFORM_STORAGE type) const {
    return NonNullBase()->StoragePath(type);
}
