#include "System.h"

#ifdef _WINDOWS
#   include <clocale>
#   include <cassert>

#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>

#   include "Exception.h"
#endif

#ifdef _WINDOWS
std::string toUtf8(std::wstring_view wstr) {
    std::string result;

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    if (len == 0)
        return result;

    result.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), result.data(), len, nullptr, nullptr);
    result.pop_back();
    return result;
}

std::wstring toUtf16(std::string_view str) {
    std::wstring result;

    int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
    if (len == 0)
        return result;

    result.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), result.data(), len);
    result.pop_back();
    return result;
}
#endif

void winUseUtf8Crt() {
#ifdef _WINDOWS
    // Use UTF-8 for CRT functions.
    if (setlocale(LC_ALL, ".UTF-8") == nullptr)
        throw Exception("Could not change system locale to UTF-8");

    // Also use UTF-8 for console io.
    if (SetConsoleCP(CP_UTF8) == 0)
        throw Exception("Could not set console input codepage to UTF-8");
    if (SetConsoleOutputCP(CP_UTF8) == 0)
        throw Exception("Could not set console output codepage to UTF-8");
#endif
}

std::string u8getenv(const std::string &key) {
#ifdef _WINDOWS
    return toUtf8(_wgetenv(toUtf16(key).c_str()));
#else
    return std::getenv(key.c_str());
#endif
}
