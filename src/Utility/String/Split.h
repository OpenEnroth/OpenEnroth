#pragma once

#include <cassert>
#include <cstring> // For memchr, memcmp.
#include <string>
#include <type_traits>
#include <algorithm> // For std::find.
#include <vector>
#include <ranges>

namespace detail {
template<class T>
struct is_initializer_list : std::false_type {};

template<class T>
struct is_initializer_list<std::initializer_list<T>> : std::true_type {};

template<class T>
inline constexpr bool is_initializer_list_v = is_initializer_list<T>::value;

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

/**
 * Splitter for multi-character separators, e.g. `"\r\n"`.
 *
 * Uses `memchr` for the first character followed by `memcmp` for the rest. We're not using `std::search` here because
 * `memchr` is SIMD-optimized and ~2x faster on the data sizes we care about.
 *
 * String searching can be done way more efficiently (e.g. see Volnitsky algo as used in ClickHouse), but what we have
 * here is already good enough.
 */
class StringSplitter {
 public:
    using result_type = const char *;

    StringSplitter() = default;

    explicit StringSplitter(std::string_view sep) : _sep(sep) {
        assert(!sep.empty());
    }

    result_type split(const char *begin, const char *end) const {
        assert(begin <= end);
        while (begin <= end - static_cast<std::ptrdiff_t>(_sep.size())) {
            const char *p = static_cast<const char *>(memchr(begin, _sep[0], end - begin));
            if (!p || p > end - static_cast<std::ptrdiff_t>(_sep.size()))
                return end;
            if (memcmp(p, _sep.data(), _sep.size()) == 0)
                return p;
            begin = p + 1;
        }
        return end;
    }

    const char *begin(result_type result) const {
        return result;
    }

    const char *end(result_type result) const {
        return result + _sep.size();
    }

    friend bool operator==(const StringSplitter &l, const StringSplitter &r) = default;

 private:
    std::string_view _sep;
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
        return l._pos > l._end;
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
 * We use C++ and this is why we can't have nice things. Just look at this: https://cplusplus.github.io/LWG/issue4017. 
 * One option is to use `std::views::drop` in a hacky way to fix this. But then we run into the fact that 
 * `std::views::split` isn't even implemented in AppleClang 14...
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

    [[nodiscard]] std::string_view str() const {
        return std::string_view(_begin, _end);
    }

    template<class Container>
    void to(Container *container) const {
        // We can't use assign_range / insert_range / std::ranges::to here because:
        // - assign_range / insert_range require implicit convertibility, and string_view -> string is explicit.
        // - on top of that, std::ranges::to would enter infinite recursion through operator Container() below.
        // So we use a manual loop with emplace_back / emplace.
        container->clear();
        for (std::string_view chunk : *this) {
            if constexpr (requires { container->emplace_back(chunk); })
                container->emplace_back(chunk);
            else
                container->emplace(chunk);
        }
    }

    /**
     * Converts this view to a container of string-like elements.
     *
     * This operator enables implicit conversion to various container types (e.g., `std::vector<std::string>`,
     * `std::set<std::string_view>`) for convenient use in initialization and assignment contexts.
     *
     * The `initializer_list` exclusion in the requires clause is necessary to prevent ambiguous overload resolution
     * when assigning to containers. Without it, `v = split(...)` would be ambiguous because `std::vector::operator=`
     * has overloads for both `const vector&` and `initializer_list<value_type>`. Since `initializer_list<string_view>`
     * technically satisfies the `Container::value_type::value_type == char` constraint, the compiler would consider
     * both conversion paths and fail with an ambiguity error.
     *
     * @tparam Container                    Target container type. Must hold string-like elements.
     * @return                              Container populated with the string chunks from this view.
     */
    template<class Container> requires
        std::is_same_v<std::remove_cv_t<typename Container::value_type::value_type>, char> &&
        (!is_initializer_list_v<Container>)
    operator Container() const {
        Container result;
        to(&result);
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

    [[nodiscard]] auto by(std::string_view sep) const {
        return SplitView(_s, StringSplitter(sep));
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

/**
 * Splits the provided string `s`, returning a proxy object with `.by(char)` and `.by(std::string_view)` methods.
 *
 * Usage:
 * - `split(s).by('c')` - split by a single character separator.
 * - `split(s).by("\\r\\n")` - split by a multi-character separator.
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
