#include <gtest/gtest.h>

#include "Testing/Extensions/ThrowingAssertions.h"

#include "Engine/Components/Control/EngineController.h"

#include "TestController.h"

class GameTest : public testing::Test {
 public:
    static void init(EngineController *engineController, TestController *testController);

 private:
    virtual void SetUp() override;
    virtual void TearDown() override;

 protected:
    EngineController *const game = nullptr;
    TestController *const test = nullptr;
};

#ifndef TEST_GROUP
#   error "Please define TEST_GROUP before including this header."
#endif

#define GAME_TEST(TestBase, TestName) \
    GTEST_TEST_(TEST_GROUP, TestBase##_##TestName, GameTest, testing::internal::GetTypeId<GameTest>())
