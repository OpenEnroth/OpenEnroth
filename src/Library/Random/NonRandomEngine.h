#pragma once

#include "RandomEngine.h"

class NonRandomEngine : public RandomEngine {
 public:
    virtual float RandomFloat() override {
        return Random(256) / 256.0f;
    }

    virtual uint32_t Random(uint32_t hi) override {
        return ++state_ % hi;
    }

    virtual void Seed(uint32_t seed) override {
        state_ = seed;
    }

 private:
    uint32_t state_ = 0;
};
