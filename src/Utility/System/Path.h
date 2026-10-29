#pragma once

#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Utility/String/Format.h"
#include "Utility/Attributes.h"

#include "PathView.h"

/**
 * The vocabulary type for native paths. The lexical operations and the string format are `PathView`'s and documented
 * there, and `Path` forwards them. What `Path` adds is what needs storage: taking over or copying a string, appending
 * in place, and `fromNative`.
 *
 * Everything that asks the OS lives in `Fs.h`, and that's where all validation lives too. On Linux file names are
 * arbitrary byte strings and pass through as they are, so `str` isn't necessarily valid UTF-8. On Windows it is the
 * caller that keeps the string valid WTF-8, and `native` turns an invalid sequence into a replacement character. On
 * macOS, APFS only takes file names that are valid UTF-8.
 *
 * Unlike `std::filesystem::path`, nothing here depends on the C locale. `native` and `fromNative` do the charset
 * conversion, and on Windows the OS is only ever handed `wchar_t` strings.
 *
 * An empty path names nothing. A default-constructed `Path` holds one, `"" / "a"` is `"a"`, and the `fs::` calls
 * treat it as invalid.
 */
class Path {
 public:
    Path() = default;

    /**
     * Constructors from a string, with no charset conversion. A literal and a `std::string` rvalue, whose buffer the
     * path takes over, convert implicitly. Any other string is copied through the explicit `PathView` constructor.
     *
     * @param path                      Path string.
     */
    Path(const char *path) : _path(path) {} // NOLINT: intentionally implicit.
    Path(std::string &&path) : _path(std::move(path)) {} // NOLINT: intentionally implicit.
    explicit Path(PathView path) : _path(path.str()) {}
    Path(std::nullptr_t) = delete; // NOLINT: implicit, or copy-initialization would fall back to const char *.

    /**
     * @param path                      Path as the OS spells it, a `wchar_t` string on Windows.
     * @return                          `path` converted to WTF-8 on Windows, as is on POSIX.
     */
#ifdef _WINDOWS
    [[nodiscard]] static Path fromNative(std::wstring_view path);
#else
    [[nodiscard]] static Path fromNative(std::string_view path);
#endif

    [[nodiscard]] const std::string &str() const MM_LIFETIMEBOUND {
        return _path;
    }

#ifdef _WINDOWS
    [[nodiscard]] std::wstring native() const {
        return PathView(*this).native();
    }
#else
    [[nodiscard]] const std::string &native() const MM_LIFETIMEBOUND {
        return _path;
    }
#endif

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    [[nodiscard]] std::string displayString() const {
        return PathView(*this).displayString();
    }

    [[nodiscard]] Path withExtension(std::string_view extension) const {
        return PathView(*this).withExtension(extension);
    }

    [[nodiscard]] std::string_view root() const MM_LIFETIMEBOUND {
        return PathView(*this).root();
    }

    [[nodiscard]] Path normalized() const {
        return PathView(*this).normalized();
    }

    [[nodiscard]] bool isNormalized() const {
        return PathView(*this).isNormalized();
    }

    [[nodiscard]] bool isEscaping() const {
        return PathView(*this).isEscaping();
    }

    [[nodiscard]] std::string_view name() const MM_LIFETIMEBOUND {
        return PathView(*this).name();
    }

    [[nodiscard]] std::string_view extension() const MM_LIFETIMEBOUND {
        return PathView(*this).extension();
    }

    [[nodiscard]] std::string_view stem() const MM_LIFETIMEBOUND {
        return PathView(*this).stem();
    }

    [[nodiscard]] PathView parent() const MM_LIFETIMEBOUND {
        return PathView(*this).parent();
    }

    [[nodiscard]] PathSplit split() const MM_LIFETIMEBOUND {
        return PathView(*this).split();
    }

    [[nodiscard]] PathView tailAt(std::same_as<std::string_view> auto segment) const MM_LIFETIMEBOUND {
        return PathView(*this).tailAt(segment);
    }

    [[nodiscard]] PathView tailAfter(std::same_as<std::string_view> auto segment) const MM_LIFETIMEBOUND {
        return PathView(*this).tailAfter(segment);
    }

    /**
     * @param tail                      Path to append, joined the same way `operator/` does it.
     * @return                          This path.
     */
    Path &operator/=(PathView tail);

    friend bool operator==(const Path &l, const std::convertible_to<PathView> auto &r) {
        return PathView(l) == PathView(r);
    }

    friend std::strong_ordering operator<=>(const Path &l, const std::convertible_to<PathView> auto &r) {
        return PathView(l) <=> PathView(r);
    }

    /**
     * Lets CLI11 options bind `Path` fields directly. CLI11 finds this function through ADL.
     *
     * @param input                     Command-line argument. WTF-8 on Windows, where `UnicodeCrt` converts it from
     *                                  the wide command line, and a byte string on POSIX.
     * @param[out] output               Path to set.
     * @return                          Always `true`.
     */
    friend bool lexical_cast(const std::string &input, Path &output) {
        output = Path(input);
        return true;
    }

 private:
    std::string _path;
};

inline PathView::PathView(const Path &path MM_LIFETIMEBOUND) : _path(path.str()) {}

namespace detail {
template<class T>
concept PathOrView = std::same_as<std::remove_cvref_t<T>, Path> || std::same_as<std::remove_cvref_t<T>, PathView>;

[[nodiscard]] Path joinPaths(PathView head, PathView tail);

/**
 * @param head                          Path that a tail without a root is about to be appended to.
 * @return                              How much of `head` the join keeps. A run of trailing separators shrinks to its
 *                                      first one, or to the root directory if it reaches it, so that `"//" / "x"`
 *                                      doesn't become the `"//x"` of a UNC root on Windows.
 */
[[nodiscard]] size_t joinedHeadSize(PathView head);
} // namespace detail

/**
 * Joins two paths. Either side may be any string, as long as one of them is a `Path` or a `PathView`.
 *
 * @param head                          Path to join onto.
 * @param tail                          Path to append.
 * @return                              `head` and `tail` with one separator between them, so `"a" / "b"` and
 *                                      `"a//" / "b"` are `"a/b"`, and `"a" / ""` is `"a/"`. An empty `head` adds
 *                                      none. A `tail` that is absolute or names another root replaces `head`, and
 *                                      one like `"/b"` keeps only the root name of `head`. A bare drive letter takes
 *                                      a separator too, so `"C:" / "x"` is `"C:/x"` and not the drive-relative
 *                                      `"C:x"`.
 */
template<class Head, class Tail>
    requires (detail::PathOrView<Head> || detail::PathOrView<Tail>) &&
             std::convertible_to<const Head &, PathView> && std::convertible_to<const Tail &, PathView>
[[nodiscard]] Path operator/(const Head &head, const Tail &tail) {
    return detail::joinPaths(PathView(head), PathView(tail));
}

template<>
struct std::hash<Path> : std::hash<PathView> {};

template<>
struct fmt::formatter<Path> : fmt::formatter<PathView> {};
