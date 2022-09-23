#include "Platform/Win/Win.h"

#include <io.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "Application/Game.h"

#include "Platform/Api.h"

#pragma comment(lib, "winmm.lib")

void OS_MsgBox(const char *msg, const char *title) {
    MessageBoxA(nullptr, msg, title, 0);
}

unsigned int OS_GetTime() { return GetTickCount(); }

unsigned __int64 OS_GetPrecisionTime() { return timeGetTime(); }

void OS_ShowCursor(bool show) { ShowCursor(show ? 1 : 0); }

void OS_Sleep(int ms) { Sleep(ms); }

bool OS_OpenConsole() {
    if (AllocConsole()) {
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);

        return true;
    }

    return false;
}

std::vector<std::string> OS_FindFiles(const std::string &folder, const std::string &mask) {
    std::vector<std::string> result;

    std::string path = folder + "\\" + mask;

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

    FindClose(hFind);

    return result;
}

std::filesystem::path OS_makepath(std::string path) {
    return path;
}

std::filesystem::path OS_casepath(std::filesystem::path path) {
    return path;
}

FILE *OS_fopen(std::filesystem::path path, const char *mode) {
    std::wstring wmode;
    if (mode)
        for (; *mode; mode++)
            wmode.push_back(*mode);

    return _wfopen(path.c_str(), wmode.c_str());
}

bool OS_FileExists(const std::string& path) {
    return _access(path.c_str(), 0) != -1;
}
