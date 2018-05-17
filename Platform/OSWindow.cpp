#include "Platform/OSWindow.h"
#if defined (_WIN32)
#include "Platform/Win/WinOSWindow.h"
#else
    // TODO(farmboy9): other platforms
#endif

OSWindow *OSWindow::Create(const wchar_t *title, int window_width,
                           int window_height) {
#if defined (_WIN32)
    return WinOSWindow::Create(title, window_width, window_height);
#else
    // TODO(farmboy9): other platforms
#endif
}
