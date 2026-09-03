#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <string>
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

#ifndef _WINDOWS
#   if defined(__aarch64__)
constexpr int SIG_BUILTIN_TRAP = SIGTRAP; // brk raises a breakpoint trap.
#   else
constexpr int SIG_BUILTIN_TRAP = SIGILL; // x86's ud2 and arm32's udf are both illegal instructions.
#   endif
#endif

#ifndef _WINDOWS
extern "C" void __cxa_pure_virtual() __attribute__((weak)); // Weak, so asking about it below links nothing in.
#endif

/**
 * A pure virtual call lands on a hook that aborts, and the vtable's reference to that hook is weak. Linking
 * the standard library statically then leaves the hook out and the slot null, and the call faults instead of
 * aborting. Asking whether the hook made it into the binary is what keeps this right either way.
 *
 * @return                              Signal a pure virtual call dies of.
 */
static int sigPureCall() {
#ifdef _WINDOWS
    return SIGABRT; // Windows dies through the CRT hook rather than a signal, and killedBy ignores this.
#else
    return &__cxa_pure_virtual != nullptr ? SIGABRT : SIGSEGV;
#endif
}

/**
 * Predicate for `EXPECT_EXIT` that pins how the process died, rather than just that it did. Gtest has no
 * `KilledBySignal` on windows, where a death test reports an exit code, so there this only asserts a death.
 *
 * @param signal                        Signal the process is expected to die of, ignored on windows.
 * @return                              Predicate over the exit status.
 */
static auto killedBy([[maybe_unused]] int signal) {
#ifdef _WINDOWS
    return [] (int status) { return status != 0; };
#else
    return testing::KilledBySignal(signal);
#endif
}

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
    int (*volatile nowhere)() = reinterpret_cast<int (*)()>(static_cast<uintptr_t>(0xdeadbeefdeacULL)); // Aligned, or arm faults on the fetch alignment instead of the mapping.
    volatile int result = nowhere(); // Using the result keeps this out of tail position, which keeps the frame.
    return result + 1;
}

#if !defined(__aarch64__) && !defined(__arm__) // No trap to test on arm, integer division by zero just yields zero there.
MM_NOINLINE int stackTraceDivisionFunction() {
    volatile int zero = 0; // Volatile, or the compiler sees the division by zero and emits a trap instead of dividing.
    volatile int result = 64 / zero; // Using the result keeps this out of tail position, which keeps the frame.
    return result + 1;
}
#endif

#ifndef _WINDOWS
MM_NOINLINE void stackTraceTrapFunction() {
    __builtin_trap(); // ud2 on x86, brk on arm.
}
#endif

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

UNIT_TEST(StackTrace, CrashHandlerNamesTheCrashingFunction) {
    EXPECT_EXIT({
        // Gtest wraps test bodies in __try/__except, and a frame-based handler runs before any unhandled
        // exception filter, so on windows ours would never see the access violation below.
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceCrashingFunction();
    }, killedBy(SIGSEGV), testing::AllOf(HasFrame(0, "stackTraceCrashingFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, CrashOnAnotherThreadIsTraced) {
    // The handlers are process-wide, but only the thread that installs them gets an alternate signal stack,
    // so this one runs on the worker's own stack. That's enough for anything short of stack exhaustion.
    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        std::thread(stackTraceCrashingFunction).join();
    // A worker's stack ends at the thread entry, so main being absent is what says we traced the thread that
    // crashed rather than the one that installed the handlers.
    }, killedBy(SIGSEGV), testing::AllOf(HasFrame(0, "stackTraceCrashingFunction"), testing::Not(testing::HasSubstr("main"))));
}

UNIT_TEST(StackTrace, NullFunctionCallIsTraced) {
    // Calling a null pointer faults at address zero, where there's nothing to unwind from. The call pushed its
    // return address first though, and walking on from that names the function that made the call and
    // everything above it.
    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceNullCallFunction();
    }, killedBy(SIGSEGV), testing::AllOf(HasFrame(0, "stackTraceNullCallFunction"), testing::HasSubstr("main")));
}

UNIT_TEST(StackTrace, BadTargetCallIsTraced) {
    // Calling 0xdeadbeefdeac faults with the pc at the bad address, and the handler has to recognize that
    // to walk from the caller instead.
    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceBadTargetCallFunction();
    }, killedBy(SIGSEGV), testing::AllOf(HasFrame(0, "stackTraceBadTargetCallFunction"), testing::HasSubstr("main")));
}

#if !defined(__aarch64__) && !defined(__arm__)
UNIT_TEST(StackTrace, DivisionByZeroIsTraced) {
    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceDivisionFunction();
    }, killedBy(SIGFPE), testing::AllOf(HasFrame(0, "stackTraceDivisionFunction"), testing::HasSubstr("main")));
}
#endif

#ifndef _WINDOWS
UNIT_TEST(StackTrace, BuiltinTrapIsTraced) {
    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceTrapFunction();
    }, killedBy(SIG_BUILTIN_TRAP),
       testing::AllOf(HasFrame(0, "stackTraceTrapFunction"), testing::HasSubstr("main")));
}
#endif

UNIT_TEST(StackTrace, StackOverflowIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "Rosetta can't reliably deliver the guard page fault.";

    // The handlers run on an alternate stack, and this is what checks it. Without one the handler itself
    // faults on the exhausted stack and the crash prints nothing at all.
    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceOverflowFunction(0);
    }, killedBy(SIGSEGV), HasFrame(0, "stackTraceOverflowFunction"));
}

UNIT_TEST(StackTrace, CrashCallbackRunsAfterTheTrace) {
    // The callback is what holds a console window open after a crash, so it has to fire after the trace is
    // printed. Matching on order and not just presence is what guards that.
    auto callbackAfterTrace = testing::Truly([](const std::string &output) {
        size_t tracePos = output.find("stackTraceCrashingFunction");
        size_t callbackPos = output.find("crash callback ran");
        return tracePos != std::string::npos && callbackPos != std::string::npos && tracePos < callbackPos;
    });

    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler([] { std::fputs("crash callback ran", stderr); });
        stackTraceCrashingFunction();
    }, killedBy(SIGSEGV), callbackAfterTrace);
}

UNIT_TEST(StackTrace, AbortIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "SIGABRT is left at its default under Rosetta, so there is no trace to match.";

    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceAbortFunction();
    }, killedBy(SIGABRT), testing::AllOf(testing::HasSubstr(isWindows ? "abort()" : isMac ? "Abort trap" : "abort"),
                                         testing::HasSubstr("stackTraceAbortFunction")));
}

UNIT_TEST(StackTrace, AssertIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "SIGABRT is left at its default under Rosetta, so there is no trace to match.";

    // A failed assert is the crash a debug build produces most. It arrives as an abort with the assertion
    // message printed in front, and the trace has to follow that message rather than replace it.
    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);
        sendAssertReportsToStderr();

        StackTraceOnCrash handler;
        stackTraceAssertFunction();
    }, killedBy(SIGABRT), testing::AllOf(testing::HasSubstr("Assertion"),
                                         testing::HasSubstr(isWindows ? "abort()" : isMac ? "Abort trap" : "abort"),
                                         testing::HasSubstr("stackTraceAssertFunction")));
}

UNIT_TEST(StackTrace, TerminateIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "SIGABRT is left at its default under Rosetta, so there is no trace to match.";

    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceTerminateFunction();
    }, killedBy(SIGABRT), testing::AllOf(testing::HasSubstr(isWindows ? "std::terminate()" : isMac ? "terminating" : "terminate"),
                                         testing::HasSubstr("stackTraceTerminateFunction")));
}

UNIT_TEST(StackTrace, PureVirtualCallIsTraced) {
    if (detail::isRunningUnderRosetta())
        GTEST_SKIP() << "SIGABRT is left at its default under Rosetta, so there is no trace to match.";

    EXPECT_EXIT({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTracePureCallFunction();
    }, killedBy(sigPureCall()), testing::AllOf(testing::HasSubstr(isWindows ? "pure virtual function call" : "callPureIndirectly"),
                                               testing::HasSubstr("stackTracePureCallFunction")));
}

#ifdef _WINDOWS
UNIT_TEST(StackTrace, InvalidParameterIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceInvalidParameterFunction();
    }, testing::AllOf(testing::HasSubstr("invalid parameter passed to a CRT function"),
                      testing::HasSubstr("stackTraceInvalidParameterFunction")));
}
#endif // _WINDOWS

#endif // !__ANDROID__
