#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Library/Platform/Interface/Platform.h"

#include "NullPlatformOptions.h"

class NullPlatformSharedState;

class NullPlatform : public Platform {
 public:
    explicit NullPlatform(NullPlatformOptions options);
    virtual ~NullPlatform();

    virtual std::unique_ptr<PlatformWindow> createWindow() override;
    virtual std::unique_ptr<PlatformEventLoop> createEventLoop() override;
    virtual std::vector<PlatformGamepad *> gamepads() override;
    virtual void setCursorShown(bool cursorShown) override;
    virtual bool isCursorShown() const override;
    virtual std::vector<Recti> displayGeometries() const override;
    virtual void showMessageBox(const std::string &title, const std::string &message) const override;
    virtual int64_t tickCount() const override;

 private:
    std::unique_ptr<NullPlatformSharedState> _state;
};
