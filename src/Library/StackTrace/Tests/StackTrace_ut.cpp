#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/StackTrace/StackTrace.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Attributes.h"

// This is what guards the toolchain flags and the debug info that stack traces need, and nothing else would
// notice if a build stopped naming frames. Not inlined so that it gets a frame of its own, and not static
// because windows drops private symbols from a stripped pdb - linux and macos name static functions fine.
MM_NOINLINE std::string oeStackTraceMarkerFunction() {
    return stackTraceToString();
}

MM_NOINLINE void oeStackTraceCrashingFunction() {
    volatile int *nowhere = nullptr;
    *nowhere = 1;
}

UNIT_TEST(StackTrace, FunctionNamesAreResolved) {
#ifdef __ANDROID__
    GTEST_SKIP() << "Stack traces are not supported on Android.";
#else
    std::string trace = oeStackTraceMarkerFunction();

    EXPECT_FALSE(trace.empty());
    EXPECT_TRUE(trace.contains("oeStackTraceMarkerFunction")) << trace;
#endif
}

// The crash path is the one that matters, and it's the one that breaks silently - a handler that traces the
// wrong thread, or traces nothing at all, still exits with the right signal.
UNIT_TEST(StackTrace, CrashHandlerNamesTheCrashingFunction) {
#ifdef __ANDROID__
    GTEST_SKIP() << "Stack traces are not supported on Android.";
#else
    EXPECT_DEATH({
        StackTraceOnCrash handler;
        oeStackTraceCrashingFunction();
    }, "oeStackTraceCrashingFunction");
#endif
}
