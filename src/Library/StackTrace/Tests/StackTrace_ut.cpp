#include <string>
#include <thread>

#include "Testing/Unit/UnitTest.h"

#include "Library/StackTrace/StackTrace.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Attributes.h"

#ifndef __ANDROID__ // Stack traces are not supported on android.

// Not inlined so that it gets a frame of its own, and not static because windows drops private symbols from a
// stripped pdb.
MM_NOINLINE std::string oeStackTraceMarkerFunction() {
    return stackTraceToString();
}

MM_NOINLINE static int oeStackTraceFaultingFunction() {
    // Volatile pointer, not pointer to volatile. Without it the null dereference is just undefined behaviour
    // the compiler may delete or turn into a trap instruction, and either way this stops being a null fault.
    int *volatile nowhere = nullptr;
    *nowhere = 1;
    return *nowhere;
}

MM_NOINLINE void oeStackTraceCrashingFunction() {
    volatile int sink = oeStackTraceFaultingFunction();
    (void) sink;
}

UNIT_TEST(StackTrace, FunctionNamesAreResolved) {
    std::string trace = oeStackTraceMarkerFunction();

    EXPECT_CONTAINS(trace, "oeStackTraceMarkerFunction");
    EXPECT_CONTAINS(trace, "main");
}

UNIT_TEST(StackTrace, CrashHandlerNamesTheCrashingFunction) {
    // The crash path is the one that matters, and it's the one that breaks silently - a handler that traces
    // the wrong thread, or traces nothing at all, still exits with the right signal.
    EXPECT_DEATH({
        // Gtest wraps test bodies in __try/__except, and a frame-based handler runs before any unhandled
        // exception filter, so on windows ours would never see the access violation below.
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        oeStackTraceCrashingFunction();
    }, testing::AllOf(testing::HasSubstr("oeStackTraceFaultingFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, CrashOnAnotherThreadIsTraced) {
    // The handlers are process-wide, but only the thread that installs them gets an alternate signal stack,
    // so this one runs on the worker's own stack. That's enough for anything short of stack exhaustion.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        std::thread(oeStackTraceCrashingFunction).join();
    // A worker's stack ends at the thread entry, so main being absent is what says we traced the thread that
    // crashed rather than the one that installed the handlers.
    }, testing::AllOf(testing::HasSubstr("oeStackTraceFaultingFunction"),
                      testing::Not(testing::HasSubstr("main"))));
}

#endif // !__ANDROID__
