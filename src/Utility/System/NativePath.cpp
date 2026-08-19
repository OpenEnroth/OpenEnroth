#include "NativePath.h"

#include <string>

#include "Utility/String/Encoding.h"

NativePath::NativePath(AsciiLiteral path) {
    *this = fromWtf8(path);
}

NativePath NativePath::fromWtf8(std::string_view path) {
#ifdef _WINDOWS
    return fromStdPath(std::filesystem::path(txt::wtf8ToWide(path)));
#else
    return fromStdPath(std::filesystem::path(path));
#endif
}

std::string NativePath::toWtf8() const {
#ifdef _WINDOWS
    return txt::wideToWtf8(_path.generic_wstring());
#else
    return _path.generic_string();
#endif
}

std::string NativePath::displayString() const {
    return txt::encodedToUtf8(toWtf8(), ENCODING_UTF8); // UTF8 to UTF8 conversion replaces all the invalid parts.
}
