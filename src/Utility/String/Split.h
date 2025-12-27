#pragma once

#include <cassert>
#include <utility> // For std::forward.
#include <string>
#include <type_traits>
#include <algorithm> // For std::find.
#include <vector>
#include <ranges>

namespace detail {

class CharSplitter {
 public:
    using result_type = const char *;

    CharSplitter() = default;

    explicit CharSplitter(char sep) : _sep(sep) {}

    result_type split(const char *begin, const char *end) const {
        assert(begin <= end);
        return std::find(begin, end, _sep);
    }

    const char *begin(result_type result) const {
        return result;
    }

    const char *end(result_type result) const {
        return result + 1;
    }

    friend bool operator==(const CharSplitter &l, const CharSplitter &r) = default;

 private:
    char _sep = '\0';
};

class CrLfSplitter {
 public:
    using result_type = std::pair<const char *, const char *>;

    CrLfSplitter() = default;

    result_type split(const char *begin, const char *end) const {
        assert(begin <= end);
        const char *l = std::find(begin, end, '\n');
        const char *r = l + 1;
        if (l > begin && *(l - 1) == '\r')
            l--;
        return {l, r};
    }

    const char *begin(result_type result) const {
        return result.first;
    }

    const char *end(result_type result) const {
        return result.second;
    }

    friend bool operator==(const CrLfSplitter &l, const CrLfSplitter &r) = default;
};

class SplitViewSentinel {};

template<class Splitter>
class SplitViewIterator {
 public:
    using difference_type = std::ptrdiff_t; // Required for std::input_or_output_iterator, even though we can't take iterator difference...
    using value_type = std::string_view; // Required for std::indirectly_readable.

    SplitViewIterator() {
        _pos = reinterpret_cast<const char *>(1); // Construct as done.
    }

    SplitViewIterator(const char *begin, const char *end, const Splitter &splitter) : _pos(begin), _end(end), _splitter(splitter) {
        if (_pos <= _end)
            _sep = _splitter.split(_pos, _end);
    }

    SplitViewIterator &operator++() {
        assert(_pos <= _end); // Don't increment past-end iterator.
        _pos = _splitter.end(_sep);
        if (_pos <= _end)
            _sep = _splitter.split(_pos, _end);
        return *this;
    }

    SplitViewIterator operator++(int) {
        SplitViewIterator result = *this;
        operator++();
        return result;
    }

    std::string_view operator*() const {
        assert(_pos <= _end); // Don't dereference past-end iterator.
        return std::string_view(_pos, _splitter.begin(_sep));
    }

    friend bool operator==(const SplitViewIterator &l, const SplitViewSentinel &r) {
        return l._pos == l._end + 1;
    }

    friend bool operator==(const SplitViewIterator &l, const SplitViewIterator &r) {
        return l._pos == r._pos;
    }

 private:
    [[no_unique_address]] Splitter _splitter;
    const char *_pos = nullptr;
    const char *_end = nullptr;
    typename Splitter::result_type _sep;
};

/**
 * We use C++ and this is why we can't have nice things. It's 2024, we still can't split a string w/o jumping through
 * hoops. Just look at this: https://cplusplus.github.io/LWG/issue4017. One option is to use `std::views::drop` in a
 * hacky way to fix this. But then we run into the fact that `std::views::split` isn't even implemented in
 * AppleClang 14...
 *
 * So here is our own implementation. More user-friendly, and more efficient.
 */
template<class Splitter>
class SplitView : public std::ranges::view_interface<SplitView<Splitter>> {
 public:
    SplitView() {
        // Construct an empty SplitView. Note that a SplitView with _begin == _end is non-empty.
        _begin = reinterpret_cast<const char *>(1);
    }

    SplitView(std::string_view s, Splitter splitter) : _splitter(splitter), _begin(s.data()), _end(s.data() + s.size()) {}

    [[nodiscard]] auto begin() const {
        return SplitViewIterator<Splitter>(_begin, _end, _splitter);
    }

    [[nodiscard]] auto end() const {
        return SplitViewSentinel();
    }

    [[nodiscard]] std::string_view string() const {
        return std::string_view(_begin, _end);
    }

    // TODO(captainurist): #cpp23 this can go once we switch over to gcc-15, just use assign_range
    template<class Container>
    void to(Container *container) const {
        container->clear();
        for (std::string_view s : *this)
            container->emplace_back(s);
    }

    template<class Container>
    operator Container() const {
        // TODO(captainurist): return std::ranges::subrange(begin(), end()) | std::ranges::to<Container>();
        //                     #cpp23 we'll need gcc-15 for this to compile. Older implementation routes through
        //                     emplace(Iterator, Args) and this blows up for associative containers, in gcc-15
        //                     we get assign_range, and std::ranges::to calls it instead.
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
    [[no_unique_address]] Splitter _splitter;
    const char *_begin = nullptr;
    const char *_end = nullptr;
};

class SplitProxy {
 public:
    explicit SplitProxy(std::string_view s) : _s(s) {}

    [[nodiscard]] auto by(char sep) const {
        return SplitView(_s, CharSplitter(sep));
    }

    [[nodiscard]] auto byCrLf() const {
        return SplitView(_s, CrLfSplitter());
    }

 private:
    std::string_view _s;
};
} // namespace detail

#ifndef __DOXYGEN__ // Doxygen chokes here...
// Enable taking detail::SplitView by value, we handle dangling at split() level.
template<class Splitter>
inline constexpr bool std::ranges::enable_borrowed_range<detail::SplitView<Splitter>> = true;
#endif

// TODO(captainurist): drop!
std::vector<char*> tokenize(char *input, const char separator);

/**
 * Splits the provided string `s`, returning a proxy object with `.by(char)` and `.byCrLf()` methods.
 *
 * Usage:
 * - `split(s).by('c')` - split by character separator.
 * - `split(s).byCrLf()` - split by line endings (`\\n` or `\\r\\n`).
 *
 * Splitting a string doesn't discard empty chunks, so the resulting range will never be empty. Splitting an empty
 * string will produce a single empty chunk.
 *
 * If you need to store a result in a container there are two options:
 * - `split(s).by(';').to(&vector)` - reuses preallocated vector.
 * - `std::vector<std::string_view> v = split(s).by(';')` - allocates a new vector.
 *
 * Note that you can do the same with other container types, not just `std::vector`.
 *
 * @param s                             String to split.
 */
inline auto split(std::string_view s) {
    return detail::SplitProxy(s);
}

inline auto split(const char *s) {
    return split(std::string_view(s));
}

void split(std::string &&s) = delete; // Don't dangle!
