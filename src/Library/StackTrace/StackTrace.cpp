#include "StackTrace.h"

#include <cstdio>
#include <string>

#ifndef __ANDROID__
#   include <cpptrace/cpptrace.hpp>
#endif

#include "Utility/String/Format.h"

#ifdef __ANDROID__

std::string stackTraceToString() {
    return "Stack traces not supported on Android...";
}

#else

std::string stackTraceToString() {
    return cpptrace::generate_trace(0, MAX_TRACE_DEPTH).to_string();
}

#endif

void printStackTrace(FILE *stream) {
    fmt::println(stream, "{}", stackTraceToString());
}
