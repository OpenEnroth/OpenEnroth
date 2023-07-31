#include "GameStarter.h"

#include <utility>
#include <filesystem>
#include <string>

#include "Engine/EngineIocContainer.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"

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

    // Create platform & init data paths.
    if (_options.headless) {
        _platform = std::make_unique<NullPlatform>(NullPlatformOptions());
    } else {
        // TODO(captainurist): this can't use log level from config. Introduce a buffer logger, log into a buffer,
        //                     dump into the real logger when it is constructed.
        _platform = Platform::createStandardPlatform(_logger.get());
    }
    resolveDefaults(_platform.get(), &_options);

    // Init config - needs data paths initialized.
    _config = std::make_shared<GameConfig>();
    std::string configLogMessage;
    if (_options.useConfig) {
        if (std::filesystem::exists(_options.configPath)) {
            _config->load(_options.configPath);
            configLogMessage = fmt::format("Configuration file '{}' loaded!", _options.configPath);
        } else {
            _config->reset();
            configLogMessage = fmt::format("Could not read configuration file '{}'! Loaded default configuration instead!", _options.configPath);
        }
    }
    if (!_options.logLevel)
        setLogLevel(_config->debug.LogLevel.value());
    if (_options.headless)
        _config->graphics.Renderer.setValue(RENDERER_NULL); // TODO(captainurist): we shouldn't be writing to config here.

    // Write the first log messages.
    Engine::LogEngineBuildInfo();
    if (!configLogMessage.empty())
        logger->info("{}", configLogMessage);

    // Validate data paths.
    ::platform = _platform.get(); // TODO(captainurist): a hack to make validateDataPath work.
    initDataPath(_options.dataPath);

    // Create application & game.
    _application = std::make_unique<PlatformApplication>(_platform.get());
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
