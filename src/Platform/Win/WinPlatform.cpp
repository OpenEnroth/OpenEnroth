#include "WinPlatform.h"

#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Utility/String.h"

static bool OS_GetAppStringRecursive(HKEY parent_key, const char *path, char *out_string, int out_string_size, int flags) {
    char current_key[128];
    char path_tail[1024];

    const char *delimiter = strstr(path, "/");
    if (delimiter) {
        strncpy(current_key, path, delimiter - path);
        current_key[delimiter - path] = '\0';

        strcpy(path_tail, delimiter + 1);

        if (!parent_key) {
            if (iequals(current_key, "HKEY_CLASSES_ROOT"))
                parent_key = HKEY_CLASSES_ROOT;
            else if (iequals(current_key, "HKEY_CURRENT_CONFIG"))
                parent_key = HKEY_CURRENT_CONFIG;
            else if (iequals(current_key, "HKEY_CURRENT_USER"))
                parent_key = HKEY_CURRENT_USER;
            else if (iequals(current_key, "HKEY_LOCAL_MACHINE"))
                parent_key = HKEY_LOCAL_MACHINE;
            else if (iequals(current_key, "HKEY_USERS"))
                parent_key = HKEY_USERS;
            else
                return false;

            delimiter = strstr(path_tail, "/");
            if (delimiter) {
                strncpy(current_key, path_tail, delimiter - path_tail);
                current_key[delimiter - path_tail] = '\0';

                strcpy(path_tail, delimiter + 1);
            } else {
                return false;
            }
        }

        if (!strcmp(current_key, "WOW6432Node")) {
            return OS_GetAppStringRecursive(parent_key, path_tail, out_string, out_string_size, KEY_WOW64_32KEY);
        }

        bool result = false;
        HKEY key;
        if (!RegOpenKeyExA(parent_key, current_key, 0, KEY_READ | flags, &key)) {
            result = OS_GetAppStringRecursive(key, path_tail, out_string, out_string_size, 0);
            RegCloseKey(key);
        }

        return result;
    } else {
        DWORD data_size = out_string_size;
        if (RegQueryValueExA(parent_key, path, nullptr, nullptr, (LPBYTE)out_string, &data_size))
            return false;
        return true;
    }
}

WinPlatform::WinPlatform(Log *log) : SdlPlatform(log) {}

void WinPlatform::WinEnsureConsole() const {
    if (AllocConsole()) {
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
    }
}

std::string WinPlatform::WinQueryRegistry(const std::string &path) const {
    char buffer[8192];
    if (OS_GetAppStringRecursive(nullptr, path.c_str(), buffer, sizeof(buffer), 0))
        return buffer;
    return {};
}

std::unique_ptr<Platform> Platform::CreateStandardPlatform(Log *log) {
    return std::make_unique<WinPlatform>(log);
}
