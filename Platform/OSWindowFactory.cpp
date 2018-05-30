#include "Platform/OSWindowFactory.h"

#include <stdio.h>
#include <Windows.h>

#include "Platform/Api.h"
#include "Platform/WinApiWindow.h"
#include "Platform/Sdl2Window.h"

OSWindow *OSWindowFactory::Create(const wchar_t *title, int window_width, int window_height) {
    return CreateSdl2(title, window_width, window_height);
}

OSWindow *OSWindowFactory::CreateSdl2(const wchar_t *title, int window_width, int window_height) {
    char title_a[1024];
    sprintf(title_a, "%S", title);

    auto sdlWindow = SDL_CreateWindow(
        title_a,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height,
        SDL_WINDOW_SHOWN);

    if (sdlWindow) {
        auto sdlWindowSurface = SDL_GetWindowSurface(sdlWindow);
        if (sdlWindowSurface) {
            return new Sdl2Window(sdlWindow, sdlWindowSurface);
        } else {
            SDL_DestroyWindow(sdlWindow);
        }
    }

    return nullptr;
}


OSWindow *OSWindowFactory::CreateWinApi(const wchar_t *title, int window_width, int window_height) {
    WNDCLASSEXW wcxw;
    wcxw.cbSize = sizeof(WNDCLASSEXW);
    wcxw.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wcxw.lpfnWndProc = (WNDPROC)WinApiMsgRouter;
    wcxw.cbClsExtra = 0;
    wcxw.cbWndExtra = 0;
    wcxw.hInstance = GetModuleHandleW(nullptr);
    wcxw.hIcon = wcxw.hIconSm = LoadIcon(NULL, IDI_QUESTION);
    wcxw.hCursor = LoadCursor(NULL, IDC_WAIT);
    wcxw.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcxw.lpszMenuName = nullptr;
    wcxw.lpszClassName = L"M&MTrilogy";
    if (!RegisterClassExW(&wcxw)) {
        return false;
    }

    auto api_handle = CreateWindowExW(
        0, wcxw.lpszClassName, title, 0,
        OS_GetAppInt("window X", (GetSystemMetrics(SM_CXSCREEN) - window_width) / 2),
        OS_GetAppInt("window Y", (GetSystemMetrics(SM_CYSCREEN) - window_height) / 2),
        window_width, window_height, nullptr, nullptr, wcxw.hInstance, this);

    if (!api_handle) {
        UnregisterClassW(wcxw.lpszClassName, wcxw.hInstance);
        return nullptr;
    }


    auto window = new WinApiWindow(api_handle);
    if (window) {
        window->SetWindowedMode(window_width, window_height);
        window->Show();
        window->Activate();
    }

    return window;
}

int main(int argc, char *argv[]) {
    extern int MM_Main(const char *);
    return MM_Main(argv[0]);
}
