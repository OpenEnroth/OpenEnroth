#include "NativePath.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

#include "Utility/String/Ascii.h"
#include "Utility/String/Encoding.h"

static constexpr char separator = '/'; // The only separator in the stored string, fromWtf8 converts backslashes.

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
 *                                  with nothing to open under it. POSIX has no root names at all.
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
            if (end + 1 >= path.size() || path[end + 1] == separator)
                break; // A missing component ends the root name early, as in a bare "//server".
            end = std::min(path.find(separator, end + 1), path.size());
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
 * @param path                      Path to append to.
 * @param root                      The path's root.
 * @return                          Whether a component appended to the path needs a separator in front of it. It
 *                                  doesn't after an empty path, a trailing separator, or a bare drive letter, since
 *                                  `"C:x"` names `"x"` in the current directory of drive C.
 */
static bool needsSeparator(std::string_view path, PathRoot root) {
    return !path.empty() && path.back() != separator && !(root.kind == PATH_ROOT_DRIVE && root.size == path.size());
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

NativePath::NativePath(std::string_view path) {
    *this = fromWtf8(path);
}

NativePath NativePath::fromWtf8(std::string_view path) {
    NativePath result;
    result._path = path;
#ifdef _WINDOWS
    std::ranges::replace(result._path, '\\', separator); // Both slashes separate components on Windows.
#endif
    return result;
}

#ifdef _WINDOWS
NativePath NativePath::fromNative(std::wstring_view path) {
    return fromWtf8(txt::wideToWtf8(path));
}
#else
NativePath NativePath::fromNative(std::string_view path) {
    return fromWtf8(path);
}
#endif

#ifdef _WINDOWS
std::wstring NativePath::native() const {
    std::wstring result = txt::wtf8ToWide(_path);

    // Win32 only recognizes a literal "\\?\". Spelled with forward slashes the prefix gets parsed like any other path,
    // which brings back MAX_PATH and strips a trailing dot off a file name. "//./" makes no difference either way.
    if (parseRoot(_path).kind == PATH_ROOT_EXTENDED)
        std::ranges::replace(result, L'/', L'\\');

    return result;
}
#endif

std::string NativePath::displayString() const {
    return txt::encodedToUtf8(_path, ENCODING_UTF8); // UTF-8 to UTF-8 conversion replaces all the invalid parts.
}

NativePath NativePath::absolute() const {
    // Resolution is delegated to std::filesystem because on Windows it's not lexical. A drive-relative "C:x"
    // resolves against the current directory of drive C, which only the OS knows.
    return fromStdPath(_path.empty() ? std::filesystem::current_path() : std::filesystem::absolute(toStdPath()));
}

NativePath NativePath::withExtension(std::string_view extension) const {
    PathRoot root = parseRoot(_path);
    size_t nameOffset = fileNameOffset(_path, root);

    NativePath result;
    result._path = _path.substr(0, extensionOffset(_path, nameOffset));
    if (extension.empty())
        return result;

    if (nameOffset == _path.size() && needsSeparator(_path, root))
        result._path += separator; // A root name like "//server/share" has no file name, so the extension starts one.
    if (extension[0] != '.')
        result._path += '.';
    result._path += extension;
    return result;
}

NativePath NativePath::operator/(const NativePath &tail) const {
    PathRoot root = parseRoot(_path);
    PathRoot tailRoot = parseRoot(tail._path);
    bool tailNamesAnotherRoot = tailRoot.size > 0 && tail._path.compare(0, tailRoot.size, _path, 0, root.size) != 0;

    if (tailRoot.isAbsolute() || tailNamesAnotherRoot)
        return tail;

    NativePath result;
    if (tailRoot.hasRootDirectory) {
        result._path = _path.substr(0, root.size); // A rooted tail keeps our root name, and drops everything after it.
    } else {
        result._path = _path;
        if (needsSeparator(_path, root))
            result._path += separator;
    }

    result._path.append(tail._path, tailRoot.size);
    return result;
}
