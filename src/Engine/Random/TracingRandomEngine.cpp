#include "TracingRandomEngine.h"

#include <cassert>
#include <utility>

#include "Engine/EngineGlobals.h"

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/StackTrace/StackTrace.h"

#include "Utility/Format.h"

TracingRandomEngine::TracingRandomEngine(std::unique_ptr<RandomEngine> base) {
    assert(base);
    _base = std::move(base);
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
                 function, application->platform()->tickCount(), value);
    printStackTrace(stderr);
}
