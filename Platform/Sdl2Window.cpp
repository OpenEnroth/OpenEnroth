#include <map>

#include "Platform/Sdl2Window.h"

#include "SDL_syswm.h"

#include "IO/Keyboard.h"

void Sdl2Window::MessageProc(const SDL_Event &e) {
    switch (e.type) {
        case SDL_QUIT: {
            extern void Engine_DeinitializeAndTerminate(int exit_code);
            Engine_DeinitializeAndTerminate(0);
        } break;

        case SDL_APP_WILLENTERBACKGROUND: {  // this is for mobile apps wrong event
            gameCallback->OnDeactivated();
        } break;

        case SDL_APP_WILLENTERFOREGROUND: {  // this is for mobile apps wrong event
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
            GameKey mappedKey;
            if (TryMapScanCode(e.key.keysym.scancode, &mappedKey)) {
                if (IsKeyUsedInTextInput(mappedKey)) {
                    // send key in case textinput waits for it
                    gameCallback->OnChar(mappedKey, -1);
                }
                gameCallback->OnKey(mappedKey);
            }

            int mappedChar;
            auto mods = e.key.keysym.mod;
            if (TryMapKeyCode(e.key.keysym.sym, (mods & KMOD_CAPS) != (mods & KMOD_SHIFT), &mappedChar)) {
                gameCallback->OnChar(GameKey::Char, mappedChar);
            }
        } break;

        case SDL_MOUSEMOTION: {
            gameCallback->OnMouseMove(
                e.motion.x, e.motion.y,
                e.motion.state & SDL_BUTTON_LMASK, e.motion.state & SDL_BUTTON_RMASK
            );
        } break;

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
                case SDL_WINDOWEVENT_FOCUS_LOST: {
                    gameCallback->OnDeactivated();
                } break;
                case SDL_WINDOWEVENT_FOCUS_GAINED: {
                    gameCallback->OnActivated();
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
#ifdef _WINDOWS
    return (void *)wmInfo.info.win.window;
#elif __APPLE__
    return (void *)wmInfo.info.cocoa.window;
#else
    return (void *)wmInfo.info.x11.window;
#endif
}


void Sdl2Window::SetFullscreenMode() {
}

void Sdl2Window::SetWindowedMode(int new_window_width, int new_window_height) {
}

void Sdl2Window::SetCursor(bool on = 1) {
    if (on) {
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        SDL_ShowCursor(SDL_DISABLE);
    }
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


bool Sdl2Window::TryMapKeyCode(SDL_Keycode key, bool uppercase, int* outKey) const {
    if (key >= SDLK_0 && key <= SDLK_9) {
        if (outKey != nullptr) {
            *outKey = '0' + (key - SDLK_0);
            return true;
        }
    }
    if (key >= SDLK_a && key <= SDLK_z) {
        if (outKey != nullptr) {
            if (uppercase) {
                *outKey = 'A' + (key - SDLK_a);
            }
            else {
                *outKey = 'a' + (key - SDLK_a);
            }
        }
        return true;
    }
    return false;
}

std::map<SDL_Scancode, GameKey> scancode_lookup =
{
    { SDL_SCANCODE_F1,              GameKey::F1 },
    { SDL_SCANCODE_F2,              GameKey::F2 },
    { SDL_SCANCODE_F3,              GameKey::F3 },
    { SDL_SCANCODE_F4,              GameKey::F4 },
    { SDL_SCANCODE_F5,              GameKey::F5 },
    { SDL_SCANCODE_F6,              GameKey::F6 },
    { SDL_SCANCODE_F7,              GameKey::F7 },
    { SDL_SCANCODE_F8,              GameKey::F8 },
    { SDL_SCANCODE_F9,              GameKey::F9 },
    { SDL_SCANCODE_F10,             GameKey::F10 },
    { SDL_SCANCODE_F11,             GameKey::F11 },
    { SDL_SCANCODE_F12,             GameKey::F12 },

    { SDL_SCANCODE_1,               GameKey::Digit1 },
    { SDL_SCANCODE_2,               GameKey::Digit2 },
    { SDL_SCANCODE_3,               GameKey::Digit3 },
    { SDL_SCANCODE_4,               GameKey::Digit4 },
    { SDL_SCANCODE_5,               GameKey::Digit5 },
    { SDL_SCANCODE_6,               GameKey::Digit6 },
    { SDL_SCANCODE_7,               GameKey::Digit7 },
    { SDL_SCANCODE_8,               GameKey::Digit8 },
    { SDL_SCANCODE_9,               GameKey::Digit9 },
    { SDL_SCANCODE_0,               GameKey::Digit0 },

    { SDL_SCANCODE_A,               GameKey::A },
    { SDL_SCANCODE_B,               GameKey::B },
    { SDL_SCANCODE_C,               GameKey::C },
    { SDL_SCANCODE_D,               GameKey::D },
    { SDL_SCANCODE_E,               GameKey::E },
    { SDL_SCANCODE_F,               GameKey::F },
    { SDL_SCANCODE_G,               GameKey::G },
    { SDL_SCANCODE_H,               GameKey::H },
    { SDL_SCANCODE_I,               GameKey::I },
    { SDL_SCANCODE_J,               GameKey::J },
    { SDL_SCANCODE_K,               GameKey::K },
    { SDL_SCANCODE_L,               GameKey::L },
    { SDL_SCANCODE_M,               GameKey::M },
    { SDL_SCANCODE_N,               GameKey::N },
    { SDL_SCANCODE_O,               GameKey::O },
    { SDL_SCANCODE_P,               GameKey::P },
    { SDL_SCANCODE_Q,               GameKey::Q },
    { SDL_SCANCODE_E,               GameKey::R },
    { SDL_SCANCODE_S,               GameKey::S },
    { SDL_SCANCODE_T,               GameKey::T },
    { SDL_SCANCODE_U,               GameKey::U },
    { SDL_SCANCODE_V,               GameKey::V },
    { SDL_SCANCODE_W,               GameKey::W },
    { SDL_SCANCODE_X,               GameKey::X },
    { SDL_SCANCODE_Y,               GameKey::Y },
    { SDL_SCANCODE_Z,               GameKey::Z },

    { SDL_SCANCODE_RETURN,          GameKey::Return },
    { SDL_SCANCODE_ESCAPE,          GameKey::Escape },
    { SDL_SCANCODE_TAB,             GameKey::Tab },
    { SDL_SCANCODE_BACKSPACE,       GameKey::Backspace },
    { SDL_SCANCODE_SPACE,           GameKey::Space },
    { SDL_SCANCODE_DECIMALSEPARATOR,GameKey::Decimal },
    { SDL_SCANCODE_SEMICOLON,       GameKey::Semicolon },
    { SDL_SCANCODE_PERIOD,          GameKey::Period },
    { SDL_SCANCODE_SLASH,           GameKey::Slash },
    { SDL_SCANCODE_APOSTROPHE,      GameKey::SingleQuote },
    { SDL_SCANCODE_BACKSLASH,       GameKey::BackSlash },

    { SDL_SCANCODE_KP_MINUS,        GameKey::Subtract },
    { SDL_SCANCODE_KP_PLUS,         GameKey::Add },
    { SDL_SCANCODE_COMMA,           GameKey::Comma },
    { SDL_SCANCODE_LEFTBRACKET,     GameKey::LeftBracket },
    { SDL_SCANCODE_RIGHTBRACKET,    GameKey::RightBracket },

    { SDL_SCANCODE_LEFT,            GameKey::Left },
    { SDL_SCANCODE_RIGHT,           GameKey::Right },
    { SDL_SCANCODE_UP,              GameKey::Up },
    { SDL_SCANCODE_DOWN,            GameKey::Down },

    { SDL_SCANCODE_PRINTSCREEN,     GameKey::PrintScreen },

    { SDL_SCANCODE_INSERT,          GameKey::Insert },
    { SDL_SCANCODE_HOME,            GameKey::Home },
    { SDL_SCANCODE_END,             GameKey::End },
    { SDL_SCANCODE_PAGEUP,          GameKey::PageUp },
    { SDL_SCANCODE_PAGEDOWN,        GameKey::PageDown },
    { SDL_SCANCODE_DELETE,          GameKey::Delete },
    { SDL_SCANCODE_SELECT,          GameKey::Select },

    { SDL_SCANCODE_LCTRL,           GameKey::Control },
    { SDL_SCANCODE_RCTRL,           GameKey::Control },
    { SDL_SCANCODE_LALT,            GameKey::Alt },
    { SDL_SCANCODE_RALT,            GameKey::Alt },
    { SDL_SCANCODE_LSHIFT,          GameKey::Shift },
    { SDL_SCANCODE_RSHIFT,          GameKey::Shift },

    { SDL_SCANCODE_KP_0,            GameKey::Numpad0 },
};



bool Sdl2Window::TryMapScanCode(SDL_Scancode code, GameKey *outKey) const {
    auto mappedKey = scancode_lookup.find(code);
    if (mappedKey != scancode_lookup.end()) {
        if (outKey != nullptr) {
            *outKey = mappedKey->second;
        }
        return true;
    }

    return false;
}


void Sdl2Window::OpenGlCreate() {
    //  Use OpenGL 3.3 core - requires all fixed pipeline code to be modernised
    //  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    //  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //  Turn on double buffering with a 24bit Z buffer.
    //  You may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    sdlOpenGlContext = SDL_GL_CreateContext(sdlWindow);
    if (!sdlOpenGlContext) {
        log->Warning("OpenGL failed: %s", SDL_GetError());
    }

    //  Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        log->Info("Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }
}

void Sdl2Window::OpenGlSwapBuffers() {
    SDL_GL_SwapWindow(sdlWindow);
}

