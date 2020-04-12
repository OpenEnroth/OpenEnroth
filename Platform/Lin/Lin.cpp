#include "Engine/Point.h"

#include <string>
#include <vector>

#include <sys/time.h>

void OS_MsgBox(const char *msg, const char *title) {
    //MessageBoxA(nullptr, msg, title, 0);
}

unsigned int OS_GetTime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return unsigned((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

bool OS_IfShiftPressed() {
    //return GetAsyncKeyState(VK_SHIFT);
    return false;
}

bool OS_IfCtrlPressed() {
    //return GetAsyncKeyState(VK_CONTROL);
    return false;
}

void OS_ShowCursor(bool show) {
    //ShowCursor(show ? 1 : 0);
}

void OS_WaitMessage() {
    //WaitMessage();
}

void OS_Sleep(int ms) {
    //Sleep(ms);
}

Point OS_GetMouseCursorPos() {
    //POINT pt;
    //GetCursorPos(&pt);

    //SDL or XQueryPointer
    return Point(0, 0);
}

bool OS_OpenConsole() {
    /*if (AllocConsole()) {
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);

        return true;
    }*/

    return false;
}

std::vector<std::string> OS_FindFiles(const std::string& folder, const std::string& mask) {
    std::vector<std::string> result;

    /*std::string path = folder + "\\" + mask;

    WIN32_FIND_DATAA ffd = { 0 };
    HANDLE hFind = FindFirstFileA(path.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        return result;
    }

    do {
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            result.push_back(ffd.cFileName);
        }
    } while (FindNextFileA(hFind, &ffd) != 0);

    FindClose(hFind);*/

    return result;
}


//////////////////// There is no Windows Registry ////////////////////

bool OS_GetAppString(const char* path, char* out_string, int out_string_size) {
    return false;
}

int OS_GetAppInt(const char* pKey, int uDefValue) {
    unsigned int cbData;         // [sp+8h] [bp-20h]@1
    const char *lpValueName;     // [sp+Ch] [bp-1Ch]@1
    unsigned int dwDisposition;  // [sp+10h] [bp-18h]@2
    unsigned char Data[4];       // [sp+14h] [bp-14h]@5

    lpValueName = pKey;
    cbData = 4;
    *(int *)Data = uDefValue;

    /*
    HKEY hKey;            // [sp+18h] [bp-10h]@1
    HKEY phkResult;       // [sp+1Ch] [bp-Ch]@1
    HKEY v10;             // [sp+20h] [bp-8h]@1
    HKEY v11;             // [sp+24h] [bp-4h]@1

    v11 = 0;
    v10 = 0;
    hKey = 0;
    phkResult = 0;

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
    */
    return *(int *)Data;
}

void OS_SetAppString(const char* pKey, const char* pString) {}

void OS_GetAppString(const char* pKeyName, char* pOutString, int uBufLen,
    const char* pDefaultValue) {}

void OS_SetAppInt(const char* pKey, int val) {}
