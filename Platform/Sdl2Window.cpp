#include "Platform/Sdl2Window.h"

#include "lib/SDL2/include/SDL_syswm.h"

void Sdl2Window::MessageProc(const SDL_Event &e) {
    switch (e.type) {
        case SDL_QUIT: {
            extern void Engine_DeinitializeAndTerminate(int exit_code);
            Engine_DeinitializeAndTerminate(0);
        } break;

        case SDL_APP_WILLENTERBACKGROUND: {
            gameCallback->OnDeactivated();
        } break;

        case SDL_APP_WILLENTERFOREGROUND: {
            gameCallback->OnActivated();
        } break;

        case SDL_KEYUP: {
            if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                extern bool _507B98_ctrl_pressed;
                _507B98_ctrl_pressed = false;
            } else if (e.key.keysym.sym == SDLK_PRINTSCREEN) {
                gameCallback->OnScreenshot();
            }
        } break;

        case SDL_KEYDOWN: {
            auto sdlk = e.key.keysym.sym;
            if (SdlkIsChar(sdlk)) {
                gameCallback->OnChar(sdlk);
            } else {
                // SDLK_*  _SEEMS_ identical to VK_*, maybe will need some special processing on occasion
                gameCallback->OnVkDown(sdlk, SdlkIsChar(sdlk) ? (int)sdlk: 0);
            }
        }

        case SDL_MOUSEMOTION: {
            gameCallback->OnMouseMove(
                e.motion.x, e.motion.y,
                e.motion.state & SDL_BUTTON_LMASK, e.motion.state & SDL_BUTTON_RMASK);
        }

        case SDL_MOUSEBUTTONDOWN: {
            int x = e.button.x;
            int y = e.button.y;
            int clicks = e.button.clicks;

            // if (e.button.state == SDL_PRESSED) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    if (clicks == 1) {
                        gameCallback->OnMouseLeftClick(x, y);
                    } else if (clicks >= 2) {
                        gameCallback->OnMouseLeftDoubleClick(x, y);
                    }
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    if (clicks == 1) {
                        gameCallback->OnMouseRightClick(x, y);
                    } else if (clicks >= 2) {
                        gameCallback->OnMouseRightDoubleClick(x, y);
                    }
                }
            // }
        } break;

        case SDL_MOUSEBUTTONUP: {
            // if (e.button.state == SDL_RELEASED) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    gameCallback->OnMouseLeftUp();
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    gameCallback->OnMouseRightUp();
                }
            // }
        } break;

        case SDL_WINDOWEVENT: {
            switch (e.window.event) {
                case SDL_WINDOWEVENT_EXPOSED: {
                    gameCallback->OnPaint();
                } break;
            }
        } break;
    }
}

void Sdl2Window::PeekMessageLoop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        MessageProc(e);
    }
}

void Sdl2Window::PeekSingleMessage() {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        MessageProc(e);
    }
}

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
    int x;
    SDL_GetWindowPosition(sdlWindow, &x, nullptr);

    return x;
}

int Sdl2Window::GetY() const {
    int y;
    SDL_GetWindowPosition(sdlWindow, nullptr, &y);

    return y;
}

unsigned int Sdl2Window::GetWidth() const {
    int w;
    SDL_GetWindowSize(sdlWindow, &w, nullptr);

    return w;
}
unsigned int Sdl2Window::GetHeight() const {
    int h;
    SDL_GetWindowSize(sdlWindow, nullptr, &h);

    return h;
}

Point Sdl2Window::TransformCursorPos(Point &pt) const {
    SDL_Point p;
    SDL_GetMouseState(&p.x, &p.y);

    return Point(p.x, p.y);
}

bool Sdl2Window::OnOSMenu(int item_id) {
    return false;
}

void Sdl2Window::Show() {
    SDL_ShowWindow(sdlWindow);
    SDL_RaiseWindow(sdlWindow);

    PeekMessageLoop();
}

bool Sdl2Window::Focused() {
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
    return (SDL_GetWindowFlags(sdlWindow) & flags) == flags;
}

void Sdl2Window::Activate() {
    SDL_RaiseWindow(sdlWindow);

    PeekMessageLoop();
}

bool Sdl2Window::SdlkIsChar(SDL_Keycode key) const {
    return (int)key >= '0' && (int)key <= 'Z';
}
