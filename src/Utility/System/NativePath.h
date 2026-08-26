#pragma once

#include <cassert>
#include <compare>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Format.h"

/**
 * The repo's vocabulary type for native paths - everything that takes a native path takes a `NativePath`.
 *
 * The path is stored as a string - WTF-8 on Windows, a byte string on POSIX - and all path manipulation is lexical,
 * these methods never touch the file system. No encoding is promised by the type itself, see below.
 *
 * Every `NativePath` is in lexical normal form, established at construction and preserved by every operation:
 * separators are single forward slashes with no trailing one, `.` segments are gone (a lone `"."` is the empty path,
 * which means "here"), and `..` is collapsed - surviving only as a leading run of a relative path, and clamped above
 * an absolute root, so `"/.."` is `"/"`. A leading `"//"` is root syntax and is preserved. On Windows a backslash is
 * a separator and is converted, on POSIX it's an ordinary character in a file name and is left alone.
 *
 * Collapsing `..` lexically is a deliberate engine-wide decision - a path *means* its normal form. Where a symlink is
 * involved this diverges from the OS: with `a/link` pointing elsewhere, `NativePath("a/link/../f")` is `"a/f"`, while
 * the OS handed the raw string would resolve through the link's target. Code that needs the OS reading has to ask the
 * OS. The trailing-slash distinction POSIX draws on symlinks (`stat("link/")`) is likewise not expressible.
 *
 * The bytes carry no encoding promise. They're WTF-8 on Windows, where that's the only encoding that round-trips
 * unpaired surrogates through the OS, and arbitrary bytes on Linux, which is what file names are there. MacOS is
 * different again - APFS only takes file names that are valid UTF-8. Encoding is checked where paths cross into the
 * OS, not here.
 */
class NativePath {
 public:
    NativePath() = default;

    /**
     * Implicit constructor from a byte string, same as `std::filesystem::path`. The bytes are taken as-is - a
     * `NativePath` promises no encoding, see the class docs.
     *
     * @param path                      Path as a byte string.
     */
    NativePath(std::string_view path); // NOLINT: intentionally implicit.
    NativePath(const char *path) : NativePath(std::string_view(path)) {} // NOLINT: intentionally implicit.
    NativePath(const std::string &path) : NativePath(std::string_view(path)) {} // NOLINT: intentionally implicit.

    /**
     * @param path                      Path as the OS spells it - a `wchar_t` string on Windows.
     * @return                          `NativePath` for the given string.
     */
#ifdef _WINDOWS
    [[nodiscard]] static NativePath fromNative(std::wstring_view path);
#else
    [[nodiscard]] static NativePath fromNative(std::string_view path);
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
     */
    [[nodiscard]] std::string_view root() const;

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
     */
    [[nodiscard]] std::string_view name() const;

    /**
     * @return                          The extension of the file name, with the leading dot, empty if there is none.
     *                                  A leading dot doesn't start one, so `".bashrc"` has no extension, and only the
     *                                  last one counts, so `"a.tar.gz"` gives `".gz"`.
     */
    [[nodiscard]] std::string_view extension() const;

    /**
     * @return                          The file name without its extension. `"a/b.tar.gz"` gives `"b.tar"`.
     */
    [[nodiscard]] std::string_view stem() const;

    /**
     * @return                          Parent path, or an empty path if this path has no parent. Clamped at the root,
     *                                  so the parent of `"/a"` is `"/"`. Note that this is the lexical parent - the
     *                                  lexical parent of `"../.."` is `".."`, which is not its semantic parent.
     */
    [[nodiscard]] NativePath parent() const;

    /**
     * @param extension                 New extension, with or without the leading dot. Pass an empty string to drop
     *                                  the extension. Must not contain a separator, and must not turn the file name
     *                                  into `"."` or `".."`.
     * @return                          Copy of this path with the extension replaced. Only the last extension is
     *                                  replaced, so `"a.tar.gz"` with `".zip"` becomes `"a.tar.zip"`.
     */
    [[nodiscard]] NativePath withExtension(std::string_view extension) const;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    /**
     * @param tail                      Path to append.
     * @return                          The two paths joined with a separator, re-normalized at the seam - so a
     *                                  leading `..` run in `tail` eats trailing components of the head. An absolute
     *                                  `tail` replaces this path instead of being appended to it, same as
     *                                  `std::filesystem::path::operator/`.
     */
    [[nodiscard]] NativePath operator/(const NativePath &tail) const;

    friend auto operator<=>(const NativePath &l, const NativePath &r) = default;

    /**
     * CLI11 picks this function up through ADL, so that options can bind `NativePath` fields directly. Note that
     * `argv` is WTF-8 on Windows, where `UnicodeCrt` converts it from the wide command line, and a byte string on
     * POSIX.
     */
    friend bool lexical_cast(const std::string &input, NativePath &output) {
        output = NativePath(input);
        return true;
    }

 private:
    // For results that are already normal - re-running normalization on them would be a no-op.
    [[nodiscard]] static NativePath fromNormalized(std::string path) {
        NativePath result;
        result._path = std::move(path);
        return result;
    }

 private:
    std::string _path;
};

template<>
struct fmt::formatter<NativePath> : fmt::formatter<std::string> {
    auto format(const NativePath &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.string(), ctx);
    }
};
