#include "StackTraceOnCrash.h"

#include <memory>

#ifndef __ANDROID__
#   include <cpptrace/cpptrace.hpp>
#endif

#ifdef __ANDROID__

StackTraceOnCrash::StackTraceOnCrash() = default;

#else

StackTraceOnCrash::StackTraceOnCrash() {
    //_private = std::make_shared<backward::SignalHandling>();
}

#endif
