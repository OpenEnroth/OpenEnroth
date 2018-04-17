#include <Windows.h>

#include "Application.h"

bool OS_GetAppStringRecursive(HKEY parent_key, const char *path,
                              char *out_string, int out_string_size) {
    char current_key[128];
    char path_tail[1024];

    const char *delimiter = strstr(path, "/");
    if (delimiter) {
        strncpy(current_key, path, delimiter - path);
        current_key[delimiter - path] = '\0';

        strcpy(path_tail, delimiter + 1);

        if (!parent_key) {
            if (!strcmpi(current_key, "HKEY_CLASSES_ROOT"))
                parent_key = HKEY_CLASSES_ROOT;
            else if (!strcmpi(current_key, "HKEY_CURRENT_CONFIG"))
                parent_key = HKEY_CURRENT_CONFIG;
            else if (!strcmpi(current_key, "HKEY_CURRENT_USER"))
                parent_key = HKEY_CURRENT_USER;
            else if (!strcmpi(current_key, "HKEY_LOCAL_MACHINE"))
                parent_key = HKEY_LOCAL_MACHINE;
            else if (!strcmpi(current_key, "HKEY_USERS"))
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

        bool result = false;
        HKEY key;
        if (!RegOpenKeyExA(parent_key, current_key, 0,
                           KEY_READ | KEY_WOW64_32KEY, &key)) {
            result = OS_GetAppStringRecursive(key, path_tail, out_string,
                                              out_string_size);
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

void GetDefaultConfig(ApplicationConfig &config) {
    char mm7_path[2048];

    bool gog_found = OS_GetAppString(
        "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM7/PATH",
        mm7_path, sizeof(mm7_path));
    if (gog_found) {
        config.mm7_install_type = "GoG MM7 installation";
        config.mm7_install_path = mm7_path;
        return;
    }

    bool std_found = OS_GetAppString(
        "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic VII/1.0/AppPath",
        mm7_path, sizeof(mm7_path));
    if (std_found) {
        config.mm7_install_type = "Standard MM7 1.0 installation";
        config.mm7_install_path = mm7_path;
        return;
    }
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, char *, int) {
    ApplicationConfig config;
    GetDefaultConfig(config);

    (new Application())->Configure(config)->Run();
    return 0;
}
