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
 * Non-owning view over a `Path`, or over a slice of one, and the home of everything lexical that a path can do.
 * Whatever it hands out that isn't a new `Path` points into the viewed string, so it is valid only as long as that
 * string is.
 */
class PathView {
 public:
    PathView() = default;
    inline PathView(const Path &path MM_LIFETIMEBOUND); // NOLINT: intentionally implicit.

    /**
     * Implicit constructors from a string that the view then points into. A literal and a `std::string` need
     * overloads of their own, since converting through `std::string_view` would take two user-defined conversions.
     *
     * @param path                      Path string. WTF-8 on Windows, byte string on POSIX.
     */
    PathView(std::string_view path) : _path(path) {} // NOLINT: intentionally implicit.
    PathView(const char *path) : _path(path) {} // NOLINT: intentionally implicit.
    PathView(const std::string &path MM_LIFETIMEBOUND) : _path(path) {} // NOLINT: intentionally implicit.

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
     *                                  extension. A path without a file name gets the extension as one.
     */
    [[nodiscard]] Path withExtension(std::string_view extension) const;

    /**
     * @return                          The root name and root directory this path starts with, empty if there are
     *                                  none. That's `"/"`, or `"C:"`, `"C:/"` and `"//server/share/"` on Windows.
     */
    [[nodiscard]] std::string_view root() const;

    /**
     * @return                          This path in lexical normal form: single separators, no trailing one, no `.`
     *                                  segments, and `..` collapsed where it can be. A leading `..` stays in a path
     *                                  without a root directory and is dropped above one, so `"/.."` is `"/"`. A
     *                                  path that collapses to nothing becomes `"."`, and only the empty path stays
     *                                  empty.
     */
    [[nodiscard]] Path normalized() const;

    /**
     * @return                          Whether `normalized` would return this path unchanged. Allocates nothing.
     */
    [[nodiscard]] bool isNormalized() const;

    /**
     * @return                          Whether this path points above its starting point. `".."` and `"a/../.."` do,
     *                                  `"a/../b"` doesn't. Always `false` for a path with a root directory, since
     *                                  nothing is above it, so `"/.."` doesn't escape, while the drive-relative
     *                                  `"C:.."` does. Doesn't require normal form.
     */
    [[nodiscard]] bool isEscaping() const;

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
     * @return                          Lexical parent, which keeps the root. The parent of `"a"` is `"."`, and the
     *                                  parent of `"/a"` and of `"/"` is `"/"`. Only the empty path has an empty
     *                                  parent. The lexical parent of `"../.."` is `".."`, which is not its semantic
     *                                  parent.
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
     * @param segment                   Segment of this path, as handed out by `split`. It has to point into this
     *                                  path.
     * @return                          The rest of this path, starting at `segment`. The root is not part of it.
     */
    [[nodiscard]] PathView tailAt(std::same_as<std::string_view> auto segment) const {
        assert(segment.data() >= _path.data() && segment.data() + segment.size() <= _path.data() + _path.size());
        return PathView(_path.substr(segment.data() - _path.data()));
    }

    /**
     * @param segment                   Segment of this path, as handed out by `split`, or an empty view. It has to
     *                                  point into this path.
     * @return                          The rest of this path after `segment` and the separators that follow it, or
     *                                  all of it, root included, for an empty `segment`.
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

    /**
     * @param tail                      Path to append.
     * @return                          The two paths joined with a separator. An absolute `tail`, or one naming
     *                                  another root, replaces this path. A rooted `tail` keeps only this path's root
     *                                  name. A bare drive letter takes a separator too, so `"C:" / "x"` is `"C:/x"`
     *                                  and not the drive-relative `"C:x"`, which has to be spelled out if wanted.
     */
    [[nodiscard]] Path operator/(PathView tail) const;

    friend auto operator<=>(PathView l, PathView r) = default;

 private:
    std::string_view _path;
};

template<>
struct std::hash<PathView> {
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
