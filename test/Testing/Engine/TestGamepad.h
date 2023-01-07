#pragma once

#include <memory>
#include <string>

#include "Platform/PlatformGamepad.h"

#include "TestStateHandle.h"

class TestGamepad: public PlatformGamepad {
 public:
    TestGamepad(std::unique_ptr<PlatformGamepad> base, TestStateHandle state);
    virtual ~TestGamepad();

    virtual std::string Model() const override;
    virtual std::string Serial() const override;
    virtual uint32_t Id() const override;

 private:
    std::unique_ptr<PlatformGamepad> base_;
    TestStateHandle state_;
};
