#include "NativePath.h"

#include <algorithm>
#include <string>
#include <utility>

#include "Utility/String/Encoding.h"

NativePath NativePath::fromWtf8(std::string_view path) {
#ifdef _WINDOWS
    std::string result(path);
    std::replace(result.begin(), result.end(), '\\', '/'); // '\\' is not a valid file name char on Windows, so it's a separator.
    return NativePath(std::move(result));
#else
    return NativePath(std::string(path));
#endif
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
    std::string result = _path;

    std::string_view name = std::string_view(result).substr(result.find_last_of('/') + 1); // npos + 1 == 0.
    if (name != "." && name != "..") {
        size_t dotPos = name.find_last_of('.');
        if (dotPos != std::string_view::npos && dotPos > 0)
            result.resize(result.size() - (name.size() - dotPos));
    }

    if (!extension.empty()) {
        if (!extension.starts_with('.'))
            result += '.';
        result += extension;
    }

    return NativePath(std::move(result));
}
