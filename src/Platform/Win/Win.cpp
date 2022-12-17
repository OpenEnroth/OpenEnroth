#include "Platform/Win/Win.h"

#include <io.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "Platform/Api.h"

#pragma comment(lib, "winmm.lib")

unsigned int OS_GetTime() { return GetTickCount(); }

uint64_t OS_GetPrecisionTime() { return timeGetTime(); }

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

std::filesystem::path OS_makepath(std::string path) {
    return path;
}

std::filesystem::path OS_casepath(std::filesystem::path path) {
    return path;
}

bool OS_FileExists(const std::string& path) {
    return _access(path.c_str(), 0) != -1;
}
