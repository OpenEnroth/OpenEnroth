#pragma once

#include <filesystem>
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
 * done by our own code, and the underlying `std::filesystem::path` is only ever constructed from `wchar_t` strings
 * on Windows. `fromWtf8` / `toWtf8` convert from and to our own strings, and `fromStdPath` / `toStdPath` are for
 * talking to the OS, with no charset conversion whatsoever.
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

    [[nodiscard]] static NativePath fromStdPath(std::filesystem::path path) {
        NativePath result;
        result._path = std::move(path);
        return result;
    }

    // Deliberately dead for strings of all charsets, see the class docs. Use fromWtf8.
    template<class T> static NativePath fromStdPath(const T &) = delete;

    /**
     * @return                          This path as a string, always using forward slashes. WTF-8 on Windows,
     *                                  byte string on POSIX. Never throws, unlike
     *                                  `std::filesystem::path::generic_string()`.
     */
    [[nodiscard]] std::string toWtf8() const;

    [[nodiscard]] const std::filesystem::path &toStdPath() const {
        return _path;
    }

    /**
     * @return                          This path as a valid UTF-8 string for displaying to the user, with everything
     *                                  that's not valid UTF-8 replaced with U+FFFD. Unlike the string returned by
     *                                  `toWtf8`, it might not round-trip back into the same path.
     */
    [[nodiscard]] std::string displayString() const;

    /**
     * @param extension                 New extension, with or without the leading dot. Pass an empty string to drop
     *                                  the extension. WTF-8 on Windows, byte string on POSIX.
     * @return                          Copy of this path with the extension replaced.
     */
    [[nodiscard]] NativePath withExtension(std::string_view extension) const {
        std::filesystem::path result = _path;
        result.replace_extension(fromWtf8(extension).toStdPath());
        return fromStdPath(std::move(result));
    }

    /**
     * @return                          Parent path, or an empty path if this path has no parent. Same semantics as
     *                                  `std::filesystem::path::parent_path`.
     */
    [[nodiscard]] NativePath parent() const {
        return fromStdPath(_path.parent_path());
    }

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    [[nodiscard]] NativePath operator/(const NativePath &tail) const {
        return fromStdPath(_path / tail._path);
    }

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
    std::filesystem::path _path;
};

template<>
struct fmt::formatter<NativePath> : fmt::formatter<std::string> {
    auto format(const NativePath &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.toWtf8(), ctx);
    }
};
