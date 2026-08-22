#pragma once

#include <compare>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/AsciiLiteral.h"
#include "Utility/String/Format.h"

/**
 * The repo's vocabulary type for native paths - everything that takes a native path takes a `NativePath`.
 *
 * The path is stored as a string - WTF-8 on Windows, a byte string on POSIX - and all path manipulation is lexical,
 * these methods never touch the file system. Separators are normalized to forward slashes on Windows, where both
 * slashes separate path components. On POSIX a backslash is an ordinary character in a file name, so it's left alone.
 *
 * Unlike `std::filesystem::path`, this class does not depend on the C locale - on Windows constructing an
 * `std::filesystem::path` from a narrow string converts it per the C locale, while here all charset conversions are
 * done by our own code, and the OS is only ever handed `wchar_t` strings. `native` / `fromNative` are the conversions
 * to use when talking to the OS, and `toStdPath` / `fromStdPath` are for the code that still needs `std::filesystem`.
 *
 * Note that `fromWtf8` / `toWtf8` are named somewhat improperly - file names on Linux are arbitrary byte strings,
 * and these bytes are passed through as-is. So the string returned by `toWtf8` is not necessarily valid UTF-8, and
 * not even necessarily valid WTF-8 - it's guaranteed to be valid WTF-8 on Windows only. And MacOS is different
 * again - APFS only takes file names that are valid UTF-8.
 */
class NativePath {
 public:
    NativePath() = default;

    /**
     * Implicit constructor from an ASCII string literal. ASCII-only - it's the only subset that means the same bytes
     * in the compiler's execution charset, in WTF-8, and in POSIX file names. Use `fromWtf8` for everything else.
     *
     * @param path                      Path as an ASCII string literal.
     */
    NativePath(AsciiLiteral path); // NOLINT: intentionally implicit.

    // TODO(captainurist): fromWtf8 / toWtf8 are misnomers, the strings are WTF-8 on Windows only. Rename.

    /**
     * @param path                      Path string. WTF-8 on Windows, byte string on POSIX.
     * @return                          `NativePath` for the given string.
     */
    [[nodiscard]] static NativePath fromWtf8(std::string_view path);

    /**
     * @param path                      Path as the OS spells it - a `wchar_t` string on Windows.
     * @return                          `NativePath` for the given string.
     */
#ifdef _WINDOWS
    [[nodiscard]] static NativePath fromNative(std::wstring_view path);
#else
    [[nodiscard]] static NativePath fromNative(std::string_view path);
#endif

    // TODO(captainurist): toStdPath / fromStdPath are for the call sites that still reach for std::filesystem
    //                     directly. Drop them once those are all on the os functions.
    [[nodiscard]] static NativePath fromStdPath(const std::filesystem::path &path) {
        return fromNative(path.native());
    }

    // Deliberately dead for strings of all charsets, see the class docs. Use fromWtf8.
    template<class T> static NativePath fromStdPath(const T &) = delete;

    /**
     * @return                          This path as a string, always using forward slashes. WTF-8 on Windows,
     *                                  byte string on POSIX.
     */
    [[nodiscard]] const std::string &toWtf8() const {
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

    [[nodiscard]] std::filesystem::path toStdPath() const {
        return std::filesystem::path(native());
    }

    /**
     * @return                          This path as a valid UTF-8 string for displaying to the user, with everything
     *                                  that's not valid UTF-8 replaced with U+FFFD. Unlike the string returned by
     *                                  `toWtf8`, it might not round-trip back into the same path.
     */
    [[nodiscard]] std::string displayString() const;

    /**
     * @return                          Absolute copy of this path, resolved against the current directory. An empty
     *                                  path resolves to the current directory itself.
     */
    [[nodiscard]] NativePath absolute() const;

    /**
     * @param extension                 New extension, with or without the leading dot. Pass an empty string to drop
     *                                  the extension. WTF-8 on Windows, byte string on POSIX.
     * @return                          Copy of this path with the extension replaced. Only the last extension is
     *                                  replaced, so `"a.tar.gz"` with `".zip"` becomes `"a.tar.zip"`.
     */
    [[nodiscard]] NativePath withExtension(std::string_view extension) const;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    /**
     * @param tail                      Path to append.
     * @return                          The two paths joined with a separator. A rooted `tail` replaces this path
     *                                  instead of being appended to it, same as `std::filesystem::path::operator/`.
     */
    [[nodiscard]] NativePath operator/(const NativePath &tail) const;

    friend auto operator<=>(const NativePath &l, const NativePath &r) = default;

    /**
     * CLI11 picks this function up through ADL, so that options can bind `NativePath` fields directly. Note that
     * `argv` is WTF-8 on Windows, where `UnicodeCrt` converts it from the wide command line, and a byte string on
     * POSIX.
     */
    friend bool lexical_cast(const std::string &input, NativePath &output) {
        output = NativePath::fromWtf8(input);
        return true;
    }

 private:
    std::string _path;
};

template<>
struct fmt::formatter<NativePath> : fmt::formatter<std::string> {
    auto format(const NativePath &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.toWtf8(), ctx);
    }
};
