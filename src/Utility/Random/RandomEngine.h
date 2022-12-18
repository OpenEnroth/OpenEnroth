#pragma once

#include <cstdint>

class RandomEngine {
 public:
    /**
     * @param hi                        Upper bound for the result. Must be greater than zero.
     * @return                          Randomly generated number in range `[0, hi)`.
     */
    virtual uint32_t Random(uint32_t hi) = 0;

    /**
     * Reinitializes this random engine with the provided seed value.
     *
     * @param seed                      Random seed.
     */
    virtual void Seed(uint32_t seed) = 0;
};