#pragma once

#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

#include "Utility/String/Format.h"
#include "Utility/Attributes.h"

#include "PathSplit.h"

class Path;

/**
 * Non-owning view over a path string, and the home of everything lexical that a path can do. The string is WTF-8 on
 * Windows and a byte string on POSIX, and nothing here validates it. On Windows both slashes separate components, and
 * a path keeps whichever it was given until `normalized` spells them all as `/`. On POSIX a backslash is an ordinary
 * character in a file name.
 *
 * The views and segments it hands out point into the viewed string, so they are valid only as long as that string is.
 */
class PathView {
 public:
    PathView() = default;
    inline PathView(const Path &path MM_LIFETIMEBOUND); // NOLINT: intentionally implicit.

    /**
     * Implicit constructors from a string that the view then points into.
     *
     * @param path                      Path string.
     */
    PathView(std::string_view path) : _path(path) {} // NOLINT: intentionally implicit.
    PathView(const char *path) : _path(path) {} // NOLINT: intentionally implicit.
    PathView(const std::string &path MM_LIFETIMEBOUND) : _path(path) {} // NOLINT: intentionally implicit.
    PathView(std::nullptr_t) = delete; // NOLINT: implicit, or copy-initialization would fall back to const char *.

    [[nodiscard]] std::string_view str() const {
        return _path;
    }

    [[nodiscard]] bool isEmpty() const {
        return _path.empty();
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
    [[nodiscard]] std::string native() const {
        return std::string(_path); // A copy, since the OS wants a null-terminated string.
    }
#endif

    /**
     * @return                          This path as a valid UTF-8 string for displaying to the user, with everything
     *                                  that's not valid UTF-8 replaced with U+FFFD.
     */
    [[nodiscard]] std::string displayString() const;

    /**
     * @param extension                 New extension, with or without the leading dot. Pass an empty string to drop
     *                                  the extension.
     * @return                          This path with the extension replaced. Only the last extension is replaced,
     *                                  so `"a.tar.gz"` with `".zip"` becomes `"a.tar.zip"`. A dotfile such as
     *                                  `".bashrc"`, or a name like `"..."` whose stem would be all dots, has no
     *                                  extension. A path without a file name gets the extension as one, so `""`
     *                                  gives `".zip"` and `"a/"` gives `"a/.zip"`, and `"."` gives `"..zip"`. The
     *                                  extension goes in as given, separators included.
     */
    [[nodiscard]] Path withExtension(std::string_view extension) const;

    /**
     * @return                          The root name and root directory this path starts with, empty if there are
     *                                  none, as in `"/"`, and on Windows `"C:"`, `"C:/"` and `"//server/share/"`.
     */
    [[nodiscard]] std::string_view root() const;

    /**
     * @return                          This path in lexical normal form: separators spelled `/`, single and with no
     *                                  trailing one past the root, no `.` segments, and `..` collapsed where it can
     *                                  be. A leading `..` stays in a path without a root directory and is dropped
     *                                  above one, so `"/.."` is `"/"`. A path that collapses to nothing becomes `"."`,
     *                                  and only the empty path stays empty.
     */
    [[nodiscard]] Path normalized() const;

    /**
     * @return                          Whether `normalized` would return this path unchanged. Allocates nothing.
     */
    [[nodiscard]] bool isNormalized() const;

    /**
     * @return                          Whether this path points above its starting point. `".."` and `"a/../.."` do,
     *                                  `"a/../b"` doesn't. A path with a root directory never does, so `"/.."`
     *                                  doesn't, while the drive-relative `"C:.."` does. Doesn't require normal form.
     */
    [[nodiscard]] bool isEscaping() const;

    /**
     * @return                          What follows the last separator and the root. `"a/b.txt"` gives `"b.txt"`,
     *                                  while `"a/"` and `"/"` give an empty name.
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
     * @return                          Lexical parent, which keeps the root. The parent of `"a"` and of `".."` is
     *                                  `"."`, of `"a/"` is `"a"`, and of `"/a"` and `"/"` is `"/"`. Only the empty
     *                                  path has an empty parent.
     */
    [[nodiscard]] PathView parent() const;

    /**
     * @return                          The segments after the root. Runs of separators count as one, so there are
     *                                  no empty segments.
     */
    [[nodiscard]] PathSplit split() const {
        return PathSplit(_path.substr(root().size()));
    }

    /**
     * @param segment                   Segment handed out by `split` of this path.
     * @return                          The rest of this path, starting at `segment`. On Windows a segment can start
     *                                  with a drive, so in `"a/c:b"` the tail at `"c:b"` has a root of its own.
     */
    [[nodiscard]] PathView tailAt(std::same_as<std::string_view> auto segment) const {
        assert(segment.data() >= _path.data() && segment.data() + segment.size() <= _path.data() + _path.size());
        return PathView(_path.substr(segment.data() - _path.data()));
    }

    /**
     * @param segment                   Segment handed out by `split` of this path, or an empty view.
     * @return                          The rest of this path after `segment` and the separators that follow it, or
     *                                  all of it, root included, for an empty `segment`. Like `tailAt`, the rest can
     *                                  start with a drive on Windows.
     */
    [[nodiscard]] PathView tailAfter(std::same_as<std::string_view> auto segment) const {
        if (segment.empty())
            return *this;

        assert(segment.data() >= _path.data() && segment.data() + segment.size() <= _path.data() + _path.size());
        size_t offset = detail::skipPathSeparators(_path, segment.data() + segment.size() - _path.data());
        if (offset == _path.size())
            return {};
        return PathView(_path.substr(offset));
    }

    friend auto operator<=>(PathView l, PathView r) = default;

 private:
    std::string_view _path;
};

/**
 * Hashes a path as its string, so a `Path` and a view of it hash the same. Transparent, so a container keyed on `Path`
 * with `std::equal_to<>` can look up a view or a string without building a `Path`.
 */
template<>
struct std::hash<PathView> {
    using is_transparent = void;

    [[nodiscard]] size_t operator()(PathView path) const {
        return std::hash<std::string_view>()(path.str());
    }
};

template<>
struct fmt::formatter<PathView> : fmt::formatter<std::string_view> {
    auto format(PathView path, format_context &ctx) const {
        return fmt::formatter<std::string_view>::format(path.str(), ctx);
    }
};

#include "Path.h" // NOLINT: PathView converts from Path, and the conversion lives there.
