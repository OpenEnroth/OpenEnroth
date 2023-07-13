#pragma once

#include <vector>
#include <algorithm>

/**
 * Vector wrapper that provides operator== for instantiations that have different element types. Used only in tests.
 */
template<class T>
class TestVector : public std::vector<T> {
    using base_type = std::vector<T>;
 public:
    using base_type::base_type;

    template<class Y>
    friend bool operator==(const TestVector &l, const TestVector<Y> &r) {
        return std::equal(l.begin(), l.end(), r.begin(), r.end());
    }
};
