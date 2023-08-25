#include <gtest/gtest.h>
#include "Testing/Extensions/ThrowingAssertions.h"
#include "Engine/Components/Control/EngineController.h"
#include "Utility/Preprocessor.h"
#include "TestController.h"
#include "CharacterTapeRecorder.h"
#include "CommonTapeRecorder.h"

class CharacterTapeRecorder;
class CommonTapeRecorder;
class EngineController;
class TestController;

class GameTest : public testing::Test {
 public:
    static void init(EngineController *engineController, TestController *testController);

 protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

    using TestBodyFunction = void (*)(EngineController &, TestController &, CommonTapeRecorder &, CharacterTapeRecorder &);
    void runTestBody(TestBodyFunction testBody);

 protected:
    EngineController *const game = nullptr;
    TestController *const test = nullptr;
};

#define GAME_TEST(SuiteName, TestName)                                                                                  \
    GAME_TEST_I(SuiteName, TestName, GTEST_TEST_CLASS_NAME_(SuiteName, TestName))

#define GAME_TEST_I(SuiteName, TestName, TestClassName)                                                                 \
    GAME_TEST_II(SuiteName, TestName, TestClassName, MM_PP_CAT(Run_, TestClassName))

#define GAME_TEST_II(SuiteName, TestName, TestClassName, TestFunctionName)                                              \
    void TestFunctionName(EngineController &game, TestController &test,                                                 \
                          CommonTapeRecorder &tapes, CharacterTapeRecorder &ctapes);                                    \
                                                                                                                        \
    GTEST_TEST_(SuiteName, TestName, GameTest, testing::internal::GetTypeId<GameTest>()) {                              \
        runTestBody(&TestFunctionName);                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    void TestFunctionName(EngineController &game, TestController &test,                                                 \
                          CommonTapeRecorder &tapes, CharacterTapeRecorder &ctapes) /* Body follows. */
