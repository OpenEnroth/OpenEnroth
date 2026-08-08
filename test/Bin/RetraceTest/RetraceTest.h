#pragma once

#include <gtest/gtest.h>

#include <utility>

#include "Utility/System/NativePath.h"

class EngineController;
class PlatformApplication;

class RetraceTest : public testing::Test {
 public:
    explicit RetraceTest(NativePath tracePath) : _tracePath(std::move(tracePath)) {}

    static void init(EngineController *game, PlatformApplication *application);

    void TestBody() override;

 private:
    NativePath _tracePath;
};
