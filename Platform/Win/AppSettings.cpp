#include "Platform/Win/Win.h"

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

//----- (004649EF) --------------------------------------------------------
int OS_GetAppInt(const char *pKey, int uDefValue) {
    DWORD cbData;         // [sp+8h] [bp-20h]@1
    LPCSTR lpValueName;   // [sp+Ch] [bp-1Ch]@1
    DWORD dwDisposition;  // [sp+10h] [bp-18h]@2
    BYTE Data[4];         // [sp+14h] [bp-14h]@5
    HKEY hKey;            // [sp+18h] [bp-10h]@1
    HKEY phkResult;       // [sp+1Ch] [bp-Ch]@1
    HKEY v10;             // [sp+20h] [bp-8h]@1
    HKEY v11;             // [sp+24h] [bp-4h]@1

    lpValueName = pKey;
    v11 = 0;
    v10 = 0;
    hKey = 0;
    phkResult = 0;
    cbData = 4;
    *(int *)Data = uDefValue;

    if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0,
                       KEY_READ | KEY_WOW64_32KEY, &hKey)) {  // for 64 bit
        if (!RegCreateKeyExA(hKey, "New World Computing", 0, "", 0,
                             KEY_ALL_ACCESS, 0, &phkResult, &dwDisposition)) {
            if (!RegCreateKeyExA(phkResult, "Might and Magic VII", 0, "", 0,
                                 KEY_ALL_ACCESS, 0, &v10, &dwDisposition)) {
                if (!RegCreateKeyExA(v10, "1.0", 0, "", 0, KEY_ALL_ACCESS, 0,
                                     &v11, &dwDisposition)) {
                    LSTATUS status;
                    if (status = RegQueryValueExA(v11, lpValueName, 0, 0, Data,
                                                  &cbData)) {
                        status;
                        GetLastError();
                        RegSetValueExA(v11, lpValueName, 0, 4, Data, 4);
                    }
                    RegCloseKey(v11);
                }
                RegCloseKey(v10);
            }
            RegCloseKey(phkResult);
        }
        RegCloseKey(hKey);
    }
    return *(int *)Data;
}

//----- (00464B02) --------------------------------------------------------
void OS_SetAppString(const char *pKey, const char *pString) {
    size_t v2;                // eax@5
    const char *lpValueName;  // [sp+4h] [bp-1Ch]@1
    const char *Str;          // [sp+8h] [bp-18h]@1
    DWORD dwDisposition;      // [sp+Ch] [bp-14h]@2
    HKEY hKey;                // [sp+10h] [bp-10h]@1
    HKEY phkResult;           // [sp+14h] [bp-Ch]@1
    HKEY v8;                  // [sp+18h] [bp-8h]@1
    HKEY v9;                  // [sp+1Ch] [bp-4h]@1

    Str = pString;
    lpValueName = pKey;
    v9 = 0;
    v8 = 0;
    hKey = 0;
    phkResult = 0;
    if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0,
                       KEY_READ | KEY_WOW64_32KEY, &hKey)) {
        if (!RegCreateKeyExA(hKey, "New World Computing", 0, "", 0,
                             KEY_ALL_ACCESS, 0, &phkResult, &dwDisposition)) {
            if (!RegCreateKeyExA(phkResult, "Might and Magic VII", 0, "", 0,
                                 KEY_ALL_ACCESS, 0, &v8, &dwDisposition)) {
                if (!RegCreateKeyExA(v8, "1.0", 0, "", 0, KEY_ALL_ACCESS, 0,
                                     &v9, &dwDisposition)) {
                    v2 = strlen(Str);
                    RegSetValueExA(v9, lpValueName, 0, 1, (const BYTE *)Str,
                                   v2 + 1);
                    RegCloseKey(v9);
                }
                RegCloseKey(v8);
            }
            RegCloseKey(phkResult);
        }
        RegCloseKey(hKey);
    }
}

//----- (00464BEF) --------------------------------------------------------
void OS_GetAppString(const char *pKeyName, char *pOutString, int uBufLen,
                     const char *pDefaultValue) {
    // LSTATUS (__stdcall *v4)(HKEY); // esi@1
    LSTATUS result;       // eax@7
    DWORD Type;           // [sp+Ch] [bp-24h]@5
    LPCSTR lpValueName;   // [sp+10h] [bp-20h]@1
    DWORD cbData;         // [sp+14h] [bp-1Ch]@1
    LPBYTE Dest;          // [sp+18h] [bp-18h]@1
    DWORD dwDisposition;  // [sp+1Ch] [bp-14h]@2
    HKEY phkResult;       // [sp+20h] [bp-10h]@1
    HKEY hKey;            // [sp+24h] [bp-Ch]@1
    HKEY v13;             // [sp+28h] [bp-8h]@1
    HKEY v14;             // [sp+2Ch] [bp-4h]@1

    cbData = uBufLen;
    Dest = (LPBYTE)pOutString;
    lpValueName = pKeyName;
    v14 = 0;
    v13 = 0;
    hKey = 0;
    phkResult = 0;
    result = (LSTATUS)strncpy((char *)Dest, pDefaultValue, uBufLen);
    if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0,
                       KEY_READ | KEY_WOW64_32KEY, &hKey)) {
        if (!RegCreateKeyExA(hKey, "New World Computing", 0, "", 0,
                             KEY_ALL_ACCESS, 0, &phkResult, &dwDisposition)) {
            if (!RegCreateKeyExA(phkResult, "Might and Magic VII", 0, "", 0,
                                 KEY_ALL_ACCESS, 0, &v13, &dwDisposition)) {
                if (!RegCreateKeyExA(v13, "1.0", 0, "", 0, KEY_ALL_ACCESS, 0,
                                     &v14, &dwDisposition)) {
                    if (RegQueryValueExA(v14, lpValueName, 0, &Type, Dest,
                                         &cbData))
                        GetLastError();
                    RegCloseKey(v14);
                    v14 = NULL;
                }
                RegCloseKey(v13);
                v13 = NULL;
            }
            RegCloseKey(phkResult);
            phkResult = NULL;
        }
        RegCloseKey(hKey);
        hKey = NULL;
    }
}

//----- (00464D32) --------------------------------------------------------
void OS_SetAppInt(const char *pKey, int val) {
    const char *lpValueName;  // [sp+4h] [bp-1Ch]@1
    BYTE Data[4];             // [sp+8h] [bp-18h]@1
    DWORD dwDisposition;      // [sp+Ch] [bp-14h]@2
    HKEY hKey;                // [sp+10h] [bp-10h]@1
    HKEY phkResult;           // [sp+14h] [bp-Ch]@1
    HKEY v7;                  // [sp+18h] [bp-8h]@1
    HKEY v8;                  // [sp+1Ch] [bp-4h]@1

    *(int *)Data = val;
    lpValueName = pKey;
    v8 = 0;
    v7 = 0;
    hKey = 0;
    phkResult = 0;
    if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0,
                       KEY_READ | KEY_WOW64_32KEY, &hKey)) {
        if (!RegCreateKeyExA(hKey, "New World Computing", 0, "", 0,
                             KEY_ALL_ACCESS, 0, &phkResult, &dwDisposition)) {
            if (!RegCreateKeyExA(phkResult, "Might and Magic VII", 0, "", 0,
                                 KEY_ALL_ACCESS, 0, &v7, &dwDisposition)) {
                if (!RegCreateKeyExA(v7, "1.0", 0, "", 0, KEY_ALL_ACCESS, 0,
                                     &v8, &dwDisposition)) {
                    RegSetValueExA(v8, lpValueName, 0, 4, Data, 4);
                    RegCloseKey(v8);
                }
                RegCloseKey(v7);
            }
            RegCloseKey(phkResult);
        }
        RegCloseKey(hKey);
    }
}
