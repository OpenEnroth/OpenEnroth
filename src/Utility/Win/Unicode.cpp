#include "Unicode.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>

std::string win::toUtf8(std::wstring_view wstr) {
    std::string result;

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    if (len == 0)
        return result;

    result.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), result.data(), len, nullptr, nullptr);
    return result;
}

std::wstring win::toUtf16(std::string_view str) {
    std::wstring result;

    int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
    if (len == 0)
        return result;

    result.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), result.data(), len);
    return result;
}
