#include "Platform/Win/Win.h"

#include "Engine/Strings.h"

bool OS_GetAppStringRecursive(
    HKEY parent_key,
    const char *path,
    char *out_string,
    int out_string_size,
    int flags = 0
) {
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
            return OS_GetAppStringRecursive(
                parent_key,
                path_tail,
                out_string,
                out_string_size,
                KEY_WOW64_32KEY
            );
        }

        bool result = false;
        HKEY key;
        if (!RegOpenKeyExA(
                parent_key,
                current_key,
                0,
                KEY_READ | flags,
                &key
            )
        ) {
            result = OS_GetAppStringRecursive(
                key,
                path_tail,
                out_string,
                out_string_size
            );
            RegCloseKey(key);
        }

        return result;
    } else {
        DWORD data_size = out_string_size;
        if (RegQueryValueExA(parent_key, path, nullptr, nullptr,
                             (LPBYTE)out_string, &data_size))
            return false;
        return true;
    }
}

bool OS_GetAppString(const char *path, char *out_string, int out_string_size) {
    return OS_GetAppStringRecursive(nullptr, path, out_string, out_string_size);
}
