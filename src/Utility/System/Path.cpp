#include "Path.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Ascii.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"
#include "Utility/String/Split.h"
#include "Utility/SmallVector.h"

static constexpr char separator = '/'; // The only separator in the stored string, the constructor converts backslashes.

enum class PathRootKind {
    PATH_ROOT_NONE, // No root name, as in "x" and "/x", and every path on POSIX.
    PATH_ROOT_DRIVE, // A drive letter, "C:".
    PATH_ROOT_UNC, // A network share, "//server/share".
    PATH_ROOT_EXTENDED, // An extended-length or device prefix, "//?/C:", "//?/UNC/server/share" or "//./COM1".
};
using enum PathRootKind;

struct PathRoot {
    PathRootKind kind = PATH_ROOT_NONE;
    bool hasRootDirectory = false; // Whether a separator follows the root name, as in "C:/" and "/".
    size_t size = 0;

    [[nodiscard]] bool isAbsolute() const {
#ifdef _WINDOWS
        return kind == PATH_ROOT_DRIVE ? hasRootDirectory : kind != PATH_ROOT_NONE; // "C:x" and "/x" are relative.
#else
        return hasRootDirectory;
#endif
    }
};

/**
 * @param path                      Path to parse.
 * @return                          What the path starts with. Win32 roots a UNC path at the share, so
 *                                  `"//server/share"` is a single root name, and a bare `"//server"` is a root name
 *                                  with nothing to open under it. Doubled separators collapse in a plain UNC root,
 *                                  so `"//server//x"` is share `"x"` on `"server"`, but not after `"//?/"`, where
 *                                  `"//?/UNC//share"` has an empty server. POSIX has no root names at all.
 */
static PathRoot parseRoot(std::string_view path) {
    PathRoot root;
#ifdef _WINDOWS
    if (path.size() >= 2 && (ascii::isLower(path[0]) || ascii::isUpper(path[0])) && path[1] == ':') {
        root.kind = PATH_ROOT_DRIVE;
        root.size = 2;
    } else if (path.size() >= 3 && path[0] == separator && path[1] == separator && path[2] != separator) {
        bool isExtended = path.starts_with("//?/") || path.starts_with("//./");
        root.kind = isExtended ? PATH_ROOT_EXTENDED : PATH_ROOT_UNC;

        // The extended-length spelling of a share, "//?/UNC/server/share", is two components longer. Win32 takes it
        // after "//./" as well, and reads "UNC" in any case.
        size_t components = isExtended && ascii::noCaseStartsWith(path.substr(4), "UNC/") ? 4 : 2;

        size_t end = 1; // The second leading slash, which is the separator before the first component.
        for (size_t i = 0; i < components; i++) {
            size_t start = end + 1;
            while (!isExtended && start < path.size() && path[start] == separator)
                start++; // Win32 collapses doubled separators in a plain UNC root, but not after "//?/".
            if (start >= path.size())
                break; // A missing component ends the root name early, as in a bare "//server".
            end = std::min(path.find(separator, start), path.size());
        }
        root.size = end;
    }
#endif
    root.hasRootDirectory = path.size() > root.size && path[root.size] == separator;
    return root;
}

static size_t fileNameOffset(std::string_view path, PathRoot root) {
    size_t separatorPos = path.rfind(separator);
    return std::max(separatorPos == std::string_view::npos ? 0 : separatorPos + 1, root.size);
}

/**
 * @param path                      Path to scan.
 * @param nameOffset                Offset of the file name inside the path.
 * @return                          Offset of the extension inside the path, or `npos` if there is none. A leading
 *                                  dot doesn't start an extension, so `".bashrc"` has none. Neither does a name
 *                                  whose stem would be all dots, because the stem of `"..."` is `".."`, and
 *                                  dropping the extension of such a name would turn it into a navigation token.
 */
static size_t extensionOffset(std::string_view path, size_t nameOffset) {
    std::string_view fileName = path.substr(nameOffset);

    size_t dotPos = fileName.rfind('.');
    if (dotPos == std::string_view::npos || dotPos == 0)
        return std::string_view::npos;

    if (fileName.substr(0, dotPos).find_first_not_of('.') == std::string_view::npos)
        return std::string_view::npos;

    return nameOffset + dotPos;
}

static size_t rootEnd(PathRoot root) {
    return root.size + (root.hasRootDirectory ? 1 : 0);
}

/**
 * @param path                      Path to take the root name from.
 * @param root                      Its parsed root.
 * @return                          The root name in normal form. Win32 collapses doubled separators in a plain UNC
 *                                  root, so `"//server//x"` gives `"//server/x"`. Every other root name is kept as
 *                                  it is, `"//?/"` included, since Win32 keeps the separators after it.
 */
static std::string normalRootName(std::string_view path, PathRoot root) {
    std::string_view name = path.substr(0, root.size);
    if (root.kind != PATH_ROOT_UNC)
        return std::string(name);
    return "//" + join(split(name.substr(2)).by(separator).skip(""), separator);
}

static std::string normalizePath(std::string_view path) {
    std::string result;
    std::string_view input = path;
    gch::small_vector<std::string_view, 32> segments;
    while (true) {
        PathRoot root = parseRoot(input);

        segments.clear();
        for (std::string_view segment : split(input.substr(rootEnd(root))).by(separator)) {
            if (segment.empty() || segment == ".")
                continue;

            if (segment == "..") {
                if (!segments.empty() && segments.back() != "..") {
                    segments.pop_back();
                    continue;
                }
                if (root.hasRootDirectory)
                    continue; // Nothing is above a root directory, so "/.." is "/".
            }

            segments.push_back(segment);
        }

        std::string normal = normalRootName(input, root);
        size_t normalRootEnd = normal.size() + (root.hasRootDirectory ? 1 : 0);
        if (root.hasRootDirectory)
            normal += separator;
        normal += join(segments, separator);

        // Dropping the leading segments can turn the first remaining one into a drive, and "./C:." becomes "C:.",
        // whose "." only goes away on another pass.
        bool rootChanged = rootEnd(parseRoot(normal)) != normalRootEnd;
        result = std::move(normal);
        if (!rootChanged)
            break;
        input = result;
    }

    if (result.empty() && !path.empty())
        return "."; // An empty path names nothing, while this one names the current directory.
    return result;
}

static bool isNormalizedPath(std::string_view path) {
    if (path == ".")
        return true;

    PathRoot root = parseRoot(path);
    if (root.kind == PATH_ROOT_UNC && path.substr(2, root.size - 2).contains("//"))
        return false;

    std::string_view tail = path.substr(rootEnd(root));
    if (tail.empty())
        return true;
    if (tail.back() == separator)
        return false;

    bool leading = true;
    for (std::string_view segment : split(tail).by(separator)) {
        if (segment.empty() || segment == ".")
            return false;

        if (segment == "..") {
            if (root.hasRootDirectory || !leading)
                return false;
        } else {
            leading = false;
        }
    }
    return true;
}

static bool isEscapingPath(std::string_view path) {
    PathRoot root = parseRoot(path);
    if (root.hasRootDirectory)
        return false; // Nothing is above a root directory.

    int depth = 0;
    for (std::string_view segment : split(path.substr(root.size)).by(separator)) {
        if (segment.empty() || segment == ".")
            continue;

        if (segment != "..") {
            depth++;
        } else if (--depth < 0) {
            return true;
        }
    }
    return false;
}

static size_t parentEnd(std::string_view path) {
    PathRoot root = parseRoot(path);
    size_t end = fileNameOffset(path, root);
    while (end > rootEnd(root) && path[end - 1] == separator)
        end--;
    return end;
}

Path::Path(std::string_view path) : Path(std::string(path)) {}

Path::Path(std::string &&path) : _path(std::move(path)) {
#ifdef _WINDOWS
    std::ranges::replace(_path, '\\', separator); // Both slashes separate components on Windows.
#endif
}

#ifdef _WINDOWS
Path Path::fromNative(std::wstring_view path) {
    return Path(txt::wideToWtf8(path));
}
#else
Path Path::fromNative(std::string_view path) {
    return Path(path);
}
#endif

#ifdef _WINDOWS
std::wstring Path::native() const {
    std::wstring result = txt::wtf8ToWide(_path);

    // Win32 only recognizes a literal "\\?\". Spelled with forward slashes the prefix gets parsed like any other path,
    // which brings back MAX_PATH and strips a trailing dot off a file name. A "//./" path gets parsed whichever slashes
    // it uses, so converting it too does no harm.
    if (parseRoot(_path).kind == PATH_ROOT_EXTENDED)
        std::ranges::replace(result, L'/', L'\\');

    return result;
}
#endif

std::string Path::displayString() const {
    return txt::encodedToUtf8(_path, ENCODING_UTF8); // UTF-8 to UTF-8 conversion replaces all the invalid parts.
}

Path Path::normalized() const {
    Path result;
    result._path = normalizePath(_path);
    return result;
}

std::string_view Path::name() const {
    return std::string_view(_path).substr(fileNameOffset(_path, parseRoot(_path)));
}

std::string_view Path::extension() const {
    size_t offset = extensionOffset(_path, fileNameOffset(_path, parseRoot(_path)));
    return offset == std::string_view::npos ? std::string_view() : std::string_view(_path).substr(offset);
}

std::string_view Path::stem() const {
    std::string_view name = this->name();
    return name.substr(0, name.size() - extension().size());
}

Path Path::withExtension(std::string_view extension) const {
    PathRoot root = parseRoot(_path);
    size_t nameOffset = fileNameOffset(_path, root);

    Path result;
    result._path = _path.substr(0, extensionOffset(_path, nameOffset));
    if (extension.empty())
        return result;

    if (nameOffset == _path.size() && !_path.empty() && _path.back() != separator)
        result._path += separator; // A bare root name like "C:" or "//server/share" has no file name, so this starts one.
    if (extension[0] != '.')
        result._path += '.';
    result._path += extension;
    return result;
}

Path Path::joined(std::string_view tail) const {
    PathRoot selfRoot = parseRoot(_path);
    PathRoot tailRoot = parseRoot(tail);
    std::string_view selfRootName = std::string_view(_path).substr(0, selfRoot.size);
    std::string_view tailRootName = tail.substr(0, tailRoot.size);
    bool tailNamesAnotherRoot = !tailRootName.empty() && !ascii::noCaseEquals(tailRootName, selfRootName); // "c:" is "C:".

    Path result;
    if (tailRoot.isAbsolute() || tailNamesAnotherRoot) {
        result._path = tail;
        return result;
    }

    result._path.reserve(_path.size() + 1 + tail.size());
    if (tailRoot.hasRootDirectory) {
        assert(tailRootName.empty()); // A root name with a root directory is absolute.
        result._path = selfRootName; // Win32 resolves "/b" on the current drive, so it keeps this path's root name.
    } else {
        result._path = _path;
        if (!_path.empty() && _path.back() != separator)
            result._path += separator; // After a bare drive letter too, "C:" / "x" is "C:/x" and not the drive-relative "C:x".
    }

    result._path += tail.substr(tailRoot.size);
    return result;
}

std::string_view PathView::root() const {
    return _path.substr(0, rootEnd(parseRoot(_path)));
}

bool PathView::isEscaping() const {
    return isEscapingPath(_path);
}

bool PathView::isNormalized() const {
    return isNormalizedPath(_path);
}

PathView PathView::parent() const {
    std::string_view result = _path.substr(0, parentEnd(_path));
    if (result.empty() && !_path.empty())
        return PathView(std::string_view(".")); // A literal outlives every path, so the view never dangles.
    return PathView(result);
}
