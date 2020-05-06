#include "Platform/Win/Win.h"

#include <cstdio>
#include <string>
#include <vector>

#include "src/Application/Game.h"

#include "Platform/Api.h"

#ifdef _WINDOWS
#pragma comment(lib, "winmm.lib")
#else
#include <sys/time.h>
#endif

void OS_MsgBox(const char *msg, const char *title) {
    MessageBoxA(nullptr, msg, title, 0);
}

unsigned int OS_GetTime() {
#ifdef _WINDOWS
        return GetTickCount();
#else
        struct timeval tv;
        gettimeofday(&tv, 0);
        return unsigned((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
#endif
}

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
