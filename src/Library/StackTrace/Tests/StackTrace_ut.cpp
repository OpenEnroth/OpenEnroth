#include <cstdio>
#include <cstdint>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <thread>
#include <unistd.h> // PROBE
#include <dlfcn.h> // PROBE
#ifdef __APPLE__
#   include <mach/mach.h> // PROBE
#endif
#include <sys/ucontext.h> // PROBE
#include <sys/wait.h> // PROBE

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

/**
 * Death tests in this suite re-exec the binary instead of forking it. The darwin_x86_64 leg runs under
 * Rosetta on the arm64 runners, and Rosetta plants its exception server thread in every translated process,
 * so gtest counts two threads there and warns that a forked child inherits whatever locks the other one held
 * at that instant. A re-exec'd child starts clean.
 */
class ThreadSafeDeathTest : public testing::Test {
 protected:
    void SetUp() override {
        GTEST_FLAG_SET(death_test_style, "threadsafe");
    }
};

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

MM_NOINLINE int stackTraceOverflowFunction2(int depth);

// PROBE: per-variant knobs and the state the alarm handler reports.
static volatile int probeDepth = 0;
static volatile uintptr_t probeSp = 0;
static volatile uintptr_t probeSp0 = 0;
static int probePrintEvery = 0;
static int probeSyscallEvery = 0;

static void probeOnAlarm(int, siginfo_t *, void *context) {
    const ucontext_t &uc = *static_cast<ucontext_t *>(context);
#if defined(__APPLE__) && defined(__x86_64__)
    size_t pc = uc.uc_mcontext->__ss.__rip, sp = uc.uc_mcontext->__ss.__rsp;
#elif defined(__APPLE__) && defined(__aarch64__)
    size_t pc = arm_thread_state64_get_pc(uc.uc_mcontext->__ss), sp = arm_thread_state64_get_sp(uc.uc_mcontext->__ss);
#elif defined(__aarch64__)
    size_t pc = uc.uc_mcontext.pc, sp = uc.uc_mcontext.sp;
#else
    size_t pc = uc.uc_mcontext.gregs[REG_RIP], sp = uc.uc_mcontext.gregs[REG_RSP];
#endif
    Dl_info info = {};
    dladdr(reinterpret_cast<void *>(pc), &info);
    char line[256];
    int n = std::snprintf(line, sizeof(line), "[probe alarm] wedged: pc=%#zx in %s sp=%#zx sp0=%#zx used=%zu depth=%d\n",
                          pc, info.dli_sname ? info.dli_sname : "?", sp, static_cast<size_t>(probeSp0),
                          static_cast<size_t>(probeSp0 - sp), probeDepth);
    (void) !write(1, line, n);
    _exit(42);
}

// PROBE: the real helper pair, byte for byte, with nothing added to the recursion.
MM_NOINLINE int probeRealOverflow2(int depth);

MM_NOINLINE int probeRealOverflow1(int depth) {
    volatile char pad[1024];
    pad[0] = static_cast<char>(depth);
    pad[1023] = static_cast<char>(depth);
    return pad[0] + pad[1023] + probeRealOverflow2(depth + 1);
}

MM_NOINLINE int probeRealOverflow2(int depth) {
    volatile char pad[1024];
    pad[0] = static_cast<char>(depth);
    pad[1023] = static_cast<char>(depth);
    return pad[0] + pad[1023] + probeRealOverflow1(depth + 1);
}

// PROBE: the real shape plus one global store per call.
MM_NOINLINE int probeStoreOverflow2(int depth);

MM_NOINLINE int probeStoreOverflow1(int depth) {
    probeDepth = depth;
    volatile char pad[1024];
    pad[0] = static_cast<char>(depth);
    pad[1023] = static_cast<char>(depth);
    return pad[0] + pad[1023] + probeStoreOverflow2(depth + 1);
}

MM_NOINLINE int probeStoreOverflow2(int depth) {
    probeDepth = depth;
    volatile char pad[1024];
    pad[0] = static_cast<char>(depth);
    pad[1023] = static_cast<char>(depth);
    return pad[0] + pad[1023] + probeStoreOverflow1(depth + 1);
}

// PROBE: the real shape plus one frame address store per call.
MM_NOINLINE int probeFrameOverflow2(int depth);

MM_NOINLINE int probeFrameOverflow1(int depth) {
    probeSp = reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
    volatile char pad[1024];
    pad[0] = static_cast<char>(depth);
    pad[1023] = static_cast<char>(depth);
    return pad[0] + pad[1023] + probeFrameOverflow2(depth + 1);
}

MM_NOINLINE int probeFrameOverflow2(int depth) {
    probeSp = reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
    volatile char pad[1024];
    pad[0] = static_cast<char>(depth);
    pad[1023] = static_cast<char>(depth);
    return pad[0] + pad[1023] + probeFrameOverflow1(depth + 1);
}

static void probeWatchdog(uintptr_t sp0) {
    sleep(20);
#ifdef __APPLE__
    thread_act_array_t threads = nullptr;
    mach_msg_type_number_t count = 0;
    task_threads(mach_task_self(), &threads, &count);
    for (mach_msg_type_number_t i = 0; i < count; i++) {
        if (threads[i] == mach_thread_self())
            continue;
        thread_basic_info_data_t basic = {};
        mach_msg_type_number_t basicCount = THREAD_BASIC_INFO_COUNT;
        thread_info(threads[i], THREAD_BASIC_INFO, reinterpret_cast<thread_info_t>(&basic), &basicCount);
        thread_extended_info_data_t extended = {};
        mach_msg_type_number_t extendedCount = THREAD_EXTENDED_INFO_COUNT;
        thread_info(threads[i], THREAD_EXTENDED_INFO, reinterpret_cast<thread_info_t>(&extended), &extendedCount);
        size_t pc = 0, sp = 0;
        kern_return_t stateResult = KERN_FAILURE;
#if defined(__x86_64__)
        x86_thread_state64_t state;
        mach_msg_type_number_t stateCount = x86_THREAD_STATE64_COUNT;
        stateResult = thread_get_state(threads[i], x86_THREAD_STATE64, reinterpret_cast<thread_state_t>(&state), &stateCount);
        if (stateResult == KERN_SUCCESS) { pc = state.__rip; sp = state.__rsp; }
#endif
        Dl_info info = {};
        if (pc) dladdr(reinterpret_cast<void *>(pc), &info);
        char line[320];
        int n = std::snprintf(line, sizeof(line),
            "[probe watchdog] thread %u name='%s' run_state=%d flags=%#x suspend=%d cpu=%d%% state=%d pc=%#zx in %s+%#zx sp=%#zx used=%zu\n",
            i, extended.pth_name, basic.run_state, basic.flags, basic.suspend_count, basic.cpu_usage / 10, stateResult, pc,
            info.dli_sname ? info.dli_sname : "?", info.dli_saddr ? pc - reinterpret_cast<size_t>(info.dli_saddr) : 0, sp, sp ? sp0 - sp : 0);
        (void) !write(1, line, n);
    }
#else
    (void) !write(1, "[probe watchdog] not apple\n", 27);
#endif
    _exit(42);
}

static void probeArm() {
    struct sigaction action;
    std::memset(&action, 0, sizeof(action));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = &probeOnAlarm;
    sigaction(SIGALRM, &action, nullptr);
    alarm(20);
}

MM_NOINLINE int stackTraceOverflowFunction1(int depth) {
    probeDepth = depth;
    probeSp = reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
    if (probePrintEvery && depth % probePrintEvery == 0) {
        char line[64];
        int n = std::snprintf(line, sizeof(line), "[probe depth] %d used=%zu\n", depth, static_cast<size_t>(probeSp0 - probeSp));
        (void) !write(1, line, n);
    }
    if (probeSyscallEvery && depth % probeSyscallEvery == 0)
        (void) getpid();
    volatile char pad[1024]; // Big frames overflow fast, and the volatile writes keep the endless recursion out of UB land.
    pad[0] = static_cast<char>(depth); // Touching both ends, or the compiler is free to shrink the array.
    pad[1023] = static_cast<char>(depth);
    return pad[0] + pad[1023] + stackTraceOverflowFunction2(depth + 1); // Mutual, or this folds into a loop that never overflows.
}

MM_NOINLINE int stackTraceOverflowFunction2(int depth) {
    volatile char pad[1024];
    pad[0] = static_cast<char>(depth);
    pad[1023] = static_cast<char>(depth);
    return pad[0] + pad[1023] + stackTraceOverflowFunction1(depth + 1);
}

UNIT_TEST(StackTrace, FunctionNamesAreResolved) {
    std::string trace = stackTraceMarkerFunction();

    EXPECT_THAT(trace, HasFrame(1, "stackTraceMarkerFunction"));
    EXPECT_CONTAINS(trace, "main");
}

UNIT_TEST_FIXTURE(ThreadSafeDeathTest, CrashHandlerNamesTheCrashingFunction) {
    EXPECT_DEATH({
        // Gtest wraps test bodies in __try/__except, and a frame-based handler runs before any unhandled
        // exception filter, so on windows ours would never see the access violation below.
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceCrashingFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceCrashingFunction"), testing::HasSubstr("main")));
}

UNIT_TEST_FIXTURE(ThreadSafeDeathTest, CrashOnAnotherThreadIsTraced) {
    // The handlers are process-wide, but only the thread that installs them gets an alternate signal stack,
    // so this one runs on the worker's own stack. That's enough for anything short of stack exhaustion.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        std::thread(stackTraceCrashingFunction).join();
    // A worker's stack ends at the thread entry, so main being absent is what says we traced the thread that
    // crashed rather than the one that installed the handlers.
    }, testing::AllOf(HasFrame(0, "stackTraceCrashingFunction"), testing::Not(testing::HasSubstr("main"))));
}

UNIT_TEST_FIXTURE(ThreadSafeDeathTest, NullFunctionCallIsTraced) {
    // Calling a null pointer faults at address zero, where there's nothing to unwind from. The call pushed its
    // return address first though, and walking on from that names the function that made the call and
    // everything above it. The walk used to follow the frame pointer slot of a frame that had none, past main
    // into garbage, and on i386 that crashed the handler before it printed a single frame.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceNullCallFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceNullCallFunction"), testing::HasSubstr("main")));
}

UNIT_TEST_FIXTURE(ThreadSafeDeathTest, BadTargetCallIsTraced) {
    // Calling 0xdeadbeefdead faults with the pc at the bad address, and the handler has to recognize that
    // to walk from the caller instead.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceBadTargetCallFunction();
    }, testing::AllOf(HasFrame(0, "stackTraceBadTargetCallFunction"), testing::HasSubstr("main")));
}

// PROBE: overflow death is a signal other than SIGALRM. The alarm handler exits 42 after reporting the depth.
static bool probeDiedOfTheOverflow(int status) {
    return WIFSIGNALED(status) && WTERMSIG(status) != SIGALRM;
}

#define PROBE_VARIANT(NAME, HANDLER, PRINT, FUNCTION)                                                           \
    UNIT_TEST_FIXTURE(ThreadSafeDeathTest, NAME) {                                                              \
        (void) !write(1, "[probe] " #NAME "\n", sizeof("[probe] " #NAME "\n") - 1);                             \
        EXPECT_EXIT({                                                                                           \
            GTEST_FLAG_SET(catch_exceptions, false);                                                            \
            probePrintEvery = PRINT;                                                                            \
            probeSp0 = reinterpret_cast<uintptr_t>(__builtin_frame_address(0));                                 \
            if (HANDLER) new StackTraceOnCrash();                                                               \
            probeArm();                                                                                         \
            std::thread(probeWatchdog, static_cast<uintptr_t>(probeSp0)).detach();                              \
            FUNCTION(0);                                                                                        \
        }, probeDiedOfTheOverflow, "");                                                                         \
    }

PROBE_VARIANT(Probe1_RealShape_Handler, true, 0, probeRealOverflow1)
PROBE_VARIANT(Probe2_RealShape_NoHandler, false, 0, probeRealOverflow1)
PROBE_VARIANT(Probe3_StorePerCall_Handler, true, 0, probeStoreOverflow1)
PROBE_VARIANT(Probe4_FrameAddressPerCall_Handler, true, 0, probeFrameOverflow1)
PROBE_VARIANT(Probe5_Instrumented_Handler_Silent, true, 0, stackTraceOverflowFunction1)

UNIT_TEST_FIXTURE(ThreadSafeDeathTest, CrashCallbackRunsAfterTheTrace) {
    // The callback is what holds a console window open after a crash, so it has to fire after the trace is
    // printed. Matching on order and not just presence is what guards that.
    auto callbackAfterTrace = testing::Truly([](const std::string &output) {
        size_t tracePos = output.find("stackTraceCrashingFunction");
        size_t callbackPos = output.find("crash callback ran");
        return tracePos != std::string::npos && callbackPos != std::string::npos && tracePos < callbackPos;
    });

    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler([] { std::fputs("crash callback ran", stderr); });
        stackTraceCrashingFunction();
    }, callbackAfterTrace);
}

UNIT_TEST_FIXTURE(ThreadSafeDeathTest, AbortIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceAbortFunction();
    }, testing::AllOf(testing::HasSubstr(isWindows ? "abort()" : isMac ? "Abort trap" : "abort"),
                      testing::HasSubstr("stackTraceAbortFunction")));
}

UNIT_TEST_FIXTURE(ThreadSafeDeathTest, TerminateIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceTerminateFunction();
    }, testing::AllOf(testing::HasSubstr(isWindows ? "std::terminate()" : isMac ? "terminating" : "terminate"),
                      testing::HasSubstr("stackTraceTerminateFunction")));
}

UNIT_TEST_FIXTURE(ThreadSafeDeathTest, PureVirtualCallIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTracePureCallFunction();
    }, testing::AllOf(testing::HasSubstr(isWindows ? "pure virtual function call" : "callPureIndirectly"),
                      testing::HasSubstr("stackTracePureCallFunction")));
}

#ifdef _WINDOWS
UNIT_TEST_FIXTURE(ThreadSafeDeathTest, InvalidParameterIsTraced) {
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        StackTraceOnCrash handler;
        stackTraceInvalidParameterFunction();
    }, testing::AllOf(testing::HasSubstr("invalid parameter passed to a CRT function"),
                      testing::HasSubstr("stackTraceInvalidParameterFunction")));
}
#endif // _WINDOWS

#endif // !__ANDROID__
