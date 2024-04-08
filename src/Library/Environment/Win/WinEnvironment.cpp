#include "WinEnvironment.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdlib>
#include <array>
#include <memory>

#include "Utility/Win/Unicode.h"

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

std::string WinEnvironment::queryRegistry(const std::string &path) const {
    return win::toUtf8(OS_GetAppStringRecursive(NULL, win::toUtf16(path).c_str(), 0));
}

std::string WinEnvironment::path(EnvironmentPath path) const {
    if (path == PATH_HOME) {
        return getenv("USERPROFILE");
    } else {
        return {};
    }
}

std::string WinEnvironment::getenv(const std::string &key) const {
    const wchar_t *result = _wgetenv(win::toUtf16(key).c_str());
    if (result)
        return win::toUtf8(result);
    return {};
}

void WinEnvironment::setenv(const std::string &key, const std::string &value) const {
    _wputenv_s(win::toUtf16(key).c_str(), win::toUtf16(value).c_str()); // Errors are ignored.
}

std::unique_ptr<Environment> Environment::createStandardEnvironment() {
    return std::make_unique<WinEnvironment>();
}
