#pragma once

#include <memory>
#include <vector>

#include "Platform/Platform.h"

#include "GameTestStateHandle.h"

class Log;

class GameTestPlatform: public Platform {
 public:
    GameTestPlatform(std::unique_ptr<Platform> base, GameTestStateHandle state);

    virtual std::unique_ptr<PlatformWindow> CreateWindow() override;
    virtual std::unique_ptr<PlatformEventLoop> CreateEventLoop() override;
    virtual void SetCursorShown(bool cursorShown) override;
    virtual bool IsCursorShown() const override;
    virtual std::vector<Recti> DisplayGeometries() const override;

 private:
    std::unique_ptr<Platform> base_;
    GameTestStateHandle state_;
};
