#pragma once

#include <cassert>

#include "RandomEngine.h"

#include "backward.hpp"

#include "Utility/Format.h"

#include "Engine/EngineGlobals.h" // Ugh
#include "Library/Platform/Interface/Platform.h"
#include "Library/Platform/Application/PlatformApplication.h"

class SequentialRandomEngine : public RandomEngine {
 public:
    virtual float randomFloat() override {
        return random(256) / 256.0f;
    }

    virtual int random(int hi) override {
        assert(hi > 0);

#if 1
        fmt::print(stderr, "Random called at {}ms, returning {}, stacktrace:\n", application->platform()->tickCount(), _state + 1);

        //using namespace backward;
        backward::StackTrace st;
        st.load_here(32);

        backward::TraceResolver resolver;
        resolver.load_stacktrace(st);

        for (size_t trace_idx = st.size(); trace_idx > 0; --trace_idx) {
            backward::ResolvedTrace frame = resolver.resolve(st[trace_idx - 1]);

            fmt::print(stderr, "#{: <2} {}\n", frame.idx, frame.object_function);
        }
#endif

        return ++_state % hi;
    }

    virtual int peek(int hi) const override {
        assert(hi > 0);

        return (_state + 1) % hi;
    }

    virtual void seed(int seed) override {
        _state = seed;
    }

 private:
    unsigned _state = 0; // Using unsigned here so that it wraps around safely.
};
