#include "StackTrace.h"

#include <cstdio>

#ifndef __ANDROID__
#   include <backward.hpp>
#endif

#include "Utility/String/Format.h"

#ifdef __ANDROID__

void printStackTrace(FILE *stream) {
    fmt::println(stream, "Stack traces not supported on Android...");
}

#else

void printStackTrace(FILE *stream) {
    backward::StackTrace trace;
    trace.load_here(32);
    backward::TraceResolver resolver;
    resolver.load_stacktrace(trace);

    for (size_t i = 0; i < trace.size(); i++) {
        backward::ResolvedTrace frame = resolver.resolve(trace[i]);
        fmt::println(stream, "#{: <2} {}", frame.idx, frame.object_function);
    }
}

#endif
