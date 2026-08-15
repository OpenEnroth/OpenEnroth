#include "NativePath.h"

#include <algorithm>
#include <string>
#include <utility>

#include "Utility/String/Encoding.h"

NativePath::NativePath(AsciiLiteral path) {
    *this = fromWtf8(path);
}

NativePath NativePath::fromWtf8(std::string_view path) {
    NativePath result;
    result._path = path;
#ifdef _WINDOWS
    std::replace(result._path.begin(), result._path.end(), '\\', '/'); // '\\' is not a valid file name char on Windows, so it's a separator.
#endif
    return result;
}

#ifdef _WINDOWS
NativePath NativePath::fromNative(std::wstring_view path) {
    return fromWtf8(txt::wideToWtf8(path)); // fromWtf8 normalizes the separators.
}

std::wstring NativePath::native() const {
    return txt::wtf8ToWide(_path);
}
#endif

std::string NativePath::displayString() const {
    return txt::encodedToUtf8(_path, ENCODING_UTF8); // UTF8 to UTF8 conversion replaces all the invalid parts.
}

NativePath NativePath::withExtension(std::string_view extension) const {
    NativePath result = *this;

    std::string_view name = std::string_view(result._path).substr(result._path.find_last_of('/') + 1); // npos + 1 == 0.
    if (name != "." && name != "..") {
        size_t dotPos = name.find_last_of('.');
        if (dotPos != std::string_view::npos && dotPos > 0)
            result._path.resize(result._path.size() - (name.size() - dotPos));
    }

    if (!extension.empty()) {
        if (!extension.starts_with('.'))
            result._path += '.';
        result._path += extension;
    }

    return result;
}
