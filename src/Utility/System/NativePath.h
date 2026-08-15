#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/AsciiLiteral.h"
#include "Utility/String/Format.h"

/**
 * The repo's vocabulary type for native paths - everything that takes a native path takes a `NativePath`.
 *
 * Unlike `std::filesystem::path`, this class does not depend on the C locale - on Windows constructing an
 * `std::filesystem::path` from a narrow string converts it per the C locale, while here all charset conversions are
 * done by our own code. Internally the path is stored as a WTF8 string with forward slashes for separators, and
 * `native()` converts to the native encoding when it's time to talk to the OS.
 *
 * Note that `fromWtf8` / `toWtf8` are named somewhat improperly - file names on Linux are arbitrary byte strings
 * (anything goes except '/' and NUL), and these bytes are passed through as-is. So the string returned by `toWtf8`
 * is not necessarily valid UTF-8, and not even necessarily valid WTF-8 - it's guaranteed to be valid WTF-8 on
 * Windows only. And MacOS is different again - APFS only takes file names that are valid UTF-8.
 */
class NativePath {
 public:
    NativePath() = default;

    /**
     * Implicit constructor from an ASCII string literal.
     *
     * ASCII-only because a literal's bytes have to mean the same thing in three places at once - in the compiler's
     * execution charset (MSVC re-encodes literals per the locale codepage unless `/utf-8` is passed), when
     * interpreted as WTF8 on Windows, and as raw bytes on POSIX. ASCII is the one subset that all of them agree on.
     * Everything else goes through `fromWtf8`, where the encoding claim is visible at the call site.
     */
    NativePath(AsciiLiteral path); // NOLINT: intentionally implicit.

    [[nodiscard]] static NativePath fromWtf8(std::string_view path);

    /**
     * @param path                      Path in the OS-native encoding - a `wchar_t` string on Windows. Separators can
     *                                  be either forward or backward slashes.
     * @return                          `NativePath` for the given native string.
     */
#ifdef _WINDOWS
    [[nodiscard]] static NativePath fromNative(std::wstring_view path);
#else
    [[nodiscard]] static NativePath fromNative(std::string_view path) {
        NativePath result;
        result._path = path;
        return result;
    }
#endif

    /**
     * @return                          This path as a WTF-8 string, always using forward slashes.
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

    /**
     * @return                          This path as a valid UTF-8 string for displaying to the user, with everything
     *                                  that's not valid UTF-8 replaced with U+FFFD. Unlike the string returned by
     *                                  `toWtf8`, it might not round-trip back into the same path.
     */
    [[nodiscard]] std::string displayString() const;

    /**
     * @param extension                 New extension, WTF-8, with or without the leading dot. Pass an empty string
     *                                  to drop the extension.
     * @return                          Copy of this path with the extension replaced.
     */
    [[nodiscard]] NativePath withExtension(std::string_view extension) const;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    [[nodiscard]] NativePath operator/(const NativePath &tail) const {
        if (_path.empty())
            return tail;
        if (tail._path.empty())
            return *this;

        NativePath result = *this;
        if (!result._path.ends_with('/'))
            result._path += '/';
        result._path += tail._path;
        return result;
    }

    friend auto operator<=>(const NativePath &l, const NativePath &r) = default;

    /**
     * CLI11 picks this function up through ADL, so that options can bind `NativePath` fields directly. Note that
     * `argv` is WTF-8, `UnicodeCrt` converts it from the wide command line on Windows.
     */
    friend bool lexical_cast(const std::string &input, NativePath &output) {
        output = NativePath::fromWtf8(input);
        return true;
    }

 private:
    std::string _path; // WTF8, with forward slashes for separators.
};

template<>
struct fmt::formatter<NativePath> : fmt::formatter<std::string> {
    auto format(const NativePath &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.toWtf8(), ctx);
    }
};
