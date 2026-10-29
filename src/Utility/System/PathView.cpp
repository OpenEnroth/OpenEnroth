#include "PathView.h"

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

#include "Path.h"

static constexpr char separator = '/'; // How normalized() and the joins spell a separator.

using detail::findLastPathSeparator;
using detail::findPathSeparator;
using detail::isPathSeparator;

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
    size_t size = 0; // Of the root name alone, rootEnd() adds the root directory.

    [[nodiscard]] bool isAbsolute() const {
#ifdef _WINDOWS
        return kind == PATH_ROOT_DRIVE ? hasRootDirectory : kind != PATH_ROOT_NONE; // "C:x" and "/x" are relative.
#else
        return hasRootDirectory;
#endif
    }
};

/**
 * @param path                          Path to parse.
 * @return                              Root name kind and size, and whether a root directory follows. Win32 roots a UNC
 *                                      path at the share, so `"//server/share"` is one root name. POSIX has no root
 *                                      names.
 */
static PathRoot parseRoot(std::string_view path) {
    PathRoot root;
#ifdef _WINDOWS
    if (path.size() >= 2 && (ascii::isLower(path[0]) || ascii::isUpper(path[0])) && path[1] == ':') {
        root.kind = PATH_ROOT_DRIVE;
        root.size = 2;
    } else if (path.size() >= 3 && isPathSeparator(path[0]) && isPathSeparator(path[1]) && !isPathSeparator(path[2])) {
        bool isExtended = path.size() >= 4 && (path[2] == '?' || path[2] == '.') && isPathSeparator(path[3]);
        root.kind = isExtended ? PATH_ROOT_EXTENDED : PATH_ROOT_UNC;

        // The extended-length spelling of a share, "//?/UNC/server/share", is two components longer. Win32 takes it
        // after "//./" as well, and reads "UNC" in any case.
        bool isExtendedUnc = isExtended && path.size() >= 8 && ascii::noCaseStartsWith(path.substr(4), "UNC") &&
                             isPathSeparator(path[7]);
        size_t components = isExtendedUnc ? 4 : 2;

        size_t end = 1; // The second leading slash, which is the separator before the first component.
        for (size_t i = 0; i < components; i++) {
            size_t start = end + 1;
            while (!isExtended && start < path.size() && isPathSeparator(path[start]))
                start++; // Win32 collapses doubled separators in a plain UNC root, but not after "//?/".
            if (start >= path.size())
                break; // A missing component ends the root name early, as in a bare "//server".
            end = std::min(findPathSeparator(path, start), path.size());
        }
        root.size = end;
    }
#endif
    root.hasRootDirectory = path.size() > root.size && isPathSeparator(path[root.size]);
    return root;
}

static size_t fileNameOffset(std::string_view path, PathRoot root) {
    size_t separatorPos = findLastPathSeparator(path);
    return std::max(separatorPos == std::string_view::npos ? 0 : separatorPos + 1, root.size);
}

/**
 * @param path                          Path to scan.
 * @param nameOffset                    Offset of the file name inside the path.
 * @return                              Offset of the extension's dot inside the path, or `npos` if there is none.
 */
static size_t extensionOffset(std::string_view path, size_t nameOffset) {
    std::string_view fileName = path.substr(nameOffset);

    size_t dotPos = fileName.rfind('.');
    if (dotPos == std::string_view::npos || dotPos == 0)
        return std::string_view::npos; // A leading dot doesn't start an extension, as in ".bashrc".

    if (fileName.substr(0, dotPos).find_first_not_of('.') == std::string_view::npos)
        return std::string_view::npos; // The stem of "..." would be "..", a navigation token.

    return nameOffset + dotPos;
}

static size_t rootEnd(PathRoot root) {
    return root.size + (root.hasRootDirectory ? 1 : 0);
}

/**
 * @param path                          Path to take the root name from.
 * @param root                          Its parsed root.
 * @return                              The root name with every separator spelled `/`. Win32 collapses doubled
 *                                      separators in a plain UNC root, so `"//server//x"` gives `"//server/x"`, but
 *                                      not after `"//?/"`.
 */
static std::string normalRootName(std::string_view path, PathRoot root) {
    std::string_view name = path.substr(0, root.size);
    if (root.kind == PATH_ROOT_UNC)
        return "//" + join(PathSplit(name.substr(2)), separator);

    std::string result(name);
#ifdef _WINDOWS
    std::ranges::replace(result, '\\', separator);
#endif
    return result;
}

static std::string normalizePath(std::string_view path) {
    std::string result;
    std::string_view input = path;
    gch::small_vector<std::string_view, 32> segments;
    while (true) {
        PathRoot root = parseRoot(input);

        segments.clear();
        for (std::string_view segment : PathSplit(input.substr(rootEnd(root)))) {
            if (segment == ".")
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

#ifdef _WINDOWS
    if (path.contains('\\'))
        return false;
#endif

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
    for (std::string_view segment : PathSplit(path.substr(root.size))) {
        if (segment == ".")
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
    while (end > rootEnd(root) && isPathSeparator(path[end - 1]))
        end--;
    return end;
}

#ifdef _WINDOWS
std::wstring PathView::native() const {
    std::wstring result = txt::wtf8ToWide(_path);

    // Win32 only recognizes a literal "\\?\". Spelled with forward slashes the prefix gets parsed like any other path,
    // which brings back MAX_PATH and strips a trailing dot off a file name. A "//./" path is parsed the same with
    // either slash.
    bool isExtended = _path.size() >= 4 && isPathSeparator(_path[0]) && isPathSeparator(_path[1]) &&
                      (_path[2] == '?' || _path[2] == '.') && isPathSeparator(_path[3]);
    if (isExtended)
        std::ranges::replace(result, L'/', L'\\');

    return result;
}
#endif

std::string PathView::displayString() const {
    return txt::encodedToUtf8(_path, ENCODING_UTF8); // UTF-8 to UTF-8 conversion replaces all the invalid parts.
}

Path PathView::withExtension(std::string_view extension) const {
    PathRoot root = parseRoot(_path);
    size_t nameOffset = fileNameOffset(_path, root);

    std::string result(_path.substr(0, extensionOffset(_path, nameOffset)));
    if (extension.empty())
        return Path(std::move(result));

    if (nameOffset == _path.size() && !_path.empty() && !isPathSeparator(_path.back()))
        result += separator; // A bare root name like "C:" or "//server/share" has no file name, so this starts one.
    if (extension[0] != '.')
        result += '.';
    result += extension;
    return Path(std::move(result));
}

std::string_view PathView::root() const {
    return _path.substr(0, rootEnd(parseRoot(_path)));
}

Path PathView::normalized() const {
    return Path(normalizePath(_path));
}

bool PathView::isNormalized() const {
    return isNormalizedPath(_path);
}

bool PathView::isEscaping() const {
    return isEscapingPath(_path);
}

std::string_view PathView::name() const {
    return _path.substr(fileNameOffset(_path, parseRoot(_path)));
}

std::string_view PathView::extension() const {
    size_t offset = extensionOffset(_path, fileNameOffset(_path, parseRoot(_path)));
    return offset == std::string_view::npos ? std::string_view() : _path.substr(offset);
}

std::string_view PathView::stem() const {
    std::string_view name = this->name();
    return name.substr(0, name.size() - extension().size());
}

PathView PathView::parent() const {
    std::string_view result = _path.substr(0, parentEnd(_path));
    if (result.empty() && !_path.empty())
        return PathView(std::string_view(".")); // A literal outlives every path, so the view never dangles.
    return PathView(result);
}

size_t detail::joinedHeadSize(PathView head) {
    std::string_view path = head.str();
    size_t end = path.size();
    while (end > rootEnd(parseRoot(path)) && isPathSeparator(path[end - 1]))
        end--;
    if (end < path.size() && end > 0 && !isPathSeparator(path[end - 1]))
        end++; // Keep the first separator of the run, spelled as given.
    return end;
}

Path detail::joinPaths(PathView head, PathView tail) {
    std::string_view headString = head.str();
    std::string_view tailString = tail.str();
    PathRoot headRoot = parseRoot(headString);
    PathRoot tailRoot = parseRoot(tailString);
    std::string_view headRootName = headString.substr(0, headRoot.size);
    std::string_view tailRootName = tailString.substr(0, tailRoot.size);
    bool tailNamesAnotherRoot = !tailRootName.empty() && !ascii::noCaseEquals(tailRootName, headRootName); // "c:" is "C:".

    if (tailRoot.isAbsolute() || tailNamesAnotherRoot)
        return Path(tail);

    std::string result;
    result.reserve(headString.size() + 1 + tailString.size());
    if (tailRoot.hasRootDirectory) {
        assert(tailRootName.empty());
        result = headRootName; // Win32 resolves "/b" on the current drive, so it keeps the root name of the head.
    } else {
        result = headString.substr(0, joinedHeadSize(head));
        if (!result.empty() && !isPathSeparator(result.back()))
            result += separator; // After a bare drive letter too, "C:" / "x" is "C:/x" and not the drive-relative "C:x".
    }

    result += tailString.substr(tailRoot.size);
    return Path(std::move(result));
}
