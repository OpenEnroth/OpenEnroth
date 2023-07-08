#include "GameStarter.h"

#include <utility>

#include "Engine/EngineIocContainer.h"
#include "Engine/Engine.h"

#include "Library/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"

#include "Platform/PlatformLogger.h"

#include "GamePathResolver.h"
#include "GameConfig.h"
#include "Game.h"

GameStarter::GameStarter(GameStarterOptions options): _options(std::move(options)) {
    _logger = PlatformLogger::createStandardLogger(WIN_ENSURE_CONSOLE_OPTION);
    auto setLogLevel = [logger = _logger.get()](PlatformLogLevel level) {
        logger->setLogLevel(APPLICATION_LOG, level);
        logger->setLogLevel(PLATFORM_LOG, level);
    };
    if (_options.logLevel)
        setLogLevel(*_options.logLevel);
    EngineIocContainer::ResolveLogger()->setBaseLogger(_logger.get());
    Engine::LogEngineBuildInfo();

    _application = std::make_unique<PlatformApplication>(_logger.get());
    resolveDefaults(_application->platform(), &_options);
    initDataPath(_options.dataPath);

    _config = std::make_shared<GameConfig>(_options.configPath);
    if (_options.resetConfig) {
        _config->SaveConfiguration();
    } else {
        _config->LoadConfiguration();
    }

    if (!_options.logLevel)
        setLogLevel(_config->debug.LogLevel.value());

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
