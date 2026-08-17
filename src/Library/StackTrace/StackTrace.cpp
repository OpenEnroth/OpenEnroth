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
    return cpptrace::generate_trace(1).to_string(); // Skip this function itself.
}

#endif

void printStackTrace(FILE *stream) {
    fmt::println(stream, "{}", stackTraceToString());
}
