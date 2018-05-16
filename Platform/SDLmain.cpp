#include "lib/SDL2/include/SDL.h"

#include <string>

int SDL_main(int argc, char *argv[]) {
    std::string cmd;
    for (int i = 1; i < argc; ++i) {
        cmd += std::string(argv[i]) + " ";
    }

    int res = -1;
    if (SDL_Init(SDL_INIT_VIDEO) >= 0) {
        extern int MM_Main(const char *);
        res = MM_Main(cmd.c_str());

        SDL_Quit();
    }

    return res;
}
