#include <SDL.h>

int platformMain(int argc, char **argv);

// This gets #defined by SDL into SDL_main. Marking it as extern "C" is important for name mangling as on Android
// it gets exported from the resulting .so.
extern "C" int main(int argc, char **argv) {
    return platformMain(argc, argv);
}
