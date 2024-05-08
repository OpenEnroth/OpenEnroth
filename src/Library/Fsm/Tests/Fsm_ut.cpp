#include <utility>
#include <memory>

#include "Testing/Unit/UnitTest.h"

#include "FsmTestUtils.h"

#include "Library/Fsm/FsmBuilder.h"
#include "Utility/String/Format.h"

using ::testing::AtLeast;
using ::testing::Exactly;

UNIT_TEST_FIXTURE(FsmTest, CallStartingStateEnterWhenCreatingFsm) {
    auto state = std::make_unique<MockFsmState>();
    EXPECT_CALL(*state.get(), enter())
        .Times(Exactly(1));

    _addState("TestState", std::move(state));

    auto fsm = _createFsm("TestState");
}

UNIT_TEST_FIXTURE(FsmTest, NoCallStartingStateEnterWhenCreatingFsm) {
    auto state = std::make_unique<MockFsmState>();
    EXPECT_CALL(*state.get(), enter())
        .Times(Exactly(1));

    _addState("TestState", std::move(state));

    auto fsm = _createFsm("WrongName");
}
