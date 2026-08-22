#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <thread>

#include "Testing/Unit/UnitTest.h"

#include "Library/StackTrace/StackTrace.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Attributes.h"

#ifndef __ANDROID__ // Stack traces are not supported on android.

// Not inlined so that it gets a frame of its own, and not static because windows drops private symbols from a
// stripped pdb.
MM_NOINLINE std::string stackTraceMarkerFunction() {
    std::string trace = stackTraceToString();

    // Deriving the result from the trace keeps the call above out of tail position. Clang tail-calls it at
    // -O2 otherwise, and then this frame, which is the one the test looks for, isn't in the trace at all.
    return trace.empty() ? std::string() : trace;
}

MM_NOINLINE int stackTraceCrashingFunction() {
    // Volatile pointer, not pointer to volatile, or the compiler knows it's null and traps instead of
    // faulting. The read feeds the return value because a store on its own is dead code that some compilers
    // drop, and then nothing crashes at all.
    int *volatile nowhere = nullptr;
    *nowhere = 1;
    return *nowhere;
}

#ifdef _WIN32
// Calling a pure virtual from a constructor reaches the base vtable before the derived one is installed, which
// is the one reliable way to hit the CRT's purecall handler. The call goes through a non-virtual member so
// that the compiler can't see the dynamic type and devirtualize it into a direct call to a function with no
// body. Not static for the same reason as above.
struct StackTracePureCallBase {
    StackTracePureCallBase() { callPureIndirectly(); }
    virtual void callPure() = 0;
    void callPureIndirectly() { callPure(); }
};
struct StackTracePureCallDerived : StackTracePureCallBase {
    virtual void callPure() override {}
};

MM_NOINLINE void stackTracePureCallFunction() {
    StackTracePureCallDerived derived;
}

MM_NOINLINE void stackTraceInvalidParameterFunction() {
    std::printf(nullptr); // Null format string is the canonical way to trip the invalid parameter handler.
}

MM_NOINLINE void stackTraceTerminateFunction() {
    std::terminate();
}

MM_NOINLINE void stackTraceAbortFunction() {
    std::abort();
}
#endif

UNIT_TEST(StackTrace, FunctionNamesAreResolved) {
    std::string trace = stackTraceMarkerFunction();

    EXPECT_CONTAINS(trace, "stackTraceMarkerFunction");
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
        stackTraceCrashingFunction();
    }, testing::AllOf(testing::HasSubstr("stackTraceCrashingFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, CrashOnAnotherThreadIsTraced) {
    // The handlers are process-wide, but only the thread that installs them gets an alternate signal stack,
    // so this one runs on the worker's own stack. That's enough for anything short of stack exhaustion.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        std::thread(stackTraceCrashingFunction).join();
    // A worker's stack ends at the thread entry, so main being absent is what says we traced the thread that
    // crashed rather than the one that installed the handlers.
    }, testing::AllOf(testing::HasSubstr("stackTraceCrashingFunction"),
                      testing::Not(testing::HasSubstr("main"))));
}

#ifdef _WIN32
// The four below each go through a different CRT hook, and each hook has its own way of being installed
// and its own way of not working. What they share is the reason string, which is the only thing that tells
// them apart in the output, so that's what each test asserts on top of the frame.

UNIT_TEST(StackTrace, AbortIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceAbortFunction();
    }, testing::AllOf(testing::HasSubstr("abort()"), testing::HasSubstr("stackTraceAbortFunction")));
}

UNIT_TEST(StackTrace, TerminateIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceTerminateFunction();
    }, testing::AllOf(testing::HasSubstr("std::terminate()"), testing::HasSubstr("stackTraceTerminateFunction")));
}

UNIT_TEST(StackTrace, PureVirtualCallIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTracePureCallFunction();
    }, testing::AllOf(testing::HasSubstr("pure virtual function call"),
                      testing::HasSubstr("stackTracePureCallFunction")));
}

UNIT_TEST(StackTrace, InvalidParameterIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceInvalidParameterFunction();
    }, testing::AllOf(testing::HasSubstr("invalid parameter passed to a CRT function"),
                      testing::HasSubstr("stackTraceInvalidParameterFunction")));
}
#endif // _WIN32

#endif // !__ANDROID__
