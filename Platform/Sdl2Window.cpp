#include <SDL_syswm.h>
#ifdef None
#undef None
#endif

#include <map>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Graphics/Nuklear.h"
#include "Platform/Sdl2Window.h"

#include "Io/GameKey.h"

#include "glad/glad.h"

using Io::GameKey;


std::map<SDL_Scancode, GameKey> scancode_lookup = {
    { SDL_SCANCODE_F1,               GameKey::F1 },
    { SDL_SCANCODE_F2,               GameKey::F2 },
    { SDL_SCANCODE_F3,               GameKey::F3 },
    { SDL_SCANCODE_F4,               GameKey::F4 },
    { SDL_SCANCODE_F5,               GameKey::F5 },
    { SDL_SCANCODE_F6,               GameKey::F6 },
    { SDL_SCANCODE_F7,               GameKey::F7 },
    { SDL_SCANCODE_F8,               GameKey::F8 },
    { SDL_SCANCODE_F9,               GameKey::F9 },
    { SDL_SCANCODE_F10,              GameKey::F10 },
    { SDL_SCANCODE_F11,              GameKey::F11 },
    { SDL_SCANCODE_F12,              GameKey::F12 },

    { SDL_SCANCODE_1,                GameKey::Digit1 },
    { SDL_SCANCODE_2,                GameKey::Digit2 },
    { SDL_SCANCODE_3,                GameKey::Digit3 },
    { SDL_SCANCODE_4,                GameKey::Digit4 },
    { SDL_SCANCODE_5,                GameKey::Digit5 },
    { SDL_SCANCODE_6,                GameKey::Digit6 },
    { SDL_SCANCODE_7,                GameKey::Digit7 },
    { SDL_SCANCODE_8,                GameKey::Digit8 },
    { SDL_SCANCODE_9,                GameKey::Digit9 },
    { SDL_SCANCODE_0,                GameKey::Digit0 },

    { SDL_SCANCODE_A,                GameKey::A },
    { SDL_SCANCODE_B,                GameKey::B },
    { SDL_SCANCODE_C,                GameKey::C },
    { SDL_SCANCODE_D,                GameKey::D },
    { SDL_SCANCODE_E,                GameKey::E },
    { SDL_SCANCODE_F,                GameKey::F },
    { SDL_SCANCODE_G,                GameKey::G },
    { SDL_SCANCODE_H,                GameKey::H },
    { SDL_SCANCODE_I,                GameKey::I },
    { SDL_SCANCODE_J,                GameKey::J },
    { SDL_SCANCODE_K,                GameKey::K },
    { SDL_SCANCODE_L,                GameKey::L },
    { SDL_SCANCODE_M,                GameKey::M },
    { SDL_SCANCODE_N,                GameKey::N },
    { SDL_SCANCODE_O,                GameKey::O },
    { SDL_SCANCODE_P,                GameKey::P },
    { SDL_SCANCODE_Q,                GameKey::Q },
    { SDL_SCANCODE_R,                GameKey::R },
    { SDL_SCANCODE_S,                GameKey::S },
    { SDL_SCANCODE_T,                GameKey::T },
    { SDL_SCANCODE_U,                GameKey::U },
    { SDL_SCANCODE_V,                GameKey::V },
    { SDL_SCANCODE_W,                GameKey::W },
    { SDL_SCANCODE_X,                GameKey::X },
    { SDL_SCANCODE_Y,                GameKey::Y },
    { SDL_SCANCODE_Z,                GameKey::Z },

    { SDL_SCANCODE_RETURN,           GameKey::Return },
    { SDL_SCANCODE_ESCAPE,           GameKey::Escape },
    { SDL_SCANCODE_TAB,              GameKey::Tab },
    { SDL_SCANCODE_BACKSPACE,        GameKey::Backspace },
    { SDL_SCANCODE_SPACE,            GameKey::Space },
    { SDL_SCANCODE_DECIMALSEPARATOR, GameKey::Decimal },
    { SDL_SCANCODE_SEMICOLON,        GameKey::Semicolon },
    { SDL_SCANCODE_PERIOD,           GameKey::Period },
    { SDL_SCANCODE_SLASH,            GameKey::Slash },
    { SDL_SCANCODE_APOSTROPHE,       GameKey::SingleQuote },
    { SDL_SCANCODE_BACKSLASH,        GameKey::BackSlash },
    { SDL_SCANCODE_GRAVE,            GameKey::Tilde },

    { SDL_SCANCODE_KP_MINUS,         GameKey::Subtract },
    { SDL_SCANCODE_KP_PLUS,          GameKey::Add },
    { SDL_SCANCODE_COMMA,            GameKey::Comma },
    { SDL_SCANCODE_LEFTBRACKET,      GameKey::LeftBracket },
    { SDL_SCANCODE_RIGHTBRACKET,     GameKey::RightBracket },

    { SDL_SCANCODE_LEFT,             GameKey::Left },
    { SDL_SCANCODE_RIGHT,            GameKey::Right },
    { SDL_SCANCODE_UP,               GameKey::Up },
    { SDL_SCANCODE_DOWN,             GameKey::Down },

    { SDL_SCANCODE_PRINTSCREEN,      GameKey::PrintScreen },

    { SDL_SCANCODE_INSERT,           GameKey::Insert },
    { SDL_SCANCODE_HOME,             GameKey::Home },
    { SDL_SCANCODE_END,              GameKey::End },
    { SDL_SCANCODE_PAGEUP,           GameKey::PageUp },
    { SDL_SCANCODE_PAGEDOWN,         GameKey::PageDown },
    { SDL_SCANCODE_DELETE,           GameKey::Delete },
    { SDL_SCANCODE_SELECT,           GameKey::Select },

    { SDL_SCANCODE_LCTRL,            GameKey::Control },
    { SDL_SCANCODE_RCTRL,            GameKey::Control },
    { SDL_SCANCODE_LALT,             GameKey::Alt },
    { SDL_SCANCODE_RALT,             GameKey::Alt },
    { SDL_SCANCODE_LSHIFT,           GameKey::Shift },
    { SDL_SCANCODE_RSHIFT,           GameKey::Shift },

    { SDL_SCANCODE_KP_0,             GameKey::Numpad0 },
    { SDL_SCANCODE_KP_1,             GameKey::Numpad1 },
    { SDL_SCANCODE_KP_2,             GameKey::Numpad2 },
    { SDL_SCANCODE_KP_3,             GameKey::Numpad3 },
    { SDL_SCANCODE_KP_4,             GameKey::Numpad4 },
    { SDL_SCANCODE_KP_5,             GameKey::Numpad5 },
    { SDL_SCANCODE_KP_6,             GameKey::Numpad6 },
    { SDL_SCANCODE_KP_7,             GameKey::Numpad7 },
    { SDL_SCANCODE_KP_8,             GameKey::Numpad8 },
    { SDL_SCANCODE_KP_9,             GameKey::Numpad9 },
};

const int GAME_KEYS_NUM = (int)GameKey::None;
bool scancode_pressed_state_single_report[256];
bool scancode_pressed_state[256];
int scancode_pressed_time[256];

int SDL_GetAsyncKeyState(GameKey key, bool consume) {
    if (key != GameKey::None && key != GameKey::Char) {
        auto mappedKey = std::find_if(
            scancode_lookup.begin(),
            scancode_lookup.end(),
            [key](const std::pair<SDL_Scancode, GameKey>& i) -> bool {
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

SDL_Window* Sdl2Window::CreateSDLWindow() {
    std::vector<SDL_Rect> displayBounds;
    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    int x = engine->config->window_x;
    int y = engine->config->window_y;
    int display = engine->config->display;
    int displays = SDL_GetNumVideoDisplays();
    if (displays < display)
        display = 0;

    for (int i = 0; i < displays; i++) {
        displayBounds.push_back(SDL_Rect());
        SDL_GetDisplayBounds(i, &displayBounds.back());
        log->Info("SDL2 display %d: x=%d, y=%d, w=%d, h=%d", i,
            displayBounds[i].x, displayBounds[i].y,
            displayBounds[i].w, displayBounds[i].h);
    }

    if (engine->config->fullscreen) {
        if (engine->config->borderless)
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        else
            flags |= SDL_WINDOW_FULLSCREEN;

        x = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
        y = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    }
    else {
        if (engine->config->borderless)
            flags |= SDL_WINDOW_BORDERLESS;

        if (x >= 0 && x < displayBounds[display].w)
            x += displayBounds[display].x;
        else
            x = SDL_WINDOWPOS_CENTERED_DISPLAY(display);

        if (y >= 0 && y < displayBounds[display].h)
            y += displayBounds[display].y;
        else
            y = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    }

    displayBounds.clear();

    sdlWindow = SDL_CreateWindow(
        engine->config->window_title.c_str(),
        x, y,
        engine->config->window_width, engine->config->window_height,
        flags
    );

    if (!sdlWindow)
        return nullptr;

    if (!engine->config->no_grab)
        SDL_SetWindowGrab(sdlWindow, SDL_TRUE);

    sdlWindowSurface = SDL_GetWindowSurface(sdlWindow);
    if (!sdlWindowSurface) {
        DestroySDLWindow();
        return nullptr;
    }

    return sdlWindow;
}

void Sdl2Window::DestroySDLWindow() {
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

        GameKey mappedKey;
        if (TryMapScanCode(e.key.keysym.scancode, &mappedKey)) {
            if (nuklear->KeyEvent(mappedKey))
                return 1;
        }

        if (sym == SDLK_BACKQUOTE && state[SDL_SCANCODE_LCTRL] && !down)
            nuklear->Reload();
        else if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
            nk_input_key(nuklear->ctx, NK_KEY_SHIFT, down);
        else if (sym == SDLK_DELETE)
            nk_input_key(nuklear->ctx, NK_KEY_DEL, down);
        else if (sym == SDLK_RETURN)
            nk_input_key(nuklear->ctx, NK_KEY_ENTER, down);
        else if (sym == SDLK_TAB)
            nk_input_key(nuklear->ctx, NK_KEY_TAB, down);
        else if (sym == SDLK_BACKSPACE)
            nk_input_key(nuklear->ctx, NK_KEY_BACKSPACE, down);
        else if (sym == SDLK_HOME) {
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_START, down);
            nk_input_key(nuklear->ctx, NK_KEY_SCROLL_START, down);
        } else if (sym == SDLK_END) {
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_END, down);
            nk_input_key(nuklear->ctx, NK_KEY_SCROLL_END, down);
        } else if (sym == SDLK_PAGEDOWN) {
            nk_input_key(nuklear->ctx, NK_KEY_SCROLL_DOWN, down);
        } else if (sym == SDLK_PAGEUP) {
            nk_input_key(nuklear->ctx, NK_KEY_SCROLL_UP, down);
        } else if (sym == SDLK_z && state[SDL_SCANCODE_LCTRL])
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_UNDO, down);
        else if (sym == SDLK_r && state[SDL_SCANCODE_LCTRL])
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_REDO, down);
        else if (sym == SDLK_c && state[SDL_SCANCODE_LCTRL])
            nk_input_key(nuklear->ctx, NK_KEY_COPY, down);
        else if (sym == SDLK_v && state[SDL_SCANCODE_LCTRL])
            nk_input_key(nuklear->ctx, NK_KEY_PASTE, down);
        else if (sym == SDLK_x && state[SDL_SCANCODE_LCTRL])
            nk_input_key(nuklear->ctx, NK_KEY_CUT, down);
        else if (sym == SDLK_b && state[SDL_SCANCODE_LCTRL])
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_LINE_START, down);
        else if (sym == SDLK_e && state[SDL_SCANCODE_LCTRL])
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_LINE_END, down);
        else if (sym == SDLK_UP)
            nk_input_key(nuklear->ctx, NK_KEY_UP, down);
        else if (sym == SDLK_DOWN)
            nk_input_key(nuklear->ctx, NK_KEY_DOWN, down);
        else if (sym == SDLK_LEFT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(nuklear->ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else nk_input_key(nuklear->ctx, NK_KEY_LEFT, down);
        } else if (sym == SDLK_RIGHT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(nuklear->ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else nk_input_key(nuklear->ctx, NK_KEY_RIGHT, down);
        } else
            return 0;
        return 1;
    } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        /* mouse button */
        int down = e.type == SDL_MOUSEBUTTONDOWN;
        const int x = e.button.x, y = e.button.y;
        if (e.button.button == SDL_BUTTON_LEFT) {
            if (e.button.clicks > 1)
                nk_input_button(nuklear->ctx, NK_BUTTON_DOUBLE, x, y, down);
            nk_input_button(nuklear->ctx, NK_BUTTON_LEFT, x, y, down);
        } else if (e.button.button == SDL_BUTTON_MIDDLE)
            nk_input_button(nuklear->ctx, NK_BUTTON_MIDDLE, x, y, down);
        else if (e.button.button == SDL_BUTTON_RIGHT)
            nk_input_button(nuklear->ctx, NK_BUTTON_RIGHT, x, y, down);
        return 1;
    } else if (e.type == SDL_MOUSEMOTION) {
        /* mouse motion */
        if (nuklear->ctx->input.mouse.grabbed) {
            int x = (int)nuklear->ctx->input.mouse.prev.x, y = (int)nuklear->ctx->input.mouse.prev.y;
            nk_input_motion(nuklear->ctx, x + e.motion.xrel, y + e.motion.yrel);
        } else nk_input_motion(nuklear->ctx, e.motion.x, e.motion.y);
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

            GameKey mappedKey;
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
SDL_Surface* Sdl2Window::getSDLWindowSurface() {
    return sdlWindowSurface;
}

void Sdl2Window::SetFullscreenMode() {
    Uint32 flags;

    if (engine->config->borderless)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    else
        flags |= SDL_WINDOW_FULLSCREEN;

    SDL_SetWindowFullscreen(sdlWindow, flags);
}

void Sdl2Window::SetWindowedMode(int new_window_width, int new_window_height) {
    if (!SDL_SetWindowDisplayMode(sdlWindow, 0))
        SDL_SetWindowSize(sdlWindow, new_window_height, new_window_height);
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    //  Turn on double buffering with a 24bit Z buffer.
    //  You may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    sdlOpenGlContext = SDL_GL_CreateContext(sdlWindow);
    if (!sdlOpenGlContext) {
        log->Warning("Failed to initialize SDL with OpenGL: %s", SDL_GetError());
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        log->Warning("Failed to initialize the OpenGL loader");
    }

    log->Info("Supported OpenGL: %s", glGetString(GL_VERSION));
    log->Info("Supported GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    log->Info("OpenGL version: %d.%d", GLVersion.major, GLVersion.minor);

    //  Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        log->Info("Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }
}

void Sdl2Window::OpenGlSwapBuffers() {
    SDL_GL_SwapWindow(sdlWindow);
}
