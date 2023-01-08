#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Platform/Platform.h"

#include "TestStateHandle.h"

class TestPlatform: public Platform {
 public:
    TestPlatform(std::unique_ptr<Platform> base, TestStateHandle state);
    virtual ~TestPlatform();

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

    void Reset();

 private:
    std::unique_ptr<Platform> base_;
    TestStateHandle state_;
};
