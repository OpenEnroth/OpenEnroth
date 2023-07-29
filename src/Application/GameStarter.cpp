#include "GameStarter.h"

#include <utility>
#include <filesystem>

#include "Engine/EngineIocContainer.h"
#include "Engine/Engine.h"

#include "Library/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"

#include "Platform/PlatformLogger.h"
#include "Platform/Platform.h"
#include "Platform/Null/NullPlatform.h"

#include "GamePathResolver.h"
#include "GameConfig.h"
#include "Game.h"

GameStarter::GameStarter(GameStarterOptions options): _options(std::move(options)) {
    // Init logger.
    _logger = PlatformLogger::createStandardLogger();
    auto setLogLevel = [logger = _logger.get()](PlatformLogLevel level) {
        logger->setLogLevel(APPLICATION_LOG, level);
        logger->setLogLevel(PLATFORM_LOG, level);
    };
    if (_options.logLevel)
        setLogLevel(*_options.logLevel);
    EngineIocContainer::ResolveLogger()->setBaseLogger(_logger.get());

    // Init config.
    _config = std::make_shared<GameConfig>();
    std::function<void()> logConfigMessage;
    if (_options.useConfig) {
        if (std::filesystem::exists(_options.configPath)) {
            _config->load(_options.configPath);
            logConfigMessage = [&] {
                logger->info("Configuration file '{}' loaded!", _options.configPath);
            };
        } else {
            _config->reset();
            logConfigMessage = [&] {
                logger->warning("Could not read configuration file '{}'! Loaded default configuration instead!", _options.configPath);
            };
        }
    }

    // Set proper log level & write out first log messages.
    if (!_options.logLevel)
        setLogLevel(_config->debug.LogLevel.value());
    Engine::LogEngineBuildInfo();
    if (logConfigMessage)
        logConfigMessage();

    // Create platform & do platform-related init.
    if (_options.renderer)
        _config->graphics.Renderer.setValue(*_options.renderer);
    if (_config->graphics.Renderer.value() == RENDERER_NULL) { // TODO(captainurist): 'null' is not about renderer, redo properly.
        _platform = std::make_unique<NullPlatform>(NullPlatformOptions());
    } else {
        _platform = Platform::createStandardPlatform(_logger.get());
    }
    _application = std::make_unique<PlatformApplication>(_platform.get());
    resolveDefaults(_application->platform(), &_options);
    initDataPath(_options.dataPath);

    // Create game.
    _game = std::make_unique<Game>(_application.get(), _config);
}

GameStarter::~GameStarter() {
    EngineIocContainer::ResolveLogger()->setBaseLogger(nullptr);
}

void GameStarter::resolveDefaults(Platform *platform, GameStarterOptions* options) {
    if (options->dataPath.empty())
        options->dataPath = resolveMm7Path(platform);

    if (options->useConfig && options->configPath.empty()) {
        options->configPath = "openenroth.ini";
        if (!options->dataPath.empty())
            options->configPath = options->dataPath + "/" + options->configPath;
    }
}

void GameStarter::run() {
    _game->run();

    if (_options.useConfig) {
        _config->save(_options.configPath);
        logger->info("Configuration file '{}' saved!", _options.configPath);
    }
}
