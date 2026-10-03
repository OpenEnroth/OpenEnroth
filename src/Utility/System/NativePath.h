#pragma once

#include <compare>
#include <string>
#include <string_view>

#include "Utility/String/Format.h"

/**
 * The repo's vocabulary type for native paths - everything that takes a native path takes a `NativePath`.
 *
 * The path is stored as a string, WTF-8 on Windows and a byte string on POSIX, and path manipulation is lexical.
 * Everything that asks the OS lives in `Fs.h`. Separators are normalized to forward slashes on Windows, where
 * both slashes separate path components. On POSIX a backslash is an ordinary character in a file name, so it is left
 * alone.
 *
 * Unlike `std::filesystem::path`, this class does not depend on the C locale. On Windows constructing an
 * `std::filesystem::path` from a narrow string converts it per the C locale, while here all charset conversions are
 * done by our own code, and the OS is only ever handed `wchar_t` strings. `native` / `fromNative` are the conversions
 * to use when talking to the OS.
 *
 * An empty path names nothing. It is what a default-constructed `NativePath` holds, and joining it is a no-op, so
 * `"" / "a"` is `"a"`. The `fs::` calls treat it as invalid.
 *
 * File names on Linux are arbitrary byte strings, and these bytes are passed through as-is, so the string returned
 * by `str` is not necessarily valid UTF-8, and not even necessarily valid WTF-8. Nothing is validated on the way
 * in either, so on Windows it is the caller that keeps the string valid WTF-8, and `native` is where an invalid
 * sequence turns into a replacement character. And MacOS is different again, APFS only takes file names that are
 * valid UTF-8.
 */
class NativePath {
 public:
    NativePath() = default;

    /**
     * Implicit constructor from a string, same as `std::filesystem::path`. No charset conversion is performed. On
     * Windows backslashes become forward slashes.
     *
     * The `const char *` overload is what lets a string literal convert. Going through `std::string_view` alone
     * would need two user-defined conversions, and that's ill-formed.
     *
     * @param path                      Path string. WTF-8 on Windows, byte string on POSIX.
     */
    NativePath(std::string_view path); // NOLINT: intentionally implicit.
    NativePath(const char *path) : NativePath(std::string_view(path)) {} // NOLINT: intentionally implicit.

    /**
     * @param path                      Path as the OS spells it, a `wchar_t` string on Windows.
     * @return                          `NativePath` for the given string.
     */
#ifdef _WINDOWS
    [[nodiscard]] static NativePath fromNative(std::wstring_view path);
#else
    [[nodiscard]] static NativePath fromNative(std::string_view path);
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
    [[nodiscard]] NativePath withExtension(std::string_view extension) const;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    /**
     * @param tail                      Path to append.
     * @return                          The two paths joined with a separator. An absolute `tail`, or one naming
     *                                  another root, replaces this path. A rooted `tail` keeps only this path's root
     *                                  name. A bare drive letter takes a separator too, so `"C:" / "x"` is `"C:/x"`
     *                                  and not the drive-relative `"C:x"`, which has to be spelled out if wanted.
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
    std::string _path;
};

template<>
struct fmt::formatter<NativePath> : fmt::formatter<std::string> {
    auto format(const NativePath &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.str(), ctx);
    }
};
