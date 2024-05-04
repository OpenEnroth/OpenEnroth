#include "GameTest.h"

#include <cassert>
#include <string>

#include "Engine/EngineGlobals.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Control/EngineControlState.h"

#include "Library/Platform/Application/PlatformApplication.h"

#include "Utility/String/Format.h"

static EngineController *globalEngineController = nullptr;
static TestController *globalTestController = nullptr;
static bool globalTerminating = false;

void GameTest::init(EngineController *engineController, TestController *testController) {
    assert(!globalEngineController && !globalTestController);
    assert(engineController && testController);

    testing::GTEST_FLAG(catch_exceptions) = false; // We handle exceptions ourselves in runTestBody.

    globalEngineController = engineController;
    globalTestController = testController;
}

void GameTest::SetUp() {
    assert(!globalTerminating); // Shouldn't get here if we're already terminating.

    const_cast<EngineController *&>(game) = globalEngineController;
    const_cast<TestController *&>(test) = globalTestController;

    test->prepareForNextTest();
}

void GameTest::TearDown() {
    const_cast<EngineController *&>(game) = nullptr;
    const_cast<TestController *&>(test) = nullptr;
}

void GameTest::runTestBody(TestBodyFunction testBody) {
    auto failCurrentTest = [](const std::string &message) {
        try {
            GTEST_FATAL_FAILURE_(message.c_str());
        } catch (const testing::AssertionException &) {
            // That feel when you have to work around your own workarounds. GTEST_FATAL_FAILURE_ from
            // ThrowingAssertions throws, but this is not what we need here. We just want for the test to fail. Thus
            // need to ignore this one.
        } catch (...) {
            // Shouldn't happen. Caller expects this function to never throw.
            assert(false);
        }
    };

    try {
        game->runGameRoutine([] {
            const testing::TestInfo *info = testing::UnitTest::GetInstance()->current_test_info();
            ::application->window()->setTitle(fmt::format("{}.{}", info->test_suite_name(), info->name()));
        });

        CommonTapeRecorder tapes(test);
        CharacterTapeRecorder charTapes(test);
        ActorTapeRecorder actorTapes(test);
        testBody(*game, *test, tapes, charTapes, actorTapes);
    } catch (EngineControlState::TerminationException) {
        failCurrentTest("Game test routine was terminated, all tests cancelled");
        globalTerminating = true;
        throw; // This one goes outside to terminate the control routine.
    } catch (const testing::AssertionException &) {
        assert(::testing::Test::HasFailure()); // Test's already failed, nothing else to do.
    } catch (const std::exception &exception) {
        failCurrentTest(fmt::format("Exception thrown in test body: {}", exception.what()));
    } catch (...) {
        failCurrentTest("Unknown exception thrown in test body");
    }
}
