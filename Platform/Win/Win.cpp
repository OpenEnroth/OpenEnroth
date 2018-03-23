#include "Platform/Win/Win.h"

#include <cstdio>
#include <string>
#include <vector>

#include "src/Application/Game.h"

#include "Platform/Api.h"

#pragma comment(lib, "winmm.lib")


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprevinstance, LPSTR lpCmdLine, int nShowCmd) {
    extern int MM_Main(const char *);
    return MM_Main(lpCmdLine);
}

void OS_MsgBox(const wchar_t *msg, const wchar_t *title) {
    MessageBoxW(nullptr, msg, title, 0);
}

void OS_PeekMessage() {
    MSG msg;
    if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            extern void Engine_DeinitializeAndTerminate(int exit_code);
            Engine_DeinitializeAndTerminate(0);
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

unsigned int OS_GetTime() { return GetTickCount(); }

unsigned __int64 OS_GetPrecisionTime() { return timeGetTime(); }

bool OS_IfShiftPressed() { return GetAsyncKeyState(VK_SHIFT); }

bool OS_IfCtrlPressed() { return GetAsyncKeyState(VK_CONTROL); }

void OS_ShowCursor(bool show) { ShowCursor(show ? 1 : 0); }

void OS_PeekMessageLoop() {
    MSG msg;
    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            extern void Engine_DeinitializeAndTerminate(int);
            Engine_DeinitializeAndTerminate(0);
        }

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void OS_WaitMessage() { WaitMessage(); }

void OS_Sleep(int ms) { Sleep(ms); }

Point OS_GetMouseCursorPos() {
    POINT pt;
    GetCursorPos(&pt);

    return Point(pt.x, pt.y);
}

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
