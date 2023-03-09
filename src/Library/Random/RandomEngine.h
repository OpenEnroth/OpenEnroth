#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <initializer_list>

/**
 * Random number generator interface.
 *
 * Note that the methods of this class are NOT thread-safe, unlike their libc counterparts.
 */
class RandomEngine {
 public:
    virtual ~RandomEngine() = default;

    /**
     * @return                          Random floating point number in range `[0, 1)`.
     */
    virtual float RandomFloat() = 0;

    /**
     * @param hi                        Upper bound for the result. Must be greater than zero.
     * @return                          Random number in range `[0, hi)`.
     */
    virtual int Random(int hi) = 0;

    /**
     * Reinitializes this random engine with the provided seed value.
     *
     * @param seed                      Random seed.
     */
    virtual void Seed(int seed) = 0;

    /**
     * @param min                       Minimal result value.
     * @param max                       Maximal result value. Must be greater or equal to `min`.
     * @return                          Random number in closed interval `[min, max]`. Note that both `min` and
     *                                  `max` are included in the interval.
     */
    int RandomInSegment(int min, int max);

    /**
     * @offset 0x00452B2E
     *
     * @param count                     Number of dice to throw.
     * @param faces                     Number of faces on each die.
     * @return                          Result of throwing the dice.
     */
    int RandomDice(int count, int faces);

    /**
     * Create random binary value.
     */
    bool RandomBool() {
        return Random(2) != 0;
    }

    /**
     * @param range                     Random access range.
     * @return                          Random element from the provided range.
     */
    template<class Range, class T = typename Range::value_type>
    T RandomSample(const Range &range) {
        using std::begin;
        using std::end;

        auto b = begin(range);
        auto e = end(range);
        size_t size = e - b; // Assume random access iterators.

        assert(size > 0);

        return *(b + Random(size));
    }

    template<class T>
    T RandomSample(std::initializer_list<T> range) {
        return RandomSample<std::initializer_list<T>, T>(range);
    }
};
