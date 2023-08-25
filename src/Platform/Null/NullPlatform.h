#pragma once

#include <stdint.h>
#include <memory>
#include <vector>
#include <string>
#include "Platform/Platform.h"
#include "NullPlatformOptions.h"
#include "Utility/Geometry/Rect.h"

class NullPlatformSharedState;
class PlatformEventLoop;
class PlatformGamepad;
class PlatformWindow;
enum class PlatformStorage;
struct NullPlatformOptions;

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
    virtual std::string winQueryRegistry(const std::wstring &path) const override;
    virtual std::string storagePath(const PlatformStorage type) const override;

 private:
    std::unique_ptr<NullPlatformSharedState> _state;
    bool _cursorShown = true;
};
