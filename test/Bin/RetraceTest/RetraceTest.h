#pragma once

#include <gtest/gtest.h>

#include <utility>

#include "Utility/System/Path.h"

class EngineController;
class PlatformApplication;

class RetraceTest : public testing::Test {
 public:
    explicit RetraceTest(Path tracePath) : _tracePath(std::move(tracePath)) {}

    static void init(EngineController *game, PlatformApplication *application);

    void TestBody() override;

 private:
    Path _tracePath;
};
