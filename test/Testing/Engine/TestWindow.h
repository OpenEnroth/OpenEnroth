#pragma once

#include <memory>
#include <string>

#include "Platform/Proxy/ProxyWindow.h"

#include "TestStateHandle.h"

class TestWindow : public ProxyWindow {
 public:
    TestWindow(std::unique_ptr<PlatformWindow> base, TestStateHandle state);
    virtual ~TestWindow();

    virtual std::unique_ptr<PlatformOpenGLContext> CreateOpenGLContext(const PlatformOpenGLOptions &options) override;

 private:
    std::unique_ptr<PlatformWindow> base_;
    TestStateHandle state_;
};
