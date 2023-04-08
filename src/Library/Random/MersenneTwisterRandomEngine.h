#pragma once

#include <cassert>
#include <random>

#include "RandomEngine.h"

class MersenneTwisterRandomEngine: public RandomEngine {
 public:
    virtual float randomFloat() override {
        return std::uniform_real_distribution<float>(0.0f, 1.0f)(_base);
    }

    virtual int random(int hi) override {
        assert(hi > 0);

        return static_cast<int>((static_cast<uint64_t>(_base()) * static_cast<uint64_t>(hi)) >> 32);
    }

    virtual void seed(int seed) override {
        _base.seed(seed);
    }

 private:
    std::mt19937 _base;
};
