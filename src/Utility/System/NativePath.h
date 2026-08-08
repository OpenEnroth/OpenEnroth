#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Format.h"

/**
 * A native OS path. Unlike `std::filesystem::path`, it does not depend on the C locale - on Windows constructing an
 * `std::filesystem::path` from a narrow string converts it per the C locale, while here all charset conversions are
 * done by our own code, and the underlying `std::filesystem::path` is only ever constructed from `wchar_t` strings
 * on Windows. So:
 * - `fromWtf8` / `toWtf8` convert from and to our own strings, which are WTF-8.
 * - `fromStdPath` / `toStdPath` are for talking to the OS - `fromStdPath` takes paths the OS handed us, e.g. from
 *   `std::filesystem::current_path` or directory iteration, and `toStdPath` is for passing this path into
 *   `std::filesystem` functions. Neither does any charset conversion.
 *
 * This is the repo's vocabulary type for native paths - everything that takes a native path takes a `NativePath`.
 */
class NativePath {
 public:
    NativePath() = default;

    [[nodiscard]] static NativePath fromWtf8(std::string_view path);

    [[nodiscard]] static NativePath fromStdPath(std::filesystem::path path) {
        return NativePath(std::move(path));
    }

    // Deliberately dead for strings of all charsets, see the class docs. Use fromWtf8.
    template<class T> static NativePath fromStdPath(const T &) = delete;

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
    }

    /**
     * @return                          Absolute copy of this path, resolved against the current directory. An empty
     *                                  path resolves to the current directory itself.
     */
    [[nodiscard]] NativePath absolute() const {
        return NativePath(_path.empty() ? std::filesystem::current_path() : std::filesystem::absolute(_path));
    }

    /**
     * @return                          This path as a WTF-8 string, always using forward slashes. Never throws,
     *                                  unlike `std::filesystem::path::generic_string()`.
     */
    [[nodiscard]] std::string toWtf8() const;

    [[nodiscard]] const std::filesystem::path &toStdPath() const {
        return _path;
    }

    [[nodiscard]] NativePath operator/(const NativePath &tail) const {
        return NativePath(_path / tail._path);
    }

    /**
     * @param extension                 New extension, WTF-8, with or without the leading dot. Pass an empty string
     *                                  to drop the extension.
     * @return                          Copy of this path with the extension replaced.
     */
    [[nodiscard]] NativePath withExtension(std::string_view extension) const {
        std::filesystem::path result = _path;
        result.replace_extension(fromWtf8(extension).toStdPath());
        return NativePath(std::move(result));
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
    explicit NativePath(std::filesystem::path path) : _path(std::move(path)) {}

 private:
    std::filesystem::path _path;
};

template<>
struct fmt::formatter<NativePath> : fmt::formatter<std::string> {
    auto format(const NativePath &path, format_context &ctx) const {
        return fmt::formatter<std::string>::format(path.toWtf8(), ctx);
    }
};
