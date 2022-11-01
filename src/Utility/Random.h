#pragma once

#include <cstdlib>
#include <cassert>
#include <iterator>
#include <initializer_list>

/**
 * @param range                         Random access range.
 * @return                              Random element from the provided range.
 */
template<class Range, class T = typename Range::value_type>
T Sample(const Range &range) {
    using std::begin;
    using std::end;

    auto b = begin(range);
    auto e = end(range);
    size_t size = e - b; // Assume random access iterators.

    assert(size > 0);

    return *(b + rand() % size);
}

template<class T>
T Sample(std::initializer_list<T> range) {
    return Sample<std::initializer_list<T>, T>(range);
}
