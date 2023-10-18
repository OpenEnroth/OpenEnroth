#include "System.h"

#ifdef _WINDOWS
#   include <clocale>
#   include <cassert>
#   include <array>

#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
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
    return result;
}

std::wstring toUtf16(std::string_view str) {
    std::wstring result;

    int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
    if (len == 0)
        return result;

    result.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), result.data(), len);
    return result;
}

// TODO(captainurist): revisit this code once I'm on a win machine.
static std::wstring OS_GetAppStringRecursive(HKEY parent_key, const wchar_t *path, int flags) {
    wchar_t current_key[256];
    wchar_t path_tail[4096];

    const wchar_t *delimiter = wcsstr(path, L"/");
    if (delimiter) {
        wcsncpy(current_key, path, delimiter - path);
        current_key[delimiter - path] = L'\0';

        wcscpy(path_tail, delimiter + 1);

        if (!parent_key) {
            if (!wcsicmp(current_key, L"HKEY_CLASSES_ROOT"))
                parent_key = HKEY_CLASSES_ROOT;
            else if (!wcsicmp(current_key, L"HKEY_CURRENT_CONFIG"))
                parent_key = HKEY_CURRENT_CONFIG;
            else if (!wcsicmp(current_key, L"HKEY_CURRENT_USER"))
                parent_key = HKEY_CURRENT_USER;
            else if (!wcsicmp(current_key, L"HKEY_LOCAL_MACHINE"))
                parent_key = HKEY_LOCAL_MACHINE;
            else if (!wcsicmp(current_key, L"HKEY_USERS"))
                parent_key = HKEY_USERS;
            else
                return {};

            delimiter = wcsstr(path_tail, L"/");
            if (delimiter) {
                wcsncpy(current_key, path_tail, delimiter - path_tail);
                current_key[delimiter - path_tail] = L'\0';

                wcscpy(path_tail, delimiter + 1);
            } else {
                return {};
            }
        }

        if (!wcscmp(current_key, L"WOW6432Node")) {
            return OS_GetAppStringRecursive(parent_key, path_tail, KEY_WOW64_32KEY);
        }

        HKEY key;
        if (!RegOpenKeyExW(parent_key, current_key, 0, KEY_READ | flags, &key)) {
            std::wstring result = OS_GetAppStringRecursive(key, path_tail, 0);
            RegCloseKey(key);
            return result;
        }

        return {};
    } else {
        std::array<wchar_t, 8192> buffer = {{}};
        DWORD regNumBytesRead = sizeof(buffer);
        LSTATUS status = RegGetValueW(
            parent_key,
            nullptr,
            path,
            RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ,
            nullptr,
            buffer.data(),
            &regNumBytesRead
        );
        if (status == ERROR_SUCCESS)
            return buffer.data();

        return {};
    }
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

std::string winQueryRegistry(const std::string &path) {
#ifdef _WINDOWS
    return toUtf8(OS_GetAppStringRecursive(NULL, toUtf16(path).c_str(), 0));
#else
    return {};
#endif
}

std::string u8getenv(const std::string &key) {
#ifdef _WINDOWS
    const wchar_t *result = _wgetenv(toUtf16(key).c_str());
    if (result)
        return toUtf8(result);
    return {};
#else
    const char *result = std::getenv(key.c_str());
    if (result)
        return result;
    return {};
#endif
}
