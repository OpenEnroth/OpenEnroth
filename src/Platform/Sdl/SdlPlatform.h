#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Platform/Platform.h"

class SdlWindow;
class SdlGamepad;
class SdlPlatformSharedState;

class SdlPlatform: public Platform {
 public:
    explicit SdlPlatform(PlatformLogger *logger);
    virtual ~SdlPlatform();

    virtual std::unique_ptr<PlatformWindow> CreateWindow() override;
    virtual std::unique_ptr<PlatformEventLoop> CreateEventLoop() override;
    virtual std::unique_ptr<PlatformGamepad> CreateGamepad(uint32_t id) override;

    virtual void SetCursorShown(bool cursorShown) override;
    virtual bool IsCursorShown() const override;

    virtual std::vector<Recti> DisplayGeometries() const override;

    virtual void ShowMessageBox(const std::string &message, const std::string& title) const override;

    virtual int64_t TickCount() const override;

    virtual std::string WinQueryRegistry(const std::wstring &path) const override;

    virtual std::string StoragePath(const PLATFORM_STORAGE type) const override;

 private:
    friend class SdlWindow;
    friend class SdlEventLoop;
    friend class SdlGamepad;

 private:
    bool initialized_ = false;
    std::unique_ptr<SdlPlatformSharedState> state_;
};
