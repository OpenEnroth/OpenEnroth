#include "StackTrace.h"

#include <cstdio>
#include <string>

#ifndef __ANDROID__
#   include <cpptrace/cpptrace.hpp>
#endif

#include "Utility/Attributes.h"
#include "Utility/String/Format.h"

#ifdef __ANDROID__

std::string stackTraceToString() {
    return "Stack traces not supported on Android...";
}

#else

// Deep enough for any call stack we'd want to read, shallow enough that a runaway recursion prints a trace
// instead of megabytes of one. Backward-cpp used to stop at 32.
constexpr size_t MAX_TRACE_DEPTH = 128;

MM_NOINLINE std::string stackTraceToString() {
    return cpptrace::generate_trace(1, MAX_TRACE_DEPTH).to_string(); // Skip this function itself, hence the noinline.
}

#endif

void printStackTrace(FILE *stream) {
    fmt::println(stream, "{}", stackTraceToString());
}
