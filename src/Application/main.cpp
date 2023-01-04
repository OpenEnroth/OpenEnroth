#include <cstdio>
#include <string>
#include <exception>

#include "Application/Game.h"
#include "Application/GameConfig.h"
#include "Application/GameFactory.h"
#include "Application/GameOptions.h"

#include "Platform/Platform.h"

#include "Utility/ScopeGuard.h"

// TODO(captainurist): we don't need these namespaces
using Application::Game;
using Application::GameConfig;
using Application::GameFactory;

int MM_Main(int argc, char **argv) {
    try {
        std::unique_ptr<PlatformLogger> logger = PlatformLogger::CreateStandardLogger(WinEnsureConsoleOption);
        logger->SetLogLevel(ApplicationLog, LogInfo);
        logger->SetLogLevel(PlatformLog, LogError);
        EngineIoc::ResolveLogger()->SetBaseLogger(logger.get());
        auto guard = ScopeGuard([] { EngineIoc::ResolveLogger()->SetBaseLogger(nullptr); });
        Engine::LogEngineBuildInfo();

        std::unique_ptr<Platform> platform = Platform::CreateStandardPlatform(logger.get());

        Application::AutoInitDataPath(platform.get());

        std::shared_ptr<GameConfig> gameConfig = std::make_shared<GameConfig>();
        gameConfig->LoadConfiguration();
        if (!Application::ParseGameOptions(argc, argv, &*gameConfig))
            return 1;

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
#elif __ANDROID__
    // TODO: on android without this it won't close application properly until it finishes music track?!
    // Something is not closing and preventing proper teardown?
    exit(0);
#endif

    return result;
}
