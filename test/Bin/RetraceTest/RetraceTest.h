#pragma once

#include <gtest/gtest.h>

#include <string>
#include <utility>

class EngineController;
class PlatformApplication;

class RetraceTest : public testing::Test {
 public:
    explicit RetraceTest(std::string tracePath) : _tracePath(std::move(tracePath)) {}

    static void init(EngineController *game, PlatformApplication *application);

    void TestBody() override;

 private:
    std::string _tracePath;
};
