#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <string>
#include <string_view>
#include <thread>

#ifdef _WINDOWS
#   include <crtdbg.h> // NOLINT: not a C++ system header.
#endif

#include "Testing/Unit/UnitTest.h"

#include "Library/StackTrace/StackTrace.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Attributes.h"
#include "Utility/String/Format.h"
#include "Utility/String/Split.h"

#ifndef __ANDROID__ // Stack traces are not supported on android.

// On windows dedicated CRT hooks print the reasons asserted below. On posix there are no hooks - abort and
// terminate arrive as SIGABRT with the abort machinery in the output, and a pure call is a plain crash - so
// the posix side of each check probes for that instead. Mac needs its own spellings - its abort frame prints
// with no name, and libc++abi says "terminating" rather than "terminate".
#ifdef _WINDOWS
constexpr bool isWindows = true;
#else
constexpr bool isWindows = false;
#endif
#ifdef __APPLE__
constexpr bool isMac = true;
#else
constexpr bool isMac = false;
#endif

static void sendAssertReportsToStderr() {
#ifdef _WINDOWS
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE); // The debug CRT would otherwise put up a dialog and wait.
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
}

/**
 * Matches when the frame numbered `index` names `function`. The regexes gtest's own death test matchers take
 * aren't portable - gtest picks between two engines with different grammars depending on the platform - so
 * this walks the lines instead.
 */
MATCHER_P2(HasFrame, index, function, "") {
    std::string prefix = fmt::format("#{} ", index);
    for (std::string_view line : split(std::string_view(arg)).by('\n'))
        if (line.starts_with(prefix) && line.contains(function))
            return true;
    return false;
}

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

struct StackTracePureCallBase {
    StackTracePureCallBase() { callPureIndirectly(); }
    virtual void callPure() = 0;

    MM_NOINLINE void callPureIndirectly() {
        // An extra hop the compiler can't fold away. Inlined into the constructor, the call site would have a
        // known dynamic type, and the call devirtualizes into a direct one to a function with no body.
        volatile int keepFrame = 0;
        callPure();
        keepFrame = 1; // Or the call is in tail position, becomes a jump, and this frame is gone from the trace.
    }
};
struct StackTracePureCallDerived : StackTracePureCallBase {
    virtual void callPure() override {}
};

MM_NOINLINE void stackTracePureCallFunction() {
    StackTracePureCallDerived derived;
}

MM_NOINLINE void stackTraceTerminateFunction() {
    volatile int keepFrame = 0;
    std::terminate();
    keepFrame = 1; // Or the noreturn call becomes a jump, and this frame is gone before the handler runs.
}

MM_NOINLINE void stackTraceAbortFunction() {
    volatile int keepFrame = 0;
    std::abort();
    keepFrame = 1; // Or the noreturn call becomes a jump, and this frame is gone before the handler runs.
}

MM_NOINLINE void stackTraceAssertFunction() {
    volatile int keepFrame = 0;
    assert(keepFrame != 0);
    keepFrame = 1; // Or the noreturn call becomes a jump, and this frame is gone before the handler runs.
}

#ifdef _WINDOWS
MM_NOINLINE void stackTraceInvalidParameterFunction() {
    volatile int keepFrame = 0;
    std::printf(nullptr); // Null format string is the canonical way to trip the invalid parameter handler.
    keepFrame = 1; // Or the call is in tail position, becomes a jump, and this frame is gone from the trace.
}
#endif // _WINDOWS

MM_NOINLINE int stackTraceNullCallFunction() {
    int (*volatile nowhere)() = nullptr; // Volatile, or the compiler sees the target and emits a trap instead.
    volatile int result = nowhere(); // Using the result keeps this out of tail position, which keeps the frame.
    return result + 1;
}

MM_NOINLINE int stackTraceBadTargetCallFunction() {
    int (*volatile nowhere)() = reinterpret_cast<int (*)()>(static_cast<uintptr_t>(0xdeadbeefdeadULL));
    volatile int result = nowhere(); // Using the result keeps this out of tail position, which keeps the frame.
    return result + 1;
}

static volatile char *volatile stackTraceOverflowEscape; // Never read, the pad below is stored here so it exists.

MM_NOINLINE int stackTraceOverflowFunction(int depth) {
    volatile char pad[1024]; // Big frames overflow fast, and the volatile writes keep the endless recursion out of UB land.
    stackTraceOverflowEscape = pad; // Address taken, so the pad keeps its size at -O2 and the recursion can't be folded into a loop.
    pad[0] = static_cast<char>(depth);
    return pad[0] + stackTraceOverflowFunction(depth + 1);
}

UNIT_TEST(StackTrace, FunctionNamesAreResolved) {
    std::string trace = stackTraceMarkerFunction();

    EXPECT_THAT(trace, HasFrame(1, "stackTraceMarkerFunction"));
    EXPECT_CONTAINS(trace, "main");
}

UNIT_TEST(StackTrace, InitReturnsThePreviousCallback) {
    // The return value is what lets a callback chain to whatever was in effect before it, and what restores
    // that afterwards. Both only work if every call hands back the callback it replaced, with nullptr standing
    // for the default.
    CrashCallback custom = [](std::string_view, bool) {};
    CrashCallback previous = initStackTraceOnCrash(custom);
    EXPECT_EQ(previous, &printCrashChunk);
    EXPECT_EQ(initStackTraceOnCrash(nullptr), custom);
    EXPECT_EQ(initStackTraceOnCrash(previous), &printCrashChunk);
}

UNIT_TEST(StackTrace, CrashHandlerNamesTheCrashingFunction) {
    EXPECT_DEATH({
        // Gtest wraps test bodies in __try/__except, and a frame-based handler runs before any unhandled
        // exception filter, so on windows ours would never see the access violation below.
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        stackTraceCrashingFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceCrashingFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, CrashOnAnotherThreadIsTraced) {
    // The handlers are process-wide, but only the thread that installs them gets an alternate signal stack,
    // so this one runs on the worker's own stack. That's enough for anything short of stack exhaustion.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        std::thread(stackTraceCrashingFunction).join();
    // A worker's stack ends at the thread entry, so main being absent is what says we traced the thread that
    // crashed rather than the one that installed the handlers.
    }, testing::AllOf(HasFrame(0, "stackTraceCrashingFunction"), testing::Not(testing::HasSubstr("main"))));
}

UNIT_TEST(StackTrace, NullFunctionCallIsTraced) {
    // Calling a null pointer faults at address zero, where there's nothing to unwind from. The call pushed its
    // return address first though, and walking on from that names the function that made the call and
    // everything above it.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        stackTraceNullCallFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceNullCallFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, BadTargetCallIsTraced) {
    // Calling 0xdeadbeefdead faults with the pc at the bad address, and the handler has to recognize that
    // to walk from the caller instead.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        stackTraceBadTargetCallFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceBadTargetCallFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, StackOverflowIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "Rosetta can't reliably deliver the guard page fault.";

    // The handlers run on an alternate stack, and this is what checks it. Without one the handler itself
    // faults on the exhausted stack and the crash prints nothing at all. That is also why this child is
    // re-exec'd rather than forked - a forked child inherits the flag saying the handlers are installed, but
    // on darwin not the alternate stack, so nothing installs one. A re-exec'd child installs both in main.
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        stackTraceOverflowFunction(0);
    }, HasFrame(0, "stackTraceOverflowFunction"));
}

UNIT_TEST(StackTrace, CrashArrivesAsHeaderThenTrace) {
    // The reason goes out as a chunk of its own before anything is symbolized, so that a hang in symbolization
    // still leaves it behind, and the trace is the final chunk - the one an app callback holds a console window
    // open on, which is why nothing may follow it. Exactly two chunks, in that order, is what this guards.
    auto headerThenTrace = testing::Truly([](const std::string &output) {
        int chunks = 0;
        for (size_t pos = output.find("chunk:"); pos != std::string::npos; pos = output.find("chunk:", pos + 1))
            chunks++;
        size_t reason = output.find("chunk:\n\nCrashed because of");
        size_t trace = output.find("final chunk:\n");
        return chunks == 2 && reason != std::string::npos && trace != std::string::npos && reason < trace &&
               output.find("stackTraceCrashingFunction", trace) != std::string::npos;
    });

    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash([](std::string_view text, bool final) {
            printCrashChunk(final ? "final chunk:" : "chunk:", false);
            printCrashChunk(text, final);
        });
        stackTraceCrashingFunction();
    }, headerThenTrace);
}

UNIT_TEST(StackTrace, CrashCallbackRunsAfterTheTrace) {
    // What an app callback does on the final chunk - hold a console window open, put up a dialog - has to happen
    // after the trace has gone out, or the user acknowledges a crash they haven't been shown. The app callback
    // itself links into no test, so this pins the library half it's written against.
    auto traceBeforeMarker = testing::Truly([](const std::string &output) {
        size_t trace = output.find("stackTraceCrashingFunction");
        size_t marker = output.find("crash callback ran");
        return trace != std::string::npos && marker != std::string::npos && trace < marker;
    });

    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash([](std::string_view text, bool final) {
            printCrashChunk(text, final);
            if (final)
                printCrashChunk("crash callback ran", final);
        });
        stackTraceCrashingFunction();
    }, traceBeforeMarker);
}

UNIT_TEST(StackTrace, AbortIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "SIGABRT is left at its default under Rosetta, so there is no trace to match.";

    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        stackTraceAbortFunction();
    }, testing::AllOf(testing::HasSubstr(isWindows ? "abort()" : isMac ? "Abort trap" : "abort"),
                      testing::HasSubstr("stackTraceAbortFunction")));
}

UNIT_TEST(StackTrace, AssertIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "SIGABRT is left at its default under Rosetta, so there is no trace to match.";

    // A failed assert is the crash a debug build produces most. It arrives as an abort with the assertion
    // message printed in front, and the trace has to follow that message rather than replace it.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);
        sendAssertReportsToStderr();

        initStackTraceOnCrash();
        stackTraceAssertFunction();
    }, testing::AllOf(testing::HasSubstr("Assertion"),
                      testing::HasSubstr(isWindows ? "abort()" : isMac ? "Abort trap" : "abort"),
                      testing::HasSubstr("stackTraceAssertFunction")));
}

UNIT_TEST(StackTrace, TerminateIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "SIGABRT is left at its default under Rosetta, so there is no trace to match.";

    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        stackTraceTerminateFunction();
    }, testing::AllOf(testing::HasSubstr(isWindows ? "std::terminate()" : isMac ? "terminating" : "terminate"),
                      testing::HasSubstr("stackTraceTerminateFunction")));
}

UNIT_TEST(StackTrace, PureVirtualCallIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "SIGABRT is left at its default under Rosetta, so there is no trace to match.";

    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        stackTracePureCallFunction();
    }, testing::AllOf(testing::HasSubstr(isWindows ? "pure virtual function call" : "callPureIndirectly"),
                      testing::HasSubstr("stackTracePureCallFunction")));
}

#ifdef _WINDOWS
UNIT_TEST(StackTrace, InvalidParameterIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash();
        stackTraceInvalidParameterFunction();
    }, testing::AllOf(testing::HasSubstr("invalid parameter passed to a CRT function"),
                      testing::HasSubstr("stackTraceInvalidParameterFunction")));
}
#endif // _WINDOWS

#endif // !__ANDROID__
