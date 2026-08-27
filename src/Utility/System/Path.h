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
 * Every `Path` is in lexical normal form, established at construction and preserved by every operation:
 * separators are single forward slashes with no trailing one, `.` segments are gone (a lone `"."` is the empty path,
 * which means "here"), and `..` is collapsed - surviving only as a leading run of a relative path, and clamped above
 * an absolute root, so `"/.."` is `"/"`. On Windows a backslash is
 * a separator and is converted, on POSIX it's an ordinary character in a file name and is left alone. A leading
 * `"//"` is root syntax and survives normalization - a share name on Windows, where a bare `"//"` is just `"/"`, and
 * the implementation-defined two-slash prefix on POSIX, where `"///"` and longer collapse to `"/"`.
 *
 * Collapsing `..` lexically is a deliberate engine-wide decision - a path *means* its normal form. Where a symlink is
 * involved this diverges from the OS: with `a/link` pointing elsewhere, `Path("a/link/../f")` is `"a/f"`, while
 * the OS handed the raw string would resolve through the link's target. Code that needs the OS reading has to ask the
 * OS. The trailing-slash distinction POSIX draws on symlinks (`stat("link/")`) is likewise not expressible.
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
     * @return                          Whether this path points above its starting point. Under normal form that's
     *                                  exactly a path starting with a `..` segment. Always `false` for an absolute
     *                                  path, where `..` is clamped by the root.
     */
    [[nodiscard]] bool isEscaping() const;

    /**
     * @return                          The last component, empty if there is none. `"a/b.txt"` gives `"b.txt"`.
     *                                  The returned view points into this path, so it dies with it.
     */
    [[nodiscard]] std::string_view name() const;

    /**
     * @return                          The extension of the file name, with the leading dot, empty if there is none.
     *                                  A leading dot doesn't start one, so `".bashrc"` has no extension, and only the
     *                                  last one counts, so `"a.tar.gz"` gives `".gz"`. A name whose stem would be
     *                                  all dots has none either - `"..."` and `"...a"` are whole names, not a stem
     *                                  and an extension.
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
     * @param extension                 New extension, with or without the leading dot. Pass an empty string to drop
     *                                  the extension. Anything is accepted - this class never throws, and extension
     *                                  strings reach it from user data - so an argument that's shaped like a path
     *                                  is appended and normalized rather than rejected, and does extend the path.
     *                                  `"a/b"` with `"/c/d"` is `"a/b./c/d"`, same as `std::filesystem` gives.
     * @return                          Copy of this path with the extension replaced. Only the last extension is
     *                                  replaced, so `"a.tar.gz"` with `".zip"` becomes `"a.tar.zip"`. Dropping an
     *                                  extension never collapses the path, since the stem left behind is never all
     *                                  dots.
     */
    [[nodiscard]] Path withExtension(std::string_view extension) const;

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
    friend class PathView; // So that a view can answer root() and split() the same way this does.

    [[nodiscard]] static std::string_view rootOf(std::string_view path);

 private:
    // For results that are already normal - re-running normalization on them would be a no-op.
    [[nodiscard]] static Path fromNormalized(std::string path) {
        Path result;
        result._path = std::move(path);
        return result;
    }

 private:
    std::string _path;
};

template<>
struct fmt::formatter<Path> : fmt::formatter<std::string> {
    auto format(const Path &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.string(), ctx);
    }
};
