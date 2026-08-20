#include <string>
#include <thread>

#include "Testing/Unit/UnitTest.h"

#include "Library/StackTrace/StackTrace.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Attributes.h"

/**
 * Takes a stack trace from a function with a name the test can look for.
 *
 * Not inlined so that it gets a frame of its own, and not static because windows drops private symbols from a
 * stripped pdb. Linux and macos name static functions fine.
 *
 * @return                              Stack trace taken inside this function.
 */
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
    (void)sink;
}

UNIT_TEST(StackTrace, FunctionNamesAreResolved) {
#ifdef __ANDROID__
    GTEST_SKIP() << "Stack traces are not supported on Android.";
#endif

    std::string trace = oeStackTraceMarkerFunction();

    EXPECT_TRUE(trace.contains("oeStackTraceMarkerFunction")) << trace;
}

UNIT_TEST(StackTrace, CrashHandlerNamesTheCrashingFunction) {
    // The crash path is the one that matters, and it's the one that breaks silently - a handler that traces
    // the wrong thread, or traces nothing at all, still exits with the right signal.
#ifdef __ANDROID__
    GTEST_SKIP() << "Stack traces are not supported on Android.";
#endif

    EXPECT_DEATH({
        // Gtest wraps test bodies in __try/__except, and a frame-based handler runs before any unhandled
        // exception filter, so on windows ours would never see the access violation below.
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        oeStackTraceCrashingFunction();
    }, "oeStackTraceFaultingFunction");
}

UNIT_TEST(StackTrace, CrashOnAnotherThreadIsTraced) {
    // The handlers are process-wide, but the alternate stack they run on is per-thread, so a crash away from
    // the thread that installed them is a separate path through the handler.
#ifdef __ANDROID__
    GTEST_SKIP() << "Stack traces are not supported on Android.";
#endif

    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        std::thread(oeStackTraceCrashingFunction).join();
    }, "oeStackTraceFaultingFunction");
}
