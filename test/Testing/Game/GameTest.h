#include <gtest/gtest.h>

#include "Testing/Extensions/ThrowingAssertions.h"

#include "Engine/Components/Control/EngineController.h"

#include "Utility/Preprocessor.h"

#include "TestController.h"

class GameTest : public testing::Test {
 public:
    static void init(EngineController *engineController, TestController *testController);

 protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

    using TestBodyFunction = void (*)(GameTest *);
    void runTestBody(TestBodyFunction testBody);

 protected:
    EngineController *const game = nullptr;
    TestController *const test = nullptr;
};

#define GAME_TEST(SuiteName, TestName)                                                                                  \
    GAME_TEST_I(SuiteName, TestName, GTEST_TEST_CLASS_NAME_(SuiteName, TestName))

#define GAME_TEST_I(SuiteName, TestName, TestClassName)                                                                 \
    GAME_TEST_II(SuiteName, TestName, TestClassName, MM_PP_CAT(TestClassName, _Wrapper), MM_PP_CAT(Invoke_, TestClassName))

#define GAME_TEST_II(SuiteName, TestName, TestClassName, TestWrapperName, TestInvokerName)                              \
    void TestInvokerName(GameTest *);                                                                                   \
                                                                                                                        \
    GTEST_TEST_(SuiteName, TestName, GameTest, testing::internal::GetTypeId<GameTest>()) {                              \
        runTestBody(&TestInvokerName);                                                                                  \
    }                                                                                                                   \
                                                                                                                        \
    class TestWrapperName : public TestClassName {                                                                      \
     public:                                                                                                            \
        void ActualTestBody();                                                                                          \
    };                                                                                                                  \
                                                                                                                        \
    void TestInvokerName(GameTest *test) {                                                                              \
        static_cast<TestWrapperName *>(test)->ActualTestBody();                                                         \
    }                                                                                                                   \
                                                                                                                        \
    void TestWrapperName::ActualTestBody() /* Body follows. */
