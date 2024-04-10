#include "StackTraceOnCrash.h"

#include <memory>

#ifndef __ANDROID__
#   include <backward.hpp>
#endif

#ifdef __ANDROID__

StackTraceOnCrash::StackTraceOnCrash() = default;

#else

StackTraceOnCrash::StackTraceOnCrash() {
    _private = std::make_shared<backward::SignalHandling>();
}

#endif
