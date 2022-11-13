#include <SDL_syswm.h>
#ifdef None
#undef None
#endif

#include <map>
#include <utility>
#include <vector>
#include <string>

#include "Engine/Graphics/Nuklear.h"
#include "Platform/Sdl2Window.h"

#include "PlatformKey.h"

#include "glad/gl.h"


std::map<SDL_Scancode, PlatformKey> scancode_lookup = {
    { SDL_SCANCODE_F1,               PlatformKey::F1 },
    { SDL_SCANCODE_F2,               PlatformKey::F2 },
    { SDL_SCANCODE_F3,               PlatformKey::F3 },
    { SDL_SCANCODE_F4,               PlatformKey::F4 },
    { SDL_SCANCODE_F5,               PlatformKey::F5 },
    { SDL_SCANCODE_F6,               PlatformKey::F6 },
    { SDL_SCANCODE_F7,               PlatformKey::F7 },
    { SDL_SCANCODE_F8,               PlatformKey::F8 },
    { SDL_SCANCODE_F9,               PlatformKey::F9 },
    { SDL_SCANCODE_F10,              PlatformKey::F10 },
    { SDL_SCANCODE_F11,              PlatformKey::F11 },
    { SDL_SCANCODE_F12,              PlatformKey::F12 },

    { SDL_SCANCODE_1,                PlatformKey::Digit1 },
    { SDL_SCANCODE_2,                PlatformKey::Digit2 },
    { SDL_SCANCODE_3,                PlatformKey::Digit3 },
    { SDL_SCANCODE_4,                PlatformKey::Digit4 },
    { SDL_SCANCODE_5,                PlatformKey::Digit5 },
    { SDL_SCANCODE_6,                PlatformKey::Digit6 },
    { SDL_SCANCODE_7,                PlatformKey::Digit7 },
    { SDL_SCANCODE_8,                PlatformKey::Digit8 },
    { SDL_SCANCODE_9,                PlatformKey::Digit9 },
    { SDL_SCANCODE_0,                PlatformKey::Digit0 },

    { SDL_SCANCODE_A,                PlatformKey::A },
    { SDL_SCANCODE_B,                PlatformKey::B },
    { SDL_SCANCODE_C,                PlatformKey::C },
    { SDL_SCANCODE_D,                PlatformKey::D },
    { SDL_SCANCODE_E,                PlatformKey::E },
    { SDL_SCANCODE_F,                PlatformKey::F },
    { SDL_SCANCODE_G,                PlatformKey::G },
    { SDL_SCANCODE_H,                PlatformKey::H },
    { SDL_SCANCODE_I,                PlatformKey::I },
    { SDL_SCANCODE_J,                PlatformKey::J },
    { SDL_SCANCODE_K,                PlatformKey::K },
    { SDL_SCANCODE_L,                PlatformKey::L },
    { SDL_SCANCODE_M,                PlatformKey::M },
    { SDL_SCANCODE_N,                PlatformKey::N },
    { SDL_SCANCODE_O,                PlatformKey::O },
    { SDL_SCANCODE_P,                PlatformKey::P },
    { SDL_SCANCODE_Q,                PlatformKey::Q },
    { SDL_SCANCODE_R,                PlatformKey::R },
    { SDL_SCANCODE_S,                PlatformKey::S },
    { SDL_SCANCODE_T,                PlatformKey::T },
    { SDL_SCANCODE_U,                PlatformKey::U },
    { SDL_SCANCODE_V,                PlatformKey::V },
    { SDL_SCANCODE_W,                PlatformKey::W },
    { SDL_SCANCODE_X,                PlatformKey::X },
    { SDL_SCANCODE_Y,                PlatformKey::Y },
    { SDL_SCANCODE_Z,                PlatformKey::Z },

    { SDL_SCANCODE_RETURN,           PlatformKey::Return },
    { SDL_SCANCODE_ESCAPE,           PlatformKey::Escape },
    { SDL_SCANCODE_TAB,              PlatformKey::Tab },
    { SDL_SCANCODE_BACKSPACE,        PlatformKey::Backspace },
    { SDL_SCANCODE_SPACE,            PlatformKey::Space },
    { SDL_SCANCODE_DECIMALSEPARATOR, PlatformKey::Decimal },
    { SDL_SCANCODE_SEMICOLON,        PlatformKey::Semicolon },
    { SDL_SCANCODE_PERIOD,           PlatformKey::Period },
    { SDL_SCANCODE_SLASH,            PlatformKey::Slash },
    { SDL_SCANCODE_APOSTROPHE,       PlatformKey::SingleQuote },
    { SDL_SCANCODE_BACKSLASH,        PlatformKey::BackSlash },
    { SDL_SCANCODE_GRAVE,            PlatformKey::Tilde },

    { SDL_SCANCODE_KP_MINUS,         PlatformKey::Subtract },
    { SDL_SCANCODE_KP_PLUS,          PlatformKey::Add },
    { SDL_SCANCODE_COMMA,            PlatformKey::Comma },
    { SDL_SCANCODE_LEFTBRACKET,      PlatformKey::LeftBracket },
    { SDL_SCANCODE_RIGHTBRACKET,     PlatformKey::RightBracket },

    { SDL_SCANCODE_LEFT,             PlatformKey::Left },
    { SDL_SCANCODE_RIGHT,            PlatformKey::Right },
    { SDL_SCANCODE_UP,               PlatformKey::Up },
    { SDL_SCANCODE_DOWN,             PlatformKey::Down },

    { SDL_SCANCODE_PRINTSCREEN,      PlatformKey::PrintScreen },

    { SDL_SCANCODE_INSERT,           PlatformKey::Insert },
    { SDL_SCANCODE_HOME,             PlatformKey::Home },
    { SDL_SCANCODE_END,              PlatformKey::End },
    { SDL_SCANCODE_PAGEUP,           PlatformKey::PageUp },
    { SDL_SCANCODE_PAGEDOWN,         PlatformKey::PageDown },
    { SDL_SCANCODE_DELETE,           PlatformKey::Delete },
    { SDL_SCANCODE_SELECT,           PlatformKey::Select },

    { SDL_SCANCODE_LCTRL,            PlatformKey::Control },
    { SDL_SCANCODE_RCTRL,            PlatformKey::Control },
    { SDL_SCANCODE_LALT,             PlatformKey::Alt },
    { SDL_SCANCODE_RALT,             PlatformKey::Alt },
    { SDL_SCANCODE_LSHIFT,           PlatformKey::Shift },
    { SDL_SCANCODE_RSHIFT,           PlatformKey::Shift },

    { SDL_SCANCODE_KP_0,             PlatformKey::Numpad0 },
    { SDL_SCANCODE_KP_1,             PlatformKey::Numpad1 },
    { SDL_SCANCODE_KP_2,             PlatformKey::Numpad2 },
    { SDL_SCANCODE_KP_3,             PlatformKey::Numpad3 },
    { SDL_SCANCODE_KP_4,             PlatformKey::Numpad4 },
    { SDL_SCANCODE_KP_5,             PlatformKey::Numpad5 },
    { SDL_SCANCODE_KP_6,             PlatformKey::Numpad6 },
    { SDL_SCANCODE_KP_7,             PlatformKey::Numpad7 },
    { SDL_SCANCODE_KP_8,             PlatformKey::Numpad8 },
    { SDL_SCANCODE_KP_9,             PlatformKey::Numpad9 },
};

const int GAME_KEYS_NUM = (int)PlatformKey::None;
bool scancode_pressed_state_single_report[256];
bool scancode_pressed_state[256];
int scancode_pressed_time[256];

int SDL_GetAsyncKeyState(PlatformKey key, bool consume) {
    if (key != PlatformKey::None && key != PlatformKey::Char) {
        auto mappedKey = std::find_if(
            scancode_lookup.begin(),
            scancode_lookup.end(),
            [key](const std::pair<SDL_Scancode, PlatformKey>& i) -> bool {
                return key == i.second;
            }
        );

        if (mappedKey != scancode_lookup.end()) {
            int scancode = mappedKey->first;
            if (scancode_pressed_state[scancode]) {
                if (SDL_GetTicks() - scancode_pressed_time[scancode] > 100) {
                    return 2;
                }

                if (!consume) {
                    return 1;
                }

                // some logic relies on fact that querying a key state should reset it
                // setting consume = true allows to imitate this, returning true only once per press/release pair

                if (scancode_pressed_state_single_report[scancode]) {
                    scancode_pressed_state_single_report[scancode] = false;
                    return 1;
                }

                // dont report pressed key twice unless its held
                return 0;
            }
        }
    }

    return 0;
}

Sdl2Window::Sdl2WinParams Sdl2Window::CalculateWindowParameters() {
    std::vector<SDL_Rect> displayBounds;
    Sdl2Window::Sdl2WinParams params;

    params.flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    params.x = config->window.PositionX.Get();
    params.y = config->window.PositionY.Get();
    params.display = config->window.Display.Get();
    int displays = SDL_GetNumVideoDisplays();
    if (params.display > displays - 1)
        params.display = 0;

    for (int i = 0; i < displays; i++) {
        displayBounds.push_back(SDL_Rect());
        SDL_GetDisplayBounds(i, &displayBounds.back());
        log->Info("SDL2: display %d, x=%d, y=%d, w=%d, h=%d", i,
            displayBounds[i].x, displayBounds[i].y,
            displayBounds[i].w, displayBounds[i].h);
    }

    if (config->window.Fullscreen.Get()) {
        if (config->window.Borderless.Get())
            params.flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        else
            params.flags |= SDL_WINDOW_FULLSCREEN;

        params.x = SDL_WINDOWPOS_CENTERED_DISPLAY(params.display);
        params.y = SDL_WINDOWPOS_CENTERED_DISPLAY(params.display);
    } else {
        if (config->window.Borderless.Get())
            params.flags |= SDL_WINDOW_BORDERLESS;

        if (params.x >= 0 && params.x < displayBounds[params.display].w)
            params.x += displayBounds[params.display].x;
        else
            params.x = SDL_WINDOWPOS_CENTERED_DISPLAY(params.display);

        if (params.y >= 0 && params.y < displayBounds[params.display].h)
            params.y += displayBounds[params.display].y;
        else
            params.y = SDL_WINDOWPOS_CENTERED_DISPLAY(params.display);
    }

    displayBounds.clear();

    return params;
}

SDL_Window* Sdl2Window::CreateSDLWindow() {
    Sdl2Window::Sdl2WinParams params = CalculateWindowParameters();

    sdlWindow = SDL_CreateWindow(
        config->window.Title.Get().c_str(),
        params.x, params.y,
        config->window.Width.Get(), config->window.Height.Get(),
        params.flags
    );

    config->window.Display.Set(params.display);

    if (!sdlWindow) {
        return nullptr;
    }

    if (config->window.MouseGrab.Get()) {
        SDL_SetWindowGrab(sdlWindow, SDL_TRUE);
    }

    return sdlWindow;
}

void Sdl2Window::Release() {
    if (sdlWindow) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = nullptr;
    }
}

bool Sdl2Window::NuklearEventHandler(const SDL_Event &e) {
    if (e.type == SDL_KEYUP || e.type == SDL_KEYDOWN) {
        /* key events */
        int down = e.type == SDL_KEYDOWN;
        const Uint8 *state = SDL_GetKeyboardState(0);
        SDL_Keycode sym = e.key.keysym.sym;

        PlatformKey mappedKey;
        if (TryMapScanCode(e.key.keysym.scancode, &mappedKey)) {
            if (nuklear->KeyEvent(mappedKey))
                return 1;
        }

        if (sym == SDLK_BACKQUOTE && state[SDL_SCANCODE_LCTRL] && !down) {
            nuklear->Reload();
        } else if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT) {
            nk_input_key(nuklear->ctx, NK_KEY_SHIFT, down);
        } else if (sym == SDLK_DELETE) {
            nk_input_key(nuklear->ctx, NK_KEY_DEL, down);
        } else if (sym == SDLK_RETURN) {
            nk_input_key(nuklear->ctx, NK_KEY_ENTER, down);
        } else if (sym == SDLK_TAB) {
            nk_input_key(nuklear->ctx, NK_KEY_TAB, down);
        } else if (sym == SDLK_BACKSPACE) {
            nk_input_key(nuklear->ctx, NK_KEY_BACKSPACE, down);
        } else if (sym == SDLK_HOME) {
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_START, down);
            nk_input_key(nuklear->ctx, NK_KEY_SCROLL_START, down);
        } else if (sym == SDLK_END) {
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_END, down);
            nk_input_key(nuklear->ctx, NK_KEY_SCROLL_END, down);
        } else if (sym == SDLK_PAGEDOWN) {
            nk_input_key(nuklear->ctx, NK_KEY_SCROLL_DOWN, down);
        } else if (sym == SDLK_PAGEUP) {
            nk_input_key(nuklear->ctx, NK_KEY_SCROLL_UP, down);
        } else if (sym == SDLK_z && state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_UNDO, down);
        } else if (sym == SDLK_r && state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_REDO, down);
        } else if (sym == SDLK_c && state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(nuklear->ctx, NK_KEY_COPY, down);
        } else if (sym == SDLK_v && state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(nuklear->ctx, NK_KEY_PASTE, down);
        } else if (sym == SDLK_x && state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(nuklear->ctx, NK_KEY_CUT, down);
        } else if (sym == SDLK_b && state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_LINE_START, down);
        } else if (sym == SDLK_e && state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_LINE_END, down);
        } else if (sym == SDLK_UP) {
            nk_input_key(nuklear->ctx, NK_KEY_UP, down);
        } else if (sym == SDLK_DOWN) {
            nk_input_key(nuklear->ctx, NK_KEY_DOWN, down);
        } else if (sym == SDLK_LEFT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(nuklear->ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else
                nk_input_key(nuklear->ctx, NK_KEY_LEFT, down);
        } else if (sym == SDLK_RIGHT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(nuklear->ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else
                nk_input_key(nuklear->ctx, NK_KEY_RIGHT, down);
        } else {
            return 0;
        }

        return 1;
    } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        /* mouse button */
        int down = e.type == SDL_MOUSEBUTTONDOWN;
        const int x = e.button.x, y = e.button.y;
        if (e.button.button == SDL_BUTTON_LEFT) {
            if (e.button.clicks > 1)
                nk_input_button(nuklear->ctx, NK_BUTTON_DOUBLE, x, y, down);

            nk_input_button(nuklear->ctx, NK_BUTTON_LEFT, x, y, down);
        } else if (e.button.button == SDL_BUTTON_MIDDLE) {
            nk_input_button(nuklear->ctx, NK_BUTTON_MIDDLE, x, y, down);
        } else if (e.button.button == SDL_BUTTON_RIGHT) {
            nk_input_button(nuklear->ctx, NK_BUTTON_RIGHT, x, y, down);
        }

        return 1;
    } else if (e.type == SDL_MOUSEMOTION) {
        /* mouse motion */
        if (nuklear->ctx->input.mouse.grabbed) {
            int x = (int)nuklear->ctx->input.mouse.prev.x, y = (int)nuklear->ctx->input.mouse.prev.y;
            nk_input_motion(nuklear->ctx, x + e.motion.xrel, y + e.motion.yrel);
        } else {
            nk_input_motion(nuklear->ctx, e.motion.x, e.motion.y);
        }

        return 1;
    } else if (e.type == SDL_TEXTINPUT) {
        /* text input */
        nk_glyph glyph;
        memcpy(glyph, e.text.text, NK_UTF_SIZE);
        nk_input_glyph(nuklear->ctx, glyph);

        return 1;
    } else if (e.type == SDL_MOUSEWHEEL) {
        /* mouse wheel */
        nk_input_scroll(nuklear->ctx, nk_vec2((float)e.wheel.x, (float)e.wheel.y));

        return 1;
    }

    return 0;
}

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
            if (e.key.keysym.scancode < 256) {
                scancode_pressed_state[e.key.keysym.scancode] = false;
                scancode_pressed_time[e.key.keysym.scancode] = 0;
            }
        } break;

        case SDL_KEYDOWN: {
            if (e.key.repeat > 0) {
                break;
            }

            PlatformKey mappedKey;
            if (TryMapScanCode(e.key.keysym.scancode, &mappedKey)) {
                // send key to textinput/controls binding handler
                gameCallback->OnChar(mappedKey, -1);
                // send key to other game systems
                gameCallback->OnKey(mappedKey);
            }

            int mappedChar;
            auto mods = e.key.keysym.mod;
            if (TryMapKeyCode(e.key.keysym.sym, (mods & KMOD_CAPS) != (mods & KMOD_SHIFT), &mappedChar)) {
                gameCallback->OnChar(mappedKey, mappedChar);
            }

            if (e.key.keysym.scancode < 256) {
                scancode_pressed_state[e.key.keysym.scancode] = true;
                scancode_pressed_state_single_report[e.key.keysym.scancode] = true;
                scancode_pressed_time[e.key.keysym.scancode] = SDL_GetTicks();
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
                case SDL_WINDOWEVENT_EXPOSED:
                    gameCallback->OnPaint();
                    break;

                case SDL_WINDOWEVENT_FOCUS_LOST:
                    gameCallback->OnDeactivated();
                    break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    gameCallback->OnActivated();
                    break;

                case SDL_WINDOWEVENT_MOVED:
                    SaveWindowPosition();
                    break;

                default:
                    break;
            }
        } break;
    }
}

void Sdl2Window::HandleAllEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (nuklear && nuklear->ctx) {
            NuklearEventHandler(e);
        }

        MessageProc(e);
    }
}

void Sdl2Window::WaitSingleEvent() {
    SDL_WaitEvent(nullptr);
}

void Sdl2Window::HandleSingleEvent() {
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

SDL_GLContext* Sdl2Window::getSDLOpenGlContext() {
    return &sdlOpenGlContext;
}

SDL_Window* Sdl2Window::getSDLWindow() {
    return sdlWindow;
}

void Sdl2Window::SetFullscreenMode() {
    Uint32 flags;

    if (config->window.Borderless.Get())
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    else
        flags = SDL_WINDOW_FULLSCREEN;

    if (SDL_SetWindowFullscreen(sdlWindow, flags) < 0)
        log->Warning("SDL2: unable to enter fullscreen mode: %s", SDL_GetError());
}

void Sdl2Window::SetWindowedMode(int new_window_width, int new_window_height) {
    if (SDL_SetWindowFullscreen(sdlWindow, 0) < 0) {
        log->Warning("SDL2: unable to leave fullscreen mode: %s", SDL_GetError());
        return;
    }

    Sdl2WinParams params = CalculateWindowParameters();

    SDL_SetWindowSize(sdlWindow, new_window_width, new_window_height);
    SDL_SetWindowPosition(sdlWindow, params.x, params.y);

    config->window.Display.Set(params.display);
    config->window.Width.Set(new_window_width);
    config->window.Height.Set(new_window_height);
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

void Sdl2Window::SetWindowArea(int width, int height) {
    SDL_SetWindowSize(sdlWindow, width, height);
}

void Sdl2Window::SaveWindowPosition() {
    std::vector<SDL_Rect> displayBounds;
    int x, y;

    SDL_GetWindowPosition(sdlWindow, &x, &y);

    int display = SDL_GetWindowDisplayIndex(sdlWindow);
    config->window.Display.Set(display);;

    if (!config->window.Fullscreen.Get()) {
        int displays = SDL_GetNumVideoDisplays();
        for (int i = 0; i < displays; i++) {
            displayBounds.push_back(SDL_Rect());
            SDL_GetDisplayBounds(i, &displayBounds.back());
        }

        config->window.PositionX.Set(x - displayBounds[display].x);
        config->window.PositionY.Set(y - displayBounds[display].y);
    }
}

bool Sdl2Window::OnOSMenu(int item_id) {
    return false;
}

void Sdl2Window::Show() {
    SDL_ShowWindow(sdlWindow);
    SDL_RaiseWindow(sdlWindow);

    HandleAllEvents();
}

bool Sdl2Window::Focused() {
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
    return (SDL_GetWindowFlags(sdlWindow) & flags) == flags;
}

void Sdl2Window::Activate() {
    SDL_RaiseWindow(sdlWindow);

    HandleAllEvents();
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
            } else {
                *outKey = 'a' + (key - SDLK_a);
            }
        }
        return true;
    }
    return false;
}

bool Sdl2Window::TryMapScanCode(SDL_Scancode code, PlatformKey *outKey) const {
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
    int version;

    //  Use OpenGL 4.1 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //  Turn on double buffering with a 24bit Z buffer.
    //  You may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    sdlOpenGlContext = SDL_GL_CreateContext(sdlWindow);
    if (!sdlOpenGlContext) {
        log->Warning("SDL2: failed to initialize SDL with OpenGL: %s", SDL_GetError());
    }

    if (!(version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))) {
        log->Warning("SDL2: failed to initialize the OpenGL loader");
    }

    log->Info("SDL2: supported OpenGL: %s", glGetString(GL_VERSION));
    log->Info("SDL2: supported GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    log->Info("SDL2: OpenGL version: %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // Use Vsync
    if (SDL_GL_SetSwapInterval(config->graphics.VSync.Get() ? 1 : 0) < 0) {
        log->Info("SDL2: unable to set VSync: %s\n", SDL_GetError());
    }
}

void Sdl2Window::OpenGlSwapBuffers() {
    SDL_GL_SwapWindow(sdlWindow);
}
