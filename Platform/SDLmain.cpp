#include <string>

#include <SDL.h>


int SDL_main(int argc, char *argv[]) {
    std::string cmd;
    for (int i = 1; i < argc; ++i) {
        cmd += std::string(argv[i]) + " ";
    }

    int res = -1;
    if (SDL_Init(SDL_INIT_VIDEO) >= 0) {
        extern int MM_Main(const std::string &);
        res = MM_Main(cmd);

        SDL_Quit();
    }

    return res;
}
