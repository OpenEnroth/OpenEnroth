#include <cstdio>

#include "Utility/Format.h"

#include "GameStarter.h"
#include "GameOptions.h"

int openEnrothMain(int argc, char **argv) {
    try {
        GameOptions options = GameOptions::Parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        GameStarter starter(options);
        starter.run();
        return 0;
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}

int platformMain(int argc, char **argv) {
    int result = openEnrothMain(argc, argv);

#ifdef _WINDOWS
    // SDL on Windows creates a separate console window, and we want to be able to actually read the error message
    // before that window closes.
    if (result != 0) {
        printf("[Press any key to close this window]");
        getchar();
    }
#elif __ANDROID__
    // TODO: on android without this it won't close application properly until it finishes music track?!
    // Something is not closing and preventing proper teardown?
    exit(0);
#endif

    return result;
}
