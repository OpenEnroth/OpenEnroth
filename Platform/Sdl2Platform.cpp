#include <SDL.h>

void OS_MsgBox(const char *msg, const char *title) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, nullptr);
}
