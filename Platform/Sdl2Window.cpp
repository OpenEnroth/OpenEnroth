#include "Platform/Sdl2Window.h"

#include "SDL_syswm.h"

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
            auto key = e.key.keysym.sym;
            auto mods = e.key.keysym.mod;

            auto ch = SdlkToChar(key, (mods & KMOD_CAPS) != (mods & KMOD_SHIFT));
            auto vkkey = SdlkToVk(key);
            if (ch != -1) {
                gameCallback->OnChar(ch);
            } else if ( key == SDLK_RETURN || key == SDLK_ESCAPE || key == SDLK_TAB || key == SDLK_BACKSPACE ) {
                if (!gameCallback->OnChar(vkkey))
                    gameCallback->OnVkDown(vkkey, 0);
            } else {
                gameCallback->OnVkDown(vkkey, 0);
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


int Sdl2Window::SdlkToChar(SDL_Keycode key, bool uppercase) const {
    if (key >= SDLK_0 && key <= SDLK_9) {
        return '0' + (key - SDLK_0);
    }
    if (key >= SDLK_a && key <= SDLK_z) {
        if (uppercase) {
            return 'A' + (key - SDLK_a);
        } else {
            return 'a' + (key - SDLK_a);
        }
    }
    return -1;
}

int Sdl2Window::SdlkToVk(SDL_Keycode key) const {
#ifdef _WINDOWS
    // "Will be removed later"
    if (key >= SDLK_F1 && key <= SDLK_F12) {
        return VK_F1 + (key - SDLK_F1);
    }

    static struct {
        SDL_Keycode sdlk;
        int vk;
    }
    sdlk2vk[] = {
        { SDLK_LEFT, VK_LEFT },
        { SDLK_RIGHT, VK_RIGHT },
        { SDLK_UP, VK_UP },
        { SDLK_DOWN, VK_DOWN },
        { SDLK_PRINTSCREEN, VK_PRINT },
        { SDLK_INSERT, VK_INSERT },
        { SDLK_HOME, VK_HOME },
        { SDLK_PAGEUP, VK_PRIOR },
        { SDLK_DELETE, VK_DELETE },
        { SDLK_END, VK_END },
        { SDLK_PAGEDOWN, VK_NEXT },
    };

    for (int i = 0; i < sizeof(sdlk2vk) / sizeof(*sdlk2vk); ++i) {
        if (sdlk2vk[i].sdlk == key) {
            return sdlk2vk[i].vk;
        }
    }
#endif

    return key & 0xFFFF;
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

