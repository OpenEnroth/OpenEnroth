#pragma once

#include <cassert>

#include "RandomEngine.h"

class SequentialRandomEngine : public RandomEngine {
 public:
    virtual float randomFloat() override {
        return random(256) / 256.0f;
    }

    virtual int random(int hi) override {
        assert(hi > 0);

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
