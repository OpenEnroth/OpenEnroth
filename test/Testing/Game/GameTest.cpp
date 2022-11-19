#include "GameTest.h"

#include <cassert>

static GameWrapper *globalGameWrapper = nullptr;

void GameTest::Init(GameWrapper *withWrapper) {
    assert(withWrapper);

    globalGameWrapper = withWrapper;
}

void GameTest::SetUp() {
    const_cast<GameWrapper *&>(game) = globalGameWrapper;
}

void GameTest::TearDown() {
    const_cast<GameWrapper *&>(game) = nullptr;
}
