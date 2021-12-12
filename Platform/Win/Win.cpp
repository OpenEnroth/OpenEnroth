#include "Platform/Win/Win.h"

#include <SDL2/SDL.h>

#include <cstdio>
#include <string>
#include <vector>

#include "src/Application/Game.h"

#include "Platform/Api.h"

#pragma comment(lib, "winmm.lib")

void OS_MsgBox(const char *msg, const char *title) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, nullptr);
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


FILE *fcaseopen(char const *path, char const *mode) {
    return fopen(path, mode);
}

std::string OS_GetDirSeparator() {
    return "/";
}
