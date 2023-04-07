#include "GameStarter.h"

#include <utility>

#include "Engine/EngineIocContainer.h"
#include "Engine/Engine.h"

#include "Library/Application/PlatformApplication.h"

#include "Platform/PlatformLogger.h"

#include "GamePathResolver.h"
#include "GameConfig.h"
#include "Game.h"

GameStarter::GameStarter(GameStarterOptions options): _options(std::move(options)) {
    _logger = PlatformLogger::createStandardLogger(WIN_ENSURE_CONSOLE_OPTION);
    auto setVerboseLogging = [logger = _logger.get()](bool verbose) {
        logger->setLogLevel(APPLICATION_LOG, verbose ? LOG_VERBOSE : LOG_INFO);
        logger->setLogLevel(PLATFORM_LOG, verbose ? LOG_VERBOSE : LOG_ERROR);
    };
    setVerboseLogging(_options.verbose);
    EngineIocContainer::ResolveLogger()->setBaseLogger(_logger.get());
    Engine::LogEngineBuildInfo();

    _application = std::make_unique<PlatformApplication>(_logger.get());
    resolveDefaults(_application->platform(), &_options);
    initDataPath(_options.dataPath);

    _config = std::make_shared<GameConfig>(_options.configPath);
    _config->debug.VerboseLogging.subscribe([this, setVerboseLogging](bool value) {
        setVerboseLogging(value || _options.verbose);
    });
    if (_options.resetConfig) {
        _config->SaveConfiguration();
    } else {
        _config->LoadConfiguration();
    }

    _game = std::make_unique<Game>(_application.get(), _config);
}

GameStarter::~GameStarter() {
    EngineIocContainer::ResolveLogger()->setBaseLogger(nullptr);
}

void GameStarter::resolveDefaults(Platform *platform, GameStarterOptions* options) {
    if (options->dataPath.empty())
        options->dataPath = resolveMm7Path(platform);

    if (options->configPath.empty()) {
        options->configPath = "openenroth.ini";
        if (!options->dataPath.empty())
            options->configPath = options->dataPath + "/" + options->configPath;
    }
}

void GameStarter::run() {
    _game->run();
}
