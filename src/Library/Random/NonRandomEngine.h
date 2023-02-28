#pragma once

#include <cassert>

#include "RandomEngine.h"

class NonRandomEngine : public RandomEngine {
 public:
    virtual float RandomFloat() override {
        return Random(256) / 256.0f;
    }

    virtual int Random(int hi) override {
        assert(hi > 0);

        return ++state_ % hi;
    }

    virtual void Seed(int seed) override {
        state_ = seed;
    }

 private:
    unsigned state_ = 0; // Using unsigned here so that it wraps around safely.
};
