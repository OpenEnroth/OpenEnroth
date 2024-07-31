#include "TracingRandomEngine.h"

#include <cassert>

#include "Library/Platform/Interface/Platform.h"
#include "Library/StackTrace/StackTrace.h"

#include "Utility/String/Format.h"

TracingRandomEngine::TracingRandomEngine(Platform *platform, RandomEngine *base): _platform(platform), _base(base) {
    assert(platform);
    assert(base);
}

float TracingRandomEngine::randomFloat() {
    float result = _base->randomFloat();
    printTrace("randomFloat", result);
    return result;
}

int TracingRandomEngine::random(int hi) {
    int result = _base->random(hi);
    printTrace("random", result);
    return result;
}

int TracingRandomEngine::peek(int hi) const {
    return _base->peek(hi);
}

void TracingRandomEngine::seed(int seed) {
    _base->seed(seed);
}

template<class T>
void TracingRandomEngine::printTrace(const char *function, const T &value) const {
    fmt::println(stderr, "TracingRandomEngine::{} called at {}ms, returning {}, stacktrace:",
                 function, _platform->tickCount(), value);
    printStackTrace(stderr);
}
