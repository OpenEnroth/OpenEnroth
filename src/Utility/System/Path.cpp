#include "Path.h"

#include <string>
#include <string_view>

#include "Utility/String/Encoding.h"

#ifdef _WINDOWS
Path Path::fromNative(std::wstring_view path) {
    return Path(txt::wideToWtf8(path));
}
#else
Path Path::fromNative(std::string_view path) {
    return Path(path);
}
#endif
