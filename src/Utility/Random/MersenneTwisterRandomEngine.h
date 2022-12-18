#pragma once

#include <cassert>
#include <random>

#include "RandomEngine.h"

class MersenneTwisterRandomEngine: public RandomEngine {
 public:
    virtual uint32_t Random() override {
        return Base_();
    }

    virtual uint32_t Random(uint32_t hi) override {
        assert(hi > 0);

        return (static_cast<uint64_t>(Base_()) * hi) >> 32;
    }

    virtual void Seed(uint32_t seed) override {
        Base_.seed(seed);
    }

 private:
    std::mt19937 Base_;
};
