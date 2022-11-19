#include "GameTest.h"

#include <cassert>

static GameWrapper* globalGameWrapper = nullptr;

void GameTest::Init(GameWrapper* withWrapper) {
    assert(withWrapper);

    globalGameWrapper = withWrapper;
}

void GameTest::SetUp() {
    game_ = globalGameWrapper;
}

void GameTest::TearDown() {
    game_ = nullptr;
}
