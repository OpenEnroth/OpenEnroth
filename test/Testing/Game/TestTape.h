#pragma once

#include <cassert>
#include <algorithm>
#include <functional>
#include <ostream>
#include <memory>
#include <utility>

#include "TestVector.h"

namespace testing {} // Forward-declare gtest namespace.

namespace detail {
template<class T>
class TestTapeState {
 public:
    explicit TestTapeState(std::function<T()> callback) : _callback(std::move(callback)) {
        assert(_callback);
    }

    void tick() {
        T value = _callback();
        if (_values.empty() || _values.back() != value)
            _values.push_back(std::move(value));
    }

    const TestVector<T> &values() const {
        return _values;
    }

 private:
    std::function<T()> _callback;
    TestVector<T> _values;
};
} // namespace detail


template<class T>
class TestTape {
 public:
    explicit TestTape(std::shared_ptr<detail::TestTapeState<T>> state) : _state(std::move(state)) {
        assert(_state);
    }

    const TestVector<T> &values() const {
        return _state->values();
    }

    TestVector<T> firstLast() const {
        assert(!values().empty());
        TestVector<T> result;
        result.push_back(values().front());
        result.push_back(values().back());
        return result;
    }

    int size() const {
        return values().size();
    }

    T delta() {
        assert(!values().empty());
        return values().back() - values().front();
    }

    T min() const {
        assert(!values().empty());
        return *std::min_element(values().begin(), values().end());
    }

    T max() const {
        assert(!values().empty());
        return *std::max_element(values().begin(), values().end());
    }

    bool contains(T value) {
        return std::find(values().begin(), values().end(), value) != values().end();
    }

    template<class Y>
    friend bool operator==(const TestTape &l, const TestVector<Y> &r) {
        return l.values() == r;
    }

    // operator!= and operators with switched arguments are auto-generated.

    friend void PrintTo(const TestTape &tape, std::ostream* stream) { // gtest printers support.
        using namespace testing; // NOLINT
        *stream << PrintToString(tape.values());
    }

 private:
    std::shared_ptr<detail::TestTapeState<T>> _state;
};

template<class T>
using TestMultiTape = TestTape<TestVector<T>>;

/**
 * Shortcut function to create vectors that can then be compared with `TestTape` objects inside
 * the `EXPECT_EQ` and `ASSERT_EQ` gtest macros.
 *
 * Example code:
 * ```
 * EXPECT_EQ(strengthTape, tape(2, 4, 6)); // Two +2 strength barrels.
 * ```
 */
template<class T, class... Tail>
TestVector<T> tape(T first, Tail... tail) {
    return std::initializer_list<T>{std::move(first), std::move(tail)...};
}

/**
 * Same as the other `tape` overload, but for per-character tapes.
 *
 * Example code:
 * ```
 * EXPECT_EQ(expTape.firstLast(), tape({100, 100, 100, 100}, {254, 250, 254, 254}));
 * ```
 */
template<class T, class... Tail>
TestVector<TestVector<T>> tape(std::initializer_list<T> first, std::initializer_list<Tail>... tail) {
    return std::initializer_list<TestVector<T>>{first, tail...};
}
