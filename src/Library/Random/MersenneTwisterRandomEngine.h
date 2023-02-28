#pragma once

#include <cassert>
#include <random>

#include "RandomEngine.h"

class MersenneTwisterRandomEngine: public RandomEngine {
 public:
    virtual float RandomFloat() override {
        return std::uniform_real_distribution<float>(0.0f, 1.0f)(base_);
    }

    virtual int Random(int hi) override {
        assert(hi > 0);

        return static_cast<int>((static_cast<uint64_t>(base_()) * static_cast<uint64_t>(hi)) >> 32);
    }

    virtual void Seed(int seed) override {
        base_.seed(seed);
    }

 private:
    std::mt19937 base_;
};
