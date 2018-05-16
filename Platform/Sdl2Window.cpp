#include "Platform/Sdl2Window.h"

#include "lib/SDL2/include/SDL_syswm.h"

void *Sdl2Window::GetWinApiHandle() {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    SDL_GetWindowWMInfo(sdlWindow, &wmInfo);
    return (void *)wmInfo.info.win.window;
}


void Sdl2Window::SetFullscreenMode() {
}
void Sdl2Window::SetWindowedMode(int new_window_width, int new_window_height) {
}
void Sdl2Window::SetCursor(const char *cursor_name) {
}

int Sdl2Window::GetX() const {
}
int Sdl2Window::GetY() const {
}
unsigned int Sdl2Window::GetWidth() const {
}
unsigned int Sdl2Window::GetHeight() const {
}

Point Sdl2Window::TransformCursorPos(Point &pt) const {
}

bool Sdl2Window::OnOSMenu(int item_id) {
}

void Sdl2Window::Show() {
}
bool Sdl2Window::Focused() {
}
bool Sdl2Window::OnMouseLeftClick(int x, int y) {
}
bool Sdl2Window::OnMouseRightClick(int x, int y) {
}
bool Sdl2Window::Activate() {
}
