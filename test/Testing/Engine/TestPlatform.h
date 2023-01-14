#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Platform/Proxy/ProxyPlatform.h"

#include "TestStateHandle.h"

class TestPlatform: public ProxyPlatform {
 public:
    TestPlatform(std::unique_ptr<Platform> base, TestStateHandle state);
    virtual ~TestPlatform();

    virtual std::unique_ptr<PlatformWindow> CreateWindow() override;
    virtual std::unique_ptr<PlatformEventLoop> CreateEventLoop() override;
    virtual int64_t TickCount() const override;

    void Reset();

 private:
    std::unique_ptr<Platform> base_;
    TestStateHandle state_;
};
