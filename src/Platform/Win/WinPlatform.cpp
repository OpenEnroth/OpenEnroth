#include "WinPlatform.h"

#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Platform/Sdl/SdlLogger.h"

#include "Utility/String.h"

static bool OS_GetAppStringRecursive(HKEY parent_key, const wchar_t *path, char *out_string, int out_string_size, int flags) {
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
                return false;

            delimiter = wcsstr(path_tail, L"/");
            if (delimiter) {
                wcsncpy(current_key, path_tail, delimiter - path_tail);
                current_key[delimiter - path_tail] = L'\0';

                wcscpy(path_tail, delimiter + 1);
            } else {
                return false;
            }
        }

        if (!wcscmp(current_key, L"WOW6432Node")) {
            return OS_GetAppStringRecursive(parent_key, path_tail, out_string, out_string_size, KEY_WOW64_32KEY);
        }

        bool result = false;
        HKEY key;
        if (!RegOpenKeyExW(parent_key, current_key, 0, KEY_READ | flags, &key)) {
            result = OS_GetAppStringRecursive(key, path_tail, out_string, out_string_size, 0);
            RegCloseKey(key);
        }

        return result;
    } else {
        auto regValue = std::make_unique<wchar_t[]>(out_string_size + 1);
        for (int i = 0; i < out_string_size; ++i) {
            regValue.get()[i] = L'x';
            out_string[i] = 'x';
        }
        DWORD regNumBytesRead = sizeof(wchar_t) * (out_string_size + 1);
        LSTATUS status = RegGetValueW(
            parent_key,
            nullptr,
            path,
            RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ,
            nullptr,
            regValue.get(),
            &regNumBytesRead
        );
        if (status == ERROR_SUCCESS) {
            if (regNumBytesRead > 0) {
                int numBytesConverted = WideCharToMultiByte(CP_ACP, 0, regValue.get(), -1, out_string, out_string_size, nullptr, nullptr);
                if (0 == numBytesConverted) {
                    status = ERROR_PATH_NOT_FOUND;
                }
            } else {
                status = ERROR_PATH_NOT_FOUND;
            }
        }

        if (status != ERROR_SUCCESS) {
            strcpy(out_string, "");
        }

        return status == ERROR_SUCCESS;
    }
}

std::string WinPlatform::winQueryRegistry(const std::wstring &path) const {
    char buffer[8192];
    if (OS_GetAppStringRecursive(nullptr, path.c_str(), buffer, sizeof(buffer), 0))
        return std::string(buffer);
    return {};
}

std::unique_ptr<Platform> Platform::createStandardPlatform(PlatformLogger *logger) {
    return std::make_unique<WinPlatform>(logger);
}

std::unique_ptr<PlatformLogger> PlatformLogger::createStandardLogger() {
    return std::make_unique<SdlLogger>();
}
