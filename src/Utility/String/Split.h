#pragma once

#include <cassert>
#include <utility> // For std::forward.
#include <string>
#include <type_traits>
#include <algorithm> // For std::find.
#include <vector>

namespace detail {
class SplitViewSentinel {};

class SplitViewIterator {
 public:
    using difference_type = std::ptrdiff_t; // Required for std::input_or_output_iterator, even though we can't take iterator difference...
    using value_type = std::string_view; // Required for std::indirectly_readable.

    SplitViewIterator() {
        _pos++; // Construct an iterator that compares equal to `SplitViewSentinel`.
    }

    SplitViewIterator(const char *begin, const char *end, char sep) : _pos(begin), _end(end), _sep(sep) {}

    SplitViewIterator &operator++() {
        // This is an input iterator, all the work is done in `operator*`.
        return *this;
    }

    SplitViewIterator operator++(int) {
        // This is not supported because all the logic is in operator* (because we want less state in this iterator).
        // Just use prefix operator++ if you need it.
        assert(false);
        return *this;
    }

    std::string_view operator*() const {
        const char *next = std::find(_pos, _end, _sep);

        std::string_view result(_pos, next);

        _pos = next + 1;

        return result;
    }

    friend bool operator==(const SplitViewIterator &l, const SplitViewSentinel &r) {
        return l._pos == l._end + 1;
    }

    friend bool operator==(const SplitViewIterator &l, const SplitViewIterator &r) = default;

 private:
    mutable const char *_pos = nullptr; // std::indirectly_readable requires const operator*, so we need to make _pos mutable.
    const char *_end = nullptr;
    char _sep = '\0';
};

/**
 * We use C++ and this is why we can't have nice things. It's 2024, we still can't split a string w/o jumping through
 * hoops. Just look at this: https://cplusplus.github.io/LWG/issue4017. One option is to use `std::views::drop` in a
 * hacky way to fix this. But then we run into the fact that `std::views::split` isn't even implemented in
 * AppleClang 14...
 *
 * So here is our own implementation. More user-friendly, and more efficient.
 */
class SplitView : public std::ranges::view_interface<SplitView> {
 public:
    SplitView() {
        _begin++; // Construct an empty split view.
    }

    SplitView(std::string_view s, char sep) : _begin(s.data()), _end(s.data() + s.size()), _sep(sep) {}

    [[nodiscard]] auto begin() const {
        return SplitViewIterator(_begin, _end, _sep);
    }

    [[nodiscard]] auto end() const {
        return SplitViewSentinel();
    }

    template<class Container> requires std::is_same_v<std::remove_cv_t<typename Container::value_type::value_type>, char>
    operator Container() const {
        // Shamelessly stolen from std::ranges::to implementation. Only our code works for std::set, and
        // std::ranges::to doesn't.
        auto insert = []<class Element>(auto &container, Element &&element) {
            if constexpr (requires { container.emplace_back(std::forward<Element>(element)); }) {
                container.emplace_back(std::forward<Element>(element));
            } else if constexpr (requires { container.push_back(std::forward<Element>(element)); }) {
                container.push_back(std::forward<Element>(element));
            } else if constexpr (requires { container.emplace(std::forward<Element>(element)); }) {
                container.emplace(std::forward<Element>(element));
            } else {
                container.insert(std::forward<Element>(element));
            }
        }; // NOLINT

        Container result;
        for (std::string_view chunk : *this)
            insert(result, chunk);
        return result;
    }

 private:
    const char *_begin = nullptr;
    const char *_end = nullptr;
    char _sep = '\0';
};
} // namespace detail


// TODO(captainurist): drop!
std::vector<char*> tokenize(char *input, const char separator);

/**
 * Splits the provided string `s` using separator `sep`, returning a range of `std::string_view` chunks.
 *
 * This function doesn't discard empty chunks, so the returned range will never be empty. Splitting an empty string
 * will produce a single empty chunk.
 *
 * @param s                             String to split.
 * @param sep                           Separator character.
 */
inline auto split(std::string_view s, char sep) {
    return detail::SplitView(s, sep);
}

inline auto split(const char *s, char sep) {
    return split(std::string_view(s), sep);
}

void split(std::string &&s, char sep) = delete; // Don't dangle!

void split(std::string_view s, char sep, std::vector<std::string_view> *result);
