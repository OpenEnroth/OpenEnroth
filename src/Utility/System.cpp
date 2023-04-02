#include "System.h"

#ifdef _WINDOWS
#   include <utf8.h>
#endif

#include <cstdlib>

std::string env(const std::string& key) {
#ifdef _WINDOWS
    // What we're doing here with these reinterpret_casts is UB, but we'd rather not do unnecessary copies.
    const wchar_t *u16result = _wgetenv(reinterpret_cast<const wchar_t *>(utf8::utf8to16(key).c_str()));
    if (u16result == nullptr)
        return {};
    return utf8::utf16to8(reinterpret_cast<const char16_t *>(u16result));
#else
    return std::getenv(key.c_str());
#endif
}
