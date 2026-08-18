#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/StackTrace/StackTrace.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Attributes.h"

// This is what guards the toolchain flags and the debug info that stack traces need, and nothing else would
// notice if a build stopped naming frames. Not inlined so that it gets a frame of its own, and not static
// because windows drops private symbols from a stripped pdb - linux and macos name static functions fine.
MM_NOINLINE std::string oeStackTraceMarkerFunction() {
    std::string trace = stackTraceToString();

    // Returning something derived from the result keeps the call above out of tail position. A sibling call
    // would pop this frame before the trace is taken, and this frame is what the test looks for.
    return trace.empty() ? std::string() : trace;
}

MM_NOINLINE static int oeStackTraceFaultingFunction() {
    // Volatile pointer, not pointer to volatile - otherwise the compiler knows it's null and drops the store,
    // and at -O2 nothing crashes at all.
    int *volatile nowhere = nullptr;
    *nowhere = 1;
    return *nowhere;
}

// The fault is one call deeper so that this frame is a caller rather than the faulting leaf. Unwinding out of
// a signal recovers the leaf on linux but not on macos, where the trace jumps straight to the caller.
MM_NOINLINE void oeStackTraceCrashingFunction() {
    volatile int sink = oeStackTraceFaultingFunction();
    (void)sink;
}

UNIT_TEST(StackTrace, FunctionNamesAreResolved) {
#ifdef __ANDROID__
    GTEST_SKIP() << "Stack traces are not supported on Android.";
#elif defined(__APPLE__)
    // Cpptrace drops the innermost couple of frames on macos, so the frame this looks for is never in the
    // trace. CrashHandlerNamesTheCrashingFunction covers symbolization there instead, it asserts on a frame
    // far enough out to survive.
    GTEST_SKIP() << "Cpptrace doesn't report the innermost frames on macos.";
#else
    std::string trace = oeStackTraceMarkerFunction();

    EXPECT_TRUE(trace.contains("oeStackTraceMarkerFunction")) << trace;
#endif
}

UNIT_TEST(StackTrace, CrashHandlerNamesTheCrashingFunction) {
    // The crash path is the one that matters, and it's the one that breaks silently - a handler that traces
    // the wrong thread, or traces nothing at all, still exits with the right signal.
#ifdef __ANDROID__
    GTEST_SKIP() << "Stack traces are not supported on Android.";
#else
    // How far cpptrace gets unwinding out of a crash is platform-dependent, so the frame is only asserted on
    // where it's reachable. Everywhere else this still checks the part that matters most - that the handler
    // ran at all and said what happened. Tracing out of an SEH filter needs the CONTEXT record to get back
    // across ntdll's dispatcher and cpptrace takes none, which 64-bit survives on unwind data and 32-bit
    // doesn't. Macos x86_64 hands back an empty trace once optimizations are on.
#if (defined(_WIN32) && !defined(_WIN64)) || (defined(__APPLE__) && defined(__x86_64__))
    const char *expected = "Crashed: ";
#else
    const char *expected = "oeStackTraceCrashingFunction";
#endif

    EXPECT_DEATH({
        // Gtest wraps test bodies in __try/__except, and a frame-based handler runs before any unhandled
        // exception filter, so on windows ours would never see the access violation below.
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        oeStackTraceCrashingFunction();
    }, expected);
#endif
}
