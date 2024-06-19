#include "StackTrace.h"

#include <cstdio>

#ifndef __ANDROID__
#   include <cpptrace/cpptrace.hpp>
#endif

#include "Utility/String/Format.h"

#ifdef __ANDROID__

void printStackTrace(FILE *stream) {
    fmt::println(stream, "Stack traces not supported on Android...");
}

#else

void printStackTrace(FILE *stream) {
    cpptrace::generate_trace().print_with_snippets();
}

#endif
