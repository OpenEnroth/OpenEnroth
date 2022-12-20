#include <cstdio>
#include <string>
#include <exception>

#include "Application/Game.h"
#include "Application/GameConfig.h"
#include "Application/GameFactory.h"

#include "Platform/Platform.h"

// TODO(captainurist): we don't need these namespaces
using Application::Game;
using Application::GameConfig;
using Application::GameFactory;

int MM_Main(int argc, char **argv) {
    try {
        Log *log = EngineIoc::ResolveLogger();

        std::unique_ptr<Platform> platform = Platform::CreateStandardPlatform(log);
        platform->WinEnsureConsole();

        std::string cmd;
        for (int i = 1; i < argc; ++i)
            cmd += std::string(argv[i]) + " ";

        std::shared_ptr<GameConfig> gameConfig = std::make_shared<GameConfig>(cmd);
        gameConfig->Startup();
        std::shared_ptr<Game> game = GameFactory().CreateGame(platform.get(), gameConfig);

        return game->Run();
    } catch (const std::exception &e) {
        fprintf(stderr, "%s", e.what());
        return 1;
    }
}

int PlatformMain(int argc, char** argv) {
    int result = MM_Main(argc, argv);

#ifdef _WINDOWS
    // SDL on Windows creates a separate console window, and we want to be able to actually read the error message
    // before that window closes.
    if (result != 0) {
        printf("[Press any key to close this window]");
        getchar();
    }
#endif

    return result;
}
