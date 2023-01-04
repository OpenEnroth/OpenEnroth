#include "TestGamepad.h"

#include <cassert>
#include <utility>

TestGamepad::TestGamepad(std::unique_ptr<PlatformGamepad> base, TestStateHandle state):
    base_(std::move(base)),
    state_(std::move(state)) {
    assert(state_->gamepad == nullptr);
    state_->gamepad = this;
}

TestGamepad::~TestGamepad() {
    assert(state_->gamepad == this);
    state_->gamepad = nullptr;
}

std::string TestGamepad::Model() const {
    return base_->Model();
}

std::string TestGamepad::Serial() const {
    return base_->Serial();
}

uint32_t TestGamepad::Id() const {
    return base_->Id();
}
