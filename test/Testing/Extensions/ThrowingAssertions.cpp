#include "ThrowingAssertions.h"
#include "gtest/gtest.h"

static bool globalThrowListenerInstalled = false;
static bool globalThrowListenerEnabled = false;

class ThrowListener : public testing::EmptyTestEventListener {
    void OnTestPartResult(const testing::TestPartResult &result) override {
        if (!globalThrowListenerEnabled)
            return;

        if (result.type() == testing::TestPartResult::kFatalFailure) {
            throw testing::AssertionException(result);
        }
    }
};

detail::ScopedEnableThrowingAssertions::ScopedEnableThrowingAssertions() {
    // Totally not thread-safe, but gtest is not thread-safe on Windows anyway.
    if (!globalThrowListenerInstalled) {
        testing::UnitTest::GetInstance()->listeners().Append(new ThrowListener);
        globalThrowListenerInstalled = true;
    }

    globalThrowListenerEnabled = true;
}

detail::ScopedEnableThrowingAssertions::~ScopedEnableThrowingAssertions() {
    globalThrowListenerEnabled = false;
}
