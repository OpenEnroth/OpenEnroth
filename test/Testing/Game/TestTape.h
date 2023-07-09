#pragma once

#include <cassert>
#include <algorithm>
#include <functional>
#include <ostream>
#include <vector>
#include <memory>
#include <utility>

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
} // namespace detail


template<class T>
class TestTape {
 public:
    explicit TestTape(std::shared_ptr<detail::TestTapeState<T>> state) : _state(std::move(state)) {
        assert(_state);
    }

    const std::vector<T> &values() const {
        return _state->values();
    }

    T delta() {
        assert(!values().empty());

        return values().back() - values().front();
    }

    template<class Y>
    friend bool operator==(const TestTape &l, const std::vector<Y> &r) {
        return std::equal(l.values().begin(), l.values().end(), r.begin(), r.end());
    }

    // operator!= and operators with switched arguments are auto-generated.

    friend void PrintTo(const TestTape &tape, std::ostream* stream) { // gtest printers support.
        using namespace testing; // NOLINT
        *stream << PrintToString(tape.values());
    }

 private:
    std::shared_ptr<detail::TestTapeState<T>> _state;
};

/**
 * Basically a convenient shortcut to create vectors that can then be compared with `TestTape` objects inside
 * the `EXPECT_EQ` and `ASSERT_EQ` gtest macros.
 *
 * Example code:
 * ```
 * EXPECT_EQ(strengthTape, tape(2, 4, 6)); // Two +2 strength barrels.
 * ```
 */
template<class T, class... Tail>
std::vector<T> tape(T first, Tail... tail) {
    return std::initializer_list<T>{std::move(first), std::move(tail)...};
}
