#include "Platform/Win/Win.h"

#include <io.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "Platform/Api.h"

#pragma comment(lib, "winmm.lib")

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

