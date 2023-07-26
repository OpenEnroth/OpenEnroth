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

#define GAME_TEST(TestBase, TestName) \
    GTEST_TEST_(TestBase, TestName, GameTest, testing::internal::GetTypeId<GameTest>())
