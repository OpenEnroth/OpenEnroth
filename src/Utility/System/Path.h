#pragma once

#include <compare>
#include <string>
#include <string_view>

#include "Utility/String/Format.h"

#include "PathView.h"

/**
 * The repo's vocabulary type for native paths - everything that takes a native path takes a `Path`.
 *
 * The path is stored as a string, WTF-8 on Windows and a byte string on POSIX, and path manipulation is lexical.
 * Everything that asks the OS lives in `Fs.h`. Backslashes are converted to forward slashes on Windows, where both
 * slashes separate path components. On POSIX a backslash is an ordinary character in a file name, so it is left
 * alone. Otherwise a path keeps the bytes it was built from, and `normalized` brings it to normal form.
 *
 * Unlike `std::filesystem::path`, this class does not depend on the C locale. On Windows constructing an
 * `std::filesystem::path` from a narrow string converts it per the C locale, while here all charset conversions are
 * done by our own code, and the OS is only ever handed `wchar_t` strings. `native` / `fromNative` are the conversions
 * to use when talking to the OS.
 *
 * An empty path names nothing. It is what a default-constructed `Path` holds, and joining it is a no-op, so
 * `"" / "a"` is `"a"`. The `fs::` calls treat it as invalid.
 *
 * File names on Linux are arbitrary byte strings, and these bytes are passed through as-is, so the string returned
 * by `str` is not necessarily valid UTF-8, and not even necessarily valid WTF-8. Nothing is validated on the way
 * in either, so on Windows it is the caller that keeps the string valid WTF-8, and `native` is where an invalid
 * sequence turns into a replacement character. And MacOS is different again, APFS only takes file names that are
 * valid UTF-8.
 */
class Path {
 public:
    Path() = default;

    /**
     * Implicit constructors from a string, same as `std::filesystem::path`. No charset conversion is performed. On
     * Windows backslashes become forward slashes.
     *
     * A string literal and a `std::string` get overloads of their own, since going through `std::string_view` would
     * need two user-defined conversions, and that's ill-formed. The `std::string &&` one takes over the buffer
     * instead of copying it.
     *
     * @param path                      Path string. WTF-8 on Windows, byte string on POSIX.
     */
    Path(std::string_view path); // NOLINT: intentionally implicit.
    Path(const char *path) : Path(std::string_view(path)) {} // NOLINT: intentionally implicit.
    Path(const std::string &path) : Path(std::string_view(path)) {} // NOLINT: intentionally implicit.
    Path(std::string &&path); // NOLINT: intentionally implicit.

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
     * @return                          This path as a string, always using forward slashes. WTF-8 on Windows,
     *                                  byte string on POSIX.
     */
    [[nodiscard]] const std::string &str() const {
        return _path;
    }

    /**
     * @return                          This path as a string in the OS-native encoding, a `wchar_t` string on
     *                                  Windows. Separators stay forward slashes, which Windows APIs accept, except
     *                                  in an extended-length or device path, which gets backslashes.
     */
#ifdef _WINDOWS
    [[nodiscard]] std::wstring native() const;
#else
    [[nodiscard]] const std::string &native() const {
        return _path;
    }
#endif

    /**
     * @return                          This path as a valid UTF-8 string for displaying to the user, with everything
     *                                  that's not valid UTF-8 replaced with U+FFFD. Unlike the string returned by
     *                                  `str`, it might not round-trip back into the same path.
     */
    [[nodiscard]] std::string displayString() const;

    /**
     * @param extension                 New extension, with or without the leading dot. Pass an empty string to drop
     *                                  the extension. WTF-8 on Windows, byte string on POSIX.
     * @return                          Copy of this path with the extension replaced. Only the last extension is
     *                                  replaced, so `"a.tar.gz"` with `".zip"` becomes `"a.tar.zip"`. A dotfile such
     *                                  as `".bashrc"`, or a name like `"..."` whose stem would be all dots, has no
     *                                  extension. A path without a file name gets the extension as one.
     */
    [[nodiscard]] Path withExtension(std::string_view extension) const;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    /**
     * @return                          The root name and root directory this path starts with, empty if there are
     *                                  none. That's `"/"`, or `"C:"`, `"C:/"` and `"//server/share/"` on Windows.
     */
    [[nodiscard]] std::string_view root() const {
        return PathView(*this).root();
    }

    /**
     * @return                          Copy of this path in lexical normal form. The root is kept as it is, then
     *                                  come single separators with no trailing one, no `.` segments, and `..`
     *                                  collapsed. A `..` survives only as the leading run of a path without a root
     *                                  directory, and above a root directory it's dropped, so `"/.."` is `"/"`. A lone
     *                                  `"."` normalizes to the empty path.
     */
    [[nodiscard]] Path normalized() const;

    /**
     * @return                          Whether `normalized` would return this path unchanged. Allocates nothing.
     */
    [[nodiscard]] bool isNormalized() const {
        return PathView(*this).isNormalized();
    }

    /**
     * @return                          Whether this path points above its starting point. `".."` and `"a/../.."` do,
     *                                  `"a/../b"` doesn't. Always `false` for a path with a root. Doesn't require
     *                                  normal form.
     */
    [[nodiscard]] bool isEscaping() const {
        return PathView(*this).isEscaping();
    }

    /**
     * @return                          The last segment, empty if there is none. `"a/b.txt"` gives `"b.txt"`.
     */
    [[nodiscard]] std::string_view name() const;

    /**
     * @return                          The extension of the file name with its leading dot, empty if there is none,
     *                                  by the same rules as `withExtension`.
     */
    [[nodiscard]] std::string_view extension() const;

    /**
     * @return                          The file name without its extension. `"a/b.tar.gz"` gives `"b.tar"`.
     */
    [[nodiscard]] std::string_view stem() const;

    /**
     * @return                          Lexical parent, which keeps the root. The parent of `"a"` is `""`, and the
     *                                  parent of `"/a"` and of `"/"` is `"/"`. The lexical parent of `"../.."` is
     *                                  `".."`, which is not its semantic parent.
     */
    [[nodiscard]] Path parent() const;

    /**
     * @return                          The segments after the root. Empty segments show up for doubled separators, so
     *                                  this is meant for paths in normal form.
     */
    [[nodiscard]] PathSplit split() const {
        return PathView(*this).split();
    }

    /**
     * @param tail                      Path to append.
     * @return                          The two paths joined with a separator. An absolute `tail`, or one naming
     *                                  another root, replaces this path. A rooted `tail` keeps only this path's root
     *                                  name. A bare drive letter takes a separator too, so `"C:" / "x"` is `"C:/x"`
     *                                  and not the drive-relative `"C:x"`, which has to be spelled out if wanted.
     */
    [[nodiscard]] Path operator/(const Path &tail) const;

    Path &operator/=(PathView tail);

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

[[nodiscard]] inline Path operator/(PathView head, PathView tail) {
    return Path(head) / Path(tail);
}

template<>
struct fmt::formatter<Path> : fmt::formatter<std::string> {
    auto format(const Path &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.str(), ctx);
    }
};
