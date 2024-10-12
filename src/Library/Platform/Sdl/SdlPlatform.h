#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Library/Platform/Interface/Platform.h"

class SdlWindow;
class SdlGamepad;
class SdlPlatformSharedState;

class SdlPlatform: public Platform {
 public:
    SdlPlatform();
    virtual ~SdlPlatform();

    virtual std::unique_ptr<PlatformWindow> createWindow() override;
    virtual std::unique_ptr<PlatformEventLoop> createEventLoop() override;

    virtual std::vector<PlatformGamepad *> gamepads() override;

    virtual void setCursorShown(bool cursorShown) override;
    virtual bool isCursorShown() const override;

    virtual std::vector<Recti> displayGeometries() const override;

    virtual void showMessageBox(const std::string &title, const std::string &message) const override;

    virtual int64_t tickCount() const override;

 private:
    bool _initialized = false;
    std::unique_ptr<SdlPlatformSharedState> _state;
};
