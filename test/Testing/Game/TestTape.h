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
/**
 * Vector wrapper that can be compared via operator== with other vectors that have different element types.
 */
template<class T>
class ComparableVector : public std::vector<T> {
    using base_type = std::vector<T>;
 public:
    using base_type::base_type;

    template<class Y>
    friend bool operator==(const ComparableVector &l, const ComparableVector<Y> &r) {
        return std::equal(l.begin(), l.end(), r.begin(), r.end());
    }
};

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

    const ComparableVector<T> &values() const {
        return _values;
    }

 private:
    std::function<T()> _callback;
    ComparableVector<T> _values;
};
} // namespace detail


template<class T>
class TestTape {
 public:
    explicit TestTape(std::shared_ptr<detail::TestTapeState<T>> state) : _state(std::move(state)) {
        assert(_state);
    }

    const detail::ComparableVector<T> &values() const {
        return _state->values();
    }

    detail::ComparableVector<T> firstLast() const {
        detail::ComparableVector<T> result;
        result.push_back(values().front());
        result.push_back(values().back());
        return result;
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
    friend bool operator==(const TestTape &l, const detail::ComparableVector<Y> &r) {
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
detail::ComparableVector<T> tape(T first, Tail... tail) {
    return std::initializer_list<T>{std::move(first), std::move(tail)...};
}
