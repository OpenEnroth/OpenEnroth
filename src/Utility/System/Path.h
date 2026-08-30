#pragma once

#include <compare>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Format.h"

#include "PathSplit.h"

/**
 * The repo's vocabulary type for native paths - everything that takes a native path takes a `Path`.
 *
 * The path is stored as a string - WTF-8 on Windows, a byte string on POSIX - and all path manipulation is lexical,
 * these methods never touch the file system. No encoding is promised by the type itself, see below.
 *
 * The bytes are kept as given. `normalized` is the one operation that rewrites them, collapsing `.` and `..`,
 * squeezing separators and dropping a trailing one, and it has to be asked for - a `Path` is not normal just because
 * it exists. The file system layer normalizes at its public boundary, so everything behind that boundary can assume
 * normal form. Code talking to the OS mostly doesn't need to, since the OS resolves paths itself.
 *
 * Collapsing `..` is lexical when you do ask for it, which diverges from the OS where a symlink is involved: with
 * `a/link` pointing elsewhere, `Path("a/link/../f").normalized()` is `"a/f"`, while the OS resolves through the
 * link's target. That is why normalizing is not automatic - a path bound for the OS should keep what it was given.
 *
 * On Windows a backslash is a separator and is converted on construction, on POSIX it's an ordinary character in a
 * file name and is left alone. A leading `"//"` is root syntax - a share name on Windows, where a bare `"//"` is
 * just `"/"`, and the implementation-defined two-slash prefix on POSIX, where `"///"` and longer collapse to `"/"`.
 *
 * The bytes carry no encoding promise. They're WTF-8 on Windows, where that's the only encoding that round-trips
 * unpaired surrogates through the OS, and arbitrary bytes on Linux, which is what file names are there. MacOS is
 * different again - APFS only takes file names that are valid UTF-8. Encoding is a concern of the code that talks to
 * the OS, not of this type - note that `native` substitutes U+FFFD for anything it can't convert rather than failing.
 */
class Path {
 public:
    Path() = default;

    /**
     * Implicit constructor from a byte string, same as `std::filesystem::path`. The bytes are taken as-is - a
     * `Path` promises no encoding, see the class docs.
     *
     * @param path                      Path as a byte string.
     */
    Path(std::string_view path); // NOLINT: intentionally implicit.
    Path(const char *path) : Path(std::string_view(path)) {} // NOLINT: intentionally implicit.
    Path(const std::string &path) : Path(std::string_view(path)) {} // NOLINT: intentionally implicit.

    /**
     * @param path                      View over a path that's already in normal form.
     */
    inline explicit Path(PathView path);

    /**
     * @param path                      Path as the OS spells it - a `wchar_t` string on Windows.
     * @return                          `Path` for the given string.
     */
#ifdef _WINDOWS
    [[nodiscard]] static Path fromNative(std::wstring_view path);
#else
    [[nodiscard]] static Path fromNative(std::string_view path);
#endif

    /**
     * @return                          This path as a byte string, always using forward slashes. WTF-8 on Windows,
     *                                  byte string on POSIX.
     */
    [[nodiscard]] const std::string &string() const {
        return _path;
    }

    /**
     * @return                          This path as a string in the OS-native encoding - a `wchar_t` string on
     *                                  Windows. Separators stay forward slashes, Windows APIs accept those. Use this
     *                                  for talking to the OS.
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
     *                                  `string`, it might not round-trip back into the same path.
     */
    [[nodiscard]] std::string displayString() const;

    /**
     * @return                          The root this path is relative to, empty for a relative path. That's `"/"`,
     *                                  or `"//"` on POSIX, or `"C:/"` / `"//server/"` on Windows. Note that a bare
     *                                  `"C:"` is not root syntax - it parses as an ordinary relative segment.
     *                                  The returned view points into this path, so it dies with it.
     */
    /**
     * @return                          Copy of this path in lexical normal form - single separators with no trailing
     *                                  one, no `.` segments, and `..` collapsed, surviving only as a leading run of a
     *                                  relative path and clamped above an absolute root so that `"/.."` is `"/"`.
     *                                  A lone `"."` normalizes to the empty path, which means "here".
     */
    [[nodiscard]] Path normalized() const;

    /**
     * @return                          Whether this path is already in lexical normal form, i.e. whether
     *                                  `normalized` would return it unchanged. Cheaper than normalizing, since it
     *                                  allocates nothing.
     */
    [[nodiscard]] bool isNormalized() const {
        return isNormalizedImpl(_path);
    }

    [[nodiscard]] std::string_view root() const {
        return rootOf(_path);
    }

    [[nodiscard]] bool isAbsolute() const {
        return !root().empty();
    }

    [[nodiscard]] bool isRelative() const {
        return !isAbsolute();
    }

    /**
     * @return                          Whether this path points above its starting point - `"a/../.."` does, and so
     *                                  does `".."`, while `"a/../b"` doesn't. Always `false` for an absolute path,
     *                                  where `..` is clamped by the root. Doesn't require normal form.
     */
    [[nodiscard]] bool isEscaping() const {
        return isEscapingImpl(_path);
    }

    /**
     * @return                          The last component, empty if there is none. `"a/b.txt"` gives `"b.txt"`.
     *                                  The returned view points into this path, so it dies with it.
     */
    [[nodiscard]] std::string_view name() const;

    /**
     * @return                          The extension of the file name, with the leading dot, empty if there is none.
     *                                  A leading dot doesn't start one, so `".bashrc"` has no extension, and only the
     *                                  last one counts, so `"a.tar.gz"` gives `".gz"`. Same as `std::filesystem`,
     *                                  which means `"..."` decomposes into a stem of `".."` and an extension of
     *                                  `"."`.
     *                                  The returned view points into this path, so it dies with it.
     */
    [[nodiscard]] std::string_view extension() const;

    /**
     * @return                          The file name without its extension. `"a/b.tar.gz"` gives `"b.tar"`.
     *                                  The returned view points into this path, so it dies with it.
     */
    [[nodiscard]] std::string_view stem() const;

    /**
     * @return                          Parent path, or an empty path if this path has no parent. Clamped at the root,
     *                                  so the parent of `"/a"` is `"/"`. Note that this is the lexical parent - the
     *                                  lexical parent of `"../.."` is `".."`, which is not its semantic parent.
     */
    [[nodiscard]] Path parent() const;

    /**
     * @param extension                 New extension, which must satisfy `isExtension` - the result is unspecified
     *                                  otherwise. Pass an empty string to drop the extension.
     * @return                          Copy of this path with the extension replaced. Only the last extension is
     *                                  replaced, so `"a.tar.gz"` with `".zip"` becomes `"a.tar.zip"`.
     *
     *                                  A path with no file name gains one - `"a/b/"` with `".txt"` is `"a/b/.txt"`,
     *                                  a hidden file inside the directory rather than a rename. `std::filesystem`
     *                                  does the same, and the empty path, `"/"` and `"a/b//"` all behave that way.
     */
    [[nodiscard]] Path withExtension(std::string_view extension) const;

    /**
     * @param extension                 String to check.
     * @return                          Whether it can be passed to `withExtension` - empty, or a leading dot
     *                                  followed by something that is not a separator, a NUL or a trailing dot.
     *                                  `isExtension(p.extension())` holds for every path, so
     *                                  `p.withExtension(p.extension()) == p` is always the identity.
     */
    [[nodiscard]] static bool isExtension(std::string_view extension);

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    /**
     * @return                          Lazy view over this path's name segments. The root is not one of them, so
     *                                  `path == path.root() / join(path.split())`. Under normal form a `..` can only
     *                                  show up as the leading run of an escaping relative path, and `"."` never does.
     */
    [[nodiscard]] PathSplit split() const {
        return PathSplit(std::string_view(_path).substr(root().size()));
    }

    /**
     * @param tail                      Path to append.
     * @return                          The two paths joined with a separator, re-normalized at the seam - so a
     *                                  leading `..` run in `tail` eats trailing components of the head. A `tail` with
     *                                  any root replaces this path instead of being appended to it, so that an
     *                                  absolute intent survives the join. That last part is deliberately unlike
     *                                  `std::filesystem::path::operator/`, which keeps the head's root name when the
     *                                  tail has a root directory but no root name of its own.
     */
    [[nodiscard]] Path operator/(const Path &tail) const;

    Path &operator/=(const Path &tail) {
        return *this = *this / tail;
    }

    inline Path &operator/=(PathView tail);

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

    /**
     * @param path                      Path string that's already in normal form, e.g. a slice of another path.
     * @return                          `Path` for the given string, without re-running normalization on it.
     */
    [[nodiscard]] static Path fromNormalized(std::string path) {
        Path result;
        result._path = std::move(path);
        return result;
    }

    [[nodiscard]] static Path fromNormalized(PathView path);

 private:
    friend class PathView; // So that a view can answer root() and split() the same way this does.

    [[nodiscard]] static std::string_view rootOf(std::string_view path);
    [[nodiscard]] static bool isEscapingImpl(std::string_view path);
    [[nodiscard]] static bool isNormalizedImpl(std::string_view path);

 private:
    std::string _path;
};

/**
 * Formats as `displayString`, so `{}` is always valid UTF-8 and safe to print. A path that is going to be read back
 * as a path has to say `.string()` - the substitution the display form does is lossy.
 */
template<>
struct fmt::formatter<Path> : fmt::formatter<std::string> {
    auto format(const Path &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.displayString(), ctx);
    }
};
