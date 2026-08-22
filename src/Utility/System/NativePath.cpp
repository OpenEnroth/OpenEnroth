#include "NativePath.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

#include "Utility/String/Ascii.h"
#include "Utility/String/Encoding.h"

// Everything below operates on the stored string, where the only separator is a forward slash.
static constexpr char separator = '/';

// Length of the root name - "C:" or "//server" on Windows, always zero on POSIX. A root name is what a relative path
// is relative to, and POSIX has only one file system tree, so there's nothing to name there.
static size_t rootNameSize([[maybe_unused]] std::string_view path) {
#ifdef _WINDOWS
    if (path.size() >= 2 && (ascii::isLower(path[0]) || ascii::isUpper(path[0])) && path[1] == ':')
        return 2; // Drive letter.

    if (path.size() >= 3 && path[0] == separator && path[1] == separator && path[2] != separator)
        return std::min(path.find(separator, 2), path.size()); // UNC share, e.g. "//server" in "//server/share".
#endif
    return 0;
}

// Whether the path is rooted at its root name - "C:/x" is, "C:x" and "x" are not.
static bool hasRootDirectory(std::string_view path) {
    size_t rootSize = rootNameSize(path);
    return path.size() > rootSize && path[rootSize] == separator;
}

// An absolute path needs both parts of the root on Windows - "C:/x" is absolute, while "C:x" is relative to the
// current directory of drive C, and "/x" is relative to the current drive.
static bool isAbsolute(std::string_view path) {
#ifdef _WINDOWS
    return rootNameSize(path) > 0 && hasRootDirectory(path);
#else
    return hasRootDirectory(path);
#endif
}

// Offset of the file name inside the path, so `path.substr(fileNameOffset(path))` is the last component.
static size_t fileNameOffset(std::string_view path) {
    size_t separatorPos = path.rfind(separator);
    return std::max(separatorPos == std::string_view::npos ? 0 : separatorPos + 1, rootNameSize(path));
}

// Offset of the extension inside the path, or npos if there's none. A leading dot doesn't start an extension, so
// ".bashrc" has no extension, and neither do "." and "..".
static size_t extensionOffset(std::string_view path) {
    size_t nameOffset = fileNameOffset(path);
    std::string_view fileName = path.substr(nameOffset);
    if (fileName == "." || fileName == "..")
        return std::string_view::npos;

    size_t dotPos = fileName.rfind('.');
    if (dotPos == std::string_view::npos || dotPos == 0)
        return std::string_view::npos;

    return nameOffset + dotPos;
}

NativePath::NativePath(AsciiLiteral path) {
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

std::wstring NativePath::native() const {
    return txt::wtf8ToWide(_path); // Win32 takes forward slashes just fine, no need to convert them back.
}
#else
NativePath NativePath::fromNative(std::string_view path) {
    return fromWtf8(path);
}
#endif

std::string NativePath::displayString() const {
    return txt::encodedToUtf8(_path, ENCODING_UTF8); // UTF-8 to UTF-8 conversion replaces all the invalid parts.
}

NativePath NativePath::absolute() const {
    // Resolution is delegated to std::filesystem b/c on Windows it's not lexical - a drive-relative "C:x" resolves
    // against the current directory of drive C, which only the OS knows.
    return fromStdPath(_path.empty() ? std::filesystem::current_path() : std::filesystem::absolute(toStdPath()));
}

NativePath NativePath::withExtension(std::string_view extension) const {
    NativePath result;
    size_t offset = extensionOffset(_path);
    result._path = offset == std::string::npos ? _path : _path.substr(0, offset);

    if (!extension.empty()) {
        if (extension[0] != '.')
            result._path += '.';
        result._path += extension;
    }

    return result;
}

NativePath NativePath::operator/(const NativePath &tail) const {
    size_t rootSize = rootNameSize(_path);
    size_t tailRootSize = rootNameSize(tail._path);
    bool tailNamesAnotherRoot = tailRootSize > 0 && tail._path.substr(0, tailRootSize) != _path.substr(0, rootSize);

    // A tail that names another root replaces this path entirely, there's nothing sensible to append it to.
    if (isAbsolute(tail._path) || tailNamesAnotherRoot)
        return tail;

    NativePath result;
    if (hasRootDirectory(tail._path)) {
        result._path = _path.substr(0, rootSize); // Rooted tail keeps our root name, and drops everything after it.
    } else {
        result._path = _path;
        if (!result._path.empty() && result._path.back() != separator && result._path.size() != rootSize)
            result._path += separator; // No separator after a bare root name - "C:" / "x" is "C:x".
    }

    result._path += tail._path.substr(tailRootSize);
    return result;
}
