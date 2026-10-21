#pragma once

#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Format.h"
#include "Utility/Attributes.h"

#include "PathView.h"

/**
 * The vocabulary type for native paths.
 *
 * The path is stored as a string, WTF-8 on Windows and a byte string on POSIX. On Windows both slashes separate
 * components, and a path keeps whichever it was given until `normalized` spells them all as `/`. On POSIX a
 * backslash is an ordinary character in a file name.
 *
 * Path manipulation is lexical, and nothing is validated at `Path` level. Everything that asks the OS lives in
 * `Fs.h`, and that's where all validation lives too. On Linux file names are arbitrary byte strings and pass
 * through as they are, so `str` isn't necessarily valid UTF-8. On Windows it is the caller that keeps the string
 * valid WTF-8, and `native` turns an invalid sequence into a replacement character. On MacOS, APFS only takes file
 * names that are valid UTF-8.
 *
 * Unlike `std::filesystem::path`, nothing here depends on the C locale. `native` and `fromNative` do the charset
 * conversion, and on Windows the OS is only ever handed `wchar_t` strings.
 *
 * An empty path names nothing. A default-constructed `Path` holds one, `"" / "a"` is `"a"`, and the `fs::` calls
 * treat it as invalid.
 *
 * The lexical operations are `PathView`'s and documented there, and `Path` forwards them. What `Path` adds needs
 * the owned string: construction, `str`, `native`, and appending in place.
 */
class Path {
 public:
    Path() = default;

    /**
     * Implicit constructors from a string, with no charset conversion. A literal and a `std::string` need overloads
     * of their own, since converting through `std::string_view` would take two user-defined conversions. The
     * `std::string &&` one takes over the buffer.
     *
     * @param path                      Path string. WTF-8 on Windows, byte string on POSIX.
     */
    Path(std::string_view path) : _path(path) {} // NOLINT: intentionally implicit.
    Path(const char *path) : _path(path) {} // NOLINT: intentionally implicit.
    Path(const std::string &path) : _path(path) {} // NOLINT: intentionally implicit.
    Path(std::string &&path) : _path(std::move(path)) {} // NOLINT: intentionally implicit.

    explicit Path(PathView path) : _path(path.str()) {}

    /**
     * @param path                      Path as the OS spells it, a `wchar_t` string on Windows.
     * @return                          `Path` for the given string.
     */
#ifdef _WINDOWS
    [[nodiscard]] static Path fromNative(std::wstring_view path);
#else
    [[nodiscard]] static Path fromNative(std::string_view path);
#endif

    /**
     * @return                          This path as it was given, separators included, so on Windows it can hold both
     *                                  slashes. WTF-8 on Windows, byte string on POSIX.
     */
    [[nodiscard]] const std::string &str() const MM_LIFETIMEBOUND {
        return _path;
    }

    /**
     * @return                          This path as a string in the OS-native encoding, a `wchar_t` string on
     *                                  Windows. Separators stay as they are, since Windows APIs take both slashes,
     *                                  except in an extended-length or device path, where they all become
     *                                  backslashes.
     */
#ifdef _WINDOWS
    [[nodiscard]] std::wstring native() const;
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

    [[nodiscard]] Path operator/(PathView tail) const {
        return PathView(*this) / tail;
    }

    /**
     * @param tail                      Path to append, joined the same way `operator/` does it.
     * @return                          This path.
     */
    Path &operator/=(PathView tail) {
        return *this = PathView(*this) / tail;
    }

    friend auto operator<=>(const Path &l, const Path &r) = default;

    /**
     * CLI11 picks this function up through ADL, so that options can bind `Path` fields directly. Note that
     * `argv` is WTF-8 on Windows, where `UnicodeCrt` converts it from the wide command line, and a byte string on
     * POSIX.
     */
    friend bool lexical_cast(const std::string &input, Path &output) {
        output = Path(input);
        return true;
    }

 private:
    std::string _path;
};

inline PathView::PathView(const Path &path) : _path(path.str()) {}

template<>
struct std::hash<Path> {
    [[nodiscard]] size_t operator()(const Path &path) const {
        return std::hash<PathView>()(path);
    }
};

template<>
struct fmt::formatter<Path> : fmt::formatter<std::string> {
    auto format(const Path &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.str(), ctx);
    }
};
