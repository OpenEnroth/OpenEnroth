#pragma once

#include <cassert>
#include <algorithm>
#include <functional>
#include <ostream>
#include <memory>
#include <utility>
#include <vector>

#include "AccessibleVector.h"

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

    const std::vector<T> &values() const {
        return _values;
    }

 private:
    std::function<T()> _callback;
    std::vector<T> _values;
};

template<class T>
class TestTapeBase {
 public:
    explicit TestTapeBase(std::shared_ptr<detail::TestTapeState<T>> state) : _state(std::move(state)) {
        assert(_state);
    }

    auto begin() const {
        return _state->values().begin();
    }

    auto end() const {
        return _state->values().end();
    }

    friend void PrintTo(const TestTapeBase &tape, std::ostream* stream) { // gtest printers support.
        using namespace testing; // NOLINT
        *stream << PrintToString(tape.values());
    }

 private:
    std::shared_ptr<detail::TestTapeState<T>> _state;
};

} // namespace detail


template<class T>
using TestTape = Accessible<detail::TestTapeBase<T>>;

template<class T>
using TestMultiTape = TestTape<AccessibleVector<T>>;

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
AccessibleVector<T> tape(T first, Tail... tail) {
    return std::initializer_list<T>{std::move(first), std::move(tail)...};
}

/**
 * Same as the other `tape` overload, but for per-character tapes.
 *
 * Example code:
 * ```
 * EXPECT_EQ(expTape.frontBack(), tape({100, 100, 100, 100}, {254, 250, 254, 254}));
 * ```
 */
template<class T, class... Tail>
AccessibleVector<AccessibleVector<T>> tape(std::initializer_list<T> first, std::initializer_list<Tail>... tail) {
    return std::initializer_list<AccessibleVector<T>>{first, tail...};
}
