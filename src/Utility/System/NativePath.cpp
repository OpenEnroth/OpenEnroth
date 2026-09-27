#include "NativePath.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

#include "Utility/String/Ascii.h"
#include "Utility/String/Encoding.h"

static constexpr char separator = '/'; // The only separator in the stored string, fromWtf8 converts backslashes.

static bool hasDriveLetter([[maybe_unused]] std::string_view path) {
#ifdef _WINDOWS
    return path.size() >= 2 && (ascii::isLower(path[0]) || ascii::isUpper(path[0])) && path[1] == ':';
#else
    return false;
#endif
}

/**
 * @param path                          Path to scan.
 * @return                              Length of the root name, which is `"C:"`, `"//server/share"`, `"//?/C:"` or
 *                                      `"//?/UNC/server/share"` on Windows, and always zero on POSIX. Win32 roots a
 *                                      UNC path at the share, and a bare `"//server"` is a root name with nothing to
 *                                      open under it.
 */
static size_t rootNameSize([[maybe_unused]] std::string_view path) {
#ifdef _WINDOWS
    if (hasDriveLetter(path))
        return 2;

    if (path.size() >= 3 && path[0] == separator && path[1] == separator && path[2] != separator) {
        // The extended-length spelling of a share, "//?/UNC/server/share", is two components longer.
        size_t components = path.starts_with("//?/UNC/") ? 4 : 2;

        size_t end = 1; // The second leading slash, which is the separator before the first component.
        for (size_t i = 0; i < components; i++) {
            if (end + 1 >= path.size() || path[end + 1] == separator)
                break; // A missing component ends the root name early, as in a bare "//server".
            end = std::min(path.find(separator, end + 1), path.size());
        }
        return end;
    }
#endif
    return 0;
}

static bool hasRootDirectory(std::string_view path, size_t rootSize) {
    return path.size() > rootSize && path[rootSize] == separator;
}

/**
 * @param path                          Path to scan.
 * @param rootSize                      Length of the path's root name.
 * @return                              Whether the path depends on neither a current directory nor a current drive.
 *                                      On POSIX that takes a leading separator. On Windows it takes a UNC root name,
 *                                      or a drive letter followed by a separator, since `"C:x"` is relative to the
 *                                      current directory of drive C and `"/x"` to the current drive.
 */
static bool isAbsolute(std::string_view path, size_t rootSize) {
#ifdef _WINDOWS
    return hasDriveLetter(path) ? hasRootDirectory(path, rootSize) : rootSize > 0;
#else
    return hasRootDirectory(path, rootSize);
#endif
}

static size_t fileNameOffset(std::string_view path, size_t rootSize) {
    size_t separatorPos = path.rfind(separator);
    return std::max(separatorPos == std::string_view::npos ? 0 : separatorPos + 1, rootSize);
}

/**
 * @param path                          Path to scan.
 * @param rootSize                      Length of the path's root name.
 * @return                              Whether a component appended to the path needs a separator in front of it.
 *                                      It doesn't after an empty path, a trailing separator, or a bare drive letter,
 *                                      since `"C:x"` names `"x"` in the current directory of drive C.
 */
static bool needsSeparator(std::string_view path, size_t rootSize) {
    return !path.empty() && path.back() != separator && !(hasDriveLetter(path) && rootSize == path.size());
}

/**
 * @param path                          Path to scan.
 * @param nameOffset                    Offset of the file name inside the path.
 * @return                              Offset of the extension inside the path, or `npos` if there is none. A leading
 *                                      dot doesn't start an extension, so `".bashrc"` has none. Neither does a name
 *                                      whose stem would be all dots, because the stem of `"..."` is `".."`, and
 *                                      dropping the extension of such a name would turn it into a navigation token.
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

    // Win32 does no parsing after an extended-length prefix, so a forward slash there is just a character a file
    // name can't contain. A device path gets its backslashes back too, which is how Win32 spells those.
    if (result.starts_with(L"//?/") || result.starts_with(L"//./"))
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
    size_t rootSize = rootNameSize(_path);
    size_t nameOffset = fileNameOffset(_path, rootSize);

    NativePath result;
    result._path = _path.substr(0, extensionOffset(_path, nameOffset));
    if (extension.empty())
        return result;

    if (nameOffset == _path.size() && needsSeparator(_path, rootSize))
        result._path += separator; // A root name like "//server/share" has no file name, so the extension starts one.
    if (extension[0] != '.')
        result._path += '.';
    result._path += extension;
    return result;
}

NativePath NativePath::operator/(const NativePath &tail) const {
    size_t rootSize = rootNameSize(_path);
    size_t tailRootSize = rootNameSize(tail._path);
    bool tailNamesAnotherRoot = tailRootSize > 0 && tail._path.compare(0, tailRootSize, _path, 0, rootSize) != 0;

    if (isAbsolute(tail._path, tailRootSize) || tailNamesAnotherRoot)
        return tail;

    NativePath result;
    if (hasRootDirectory(tail._path, tailRootSize)) {
        result._path = _path.substr(0, rootSize); // A rooted tail keeps our root name, and drops everything after it.
    } else {
        result._path = _path;
        if (needsSeparator(_path, rootSize))
            result._path += separator;
    }

    result._path.append(tail._path, tailRootSize);
    return result;
}
