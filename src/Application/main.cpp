#include <cstdio>
#include <string>
#include <exception>

#include <CLI/Error.hpp>

#include "Application/Game.h"
#include "Application/GameConfig.h"
#include "Application/GameFactory.h"
#include "Application/GameOptions.h"

#include "Library/Application/PlatformApplication.h"

#include "Platform/Platform.h"

#include "Utility/ScopeGuard.h"

int MM_Main(int argc, char **argv) {
    try {
        GameOptions options = GameOptions::Parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        std::unique_ptr<PlatformLogger> logger = PlatformLogger::createStandardLogger(WIN_ENSURE_CONSOLE_OPTION);
        auto setVerboseLogging = [logger = logger.get()](bool verbose) {
            logger->setLogLevel(APPLICATION_LOG, verbose ? LOG_VERBOSE : LOG_INFO);
            logger->setLogLevel(PLATFORM_LOG, verbose ? LOG_VERBOSE : LOG_ERROR);
        };
        setVerboseLogging(options.verbose);
        EngineIocContainer::ResolveLogger()->setBaseLogger(logger.get());
        MM_AT_SCOPE_EXIT(EngineIocContainer::ResolveLogger()->setBaseLogger(nullptr));
        Engine::LogEngineBuildInfo();

        std::unique_ptr<PlatformApplication> app = std::make_unique<PlatformApplication>(logger.get());
        options.ResolveDefaults(app->platform());
        initDataPath(options.dataPath);

        std::shared_ptr<GameConfig> gameConfig = std::make_shared<GameConfig>(options.configPath);
        gameConfig->debug.VerboseLogging.subscribe([&](bool value) {
            setVerboseLogging(value || options.verbose);
        });
        gameConfig->LoadConfiguration();

        std::shared_ptr<Game> game = GameFactory().CreateGame(app.get(), gameConfig);
        return game->Run();
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}

int platformMain(int argc, char **argv) {
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
