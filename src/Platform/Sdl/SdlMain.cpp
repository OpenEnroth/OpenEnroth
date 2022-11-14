#include <SDL.h>

int PlatformMain(int argc, char** argv);

// This gets #defined by SDL into SDL_main.
int main(int argc, char** argv) {
    return PlatformMain(argc, argv);
}
