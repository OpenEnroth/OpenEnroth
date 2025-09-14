#include "Unicode.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <limits>

std::string win::toUtf8(std::wstring_view wstr) {
    if (wstr.size() >= static_cast<size_t>(std::numeric_limits<int>::max()))
        return {}; // String is too long to pass to WideCharToMultiByte.

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    if (len == 0)
        return {}; // WideCharToMultiByte failed.

    std::string result(len, '\0');
    len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), result.data(), len, nullptr, nullptr);
    if (len == 0)
        return {}; // WideCharToMultiByte failed.

    return result;
}

std::wstring win::toUtf16(std::string_view str) {
    if (str.size() >= static_cast<size_t>(std::numeric_limits<int>::max()))
        return {}; // String is too long to pass to MultiByteToWideChar.

    int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
    if (len == 0)
        return {}; // MultiByteToWideChar failed.

    std::wstring result(len, '\0');
    len = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), result.data(), len);
    if (len == 0)
        return {}; // MultiByteToWideChar failed.

    return result;
}
