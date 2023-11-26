#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <memory>
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
    virtual float randomFloat() = 0;

    /**
     * @param hi                        Upper bound for the result. Must be greater than zero.
     * @return                          Random number in range `[0, hi)`.
     */
    virtual int random(int hi) = 0;

    /**
     * Note that this method is mainly for debugging, implementation is not required to be efficient.
     *
     * @param hi                        Upper bound for the result. Must be greater than zero.
     * @return                          Random number that the next call to `random(hi)` will return.
     */
    virtual int peek(int hi) const = 0;

    /**
     * Reinitializes this random engine with the provided seed value. Passing `0` should be equivalent to calling
     * an in-place destructor and then reconstructing the object.
     *
     * @param seed                      Random seed.
     */
    virtual void seed(int seed) = 0;

    /**
     * @param min                       Minimal result value.
     * @param max                       Maximal result value. Must be greater or equal to `min`.
     * @return                          Random number in closed interval `[min, max]`. Note that both `min` and
     *                                  `max` are included in the interval.
     */
    int randomInSegment(int min, int max);

    /**
     * @offset 0x00452B2E
     *
     * @param count                     Number of dice to throw.
     * @param faces                     Number of faces on each die.
     * @return                          Result of throwing the dice.
     */
    int randomDice(int count, int faces);

    /**
     * @return                          Random binary value.
     */
    bool randomBool() {
        return random(2) != 0;
    }

    /**
     * @param range                     Random access range.
     * @return                          Random element from the provided range.
     */
    template<class Range, class T = typename Range::value_type>
    T randomSample(const Range &range) {
        using std::begin;
        using std::end;

        auto b = begin(range);
        auto e = end(range);
        size_t size = e - b; // Assume random access iterators.

        assert(size > 0);

        return *(b + random(size));
    }

    template<class T>
    T randomSample(std::initializer_list<T> range) {
        return randomSample<std::initializer_list<T>, T>(range);
    }
};
