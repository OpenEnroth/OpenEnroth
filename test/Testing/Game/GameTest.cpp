#include "GameTest.h"

#include <cassert>

#include "Engine/Components/Control/EngineController.h"

static EngineController *globalEngineController = nullptr;
static TestController *globalTestController = nullptr;

void GameTest::init(EngineController *engineController, TestController *testController) {
    assert(!globalEngineController && !globalTestController);
    assert(engineController && testController);

    globalEngineController = engineController;
    globalTestController = testController;
}

void GameTest::SetUp() {
    const_cast<EngineController *&>(game) = globalEngineController;
    const_cast<TestController *&>(test) = globalTestController;

    test->prepareForNextTest();
}

void GameTest::TearDown() {
    const_cast<EngineController *&>(game) = nullptr;
    const_cast<TestController *&>(test) = nullptr;
}
