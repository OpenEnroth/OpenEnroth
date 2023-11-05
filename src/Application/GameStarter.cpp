#include "GameStarter.h"

#include <utility>
#include <filesystem>
#include <string>

#include "Engine/Engine.h"

#include "Library/Environment/Interface/Environment.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"
#include "Library/Logger/BufferLogSink.h"

#include "Library/Platform/Interface/Platform.h"
#include "Library/Platform/Null/NullPlatform.h"

#include "GamePathResolver.h"
#include "GameConfig.h"
#include "Game.h"

GameStarter::GameStarter(GameStarterOptions options): _options(std::move(options)) {
    // Init environment.
    _environment = Environment::createStandardEnvironment();

    // Init logger.
    _bufferSink = std::make_unique<BufferLogSink>();
    _defaultSink = LogSink::createDefaultSink();
    _logger = std::make_unique<Logger>(LOG_TRACE, _bufferSink.get());
    Engine::LogEngineBuildInfo();

    // Create platform.
    if (_options.headless) {
        _platform = std::make_unique<NullPlatform>(NullPlatformOptions());
    } else {
        _platform = Platform::createStandardPlatform(_logger.get());
    }

    // Init paths.
    resolveDefaults(_environment.get(), &_options);

    // Init config - needs data paths initialized.
    _config = std::make_shared<GameConfig>();
    if (_options.useConfig) {
        if (std::filesystem::exists(_options.configPath)) {
            _config->load(_options.configPath);
            _logger->info("Configuration file '{}' loaded!", _options.configPath);
        } else {
            _config->reset();
            _logger->info("Could not read configuration file '{}'! Loaded default configuration instead!", _options.configPath);
        }
    }
    if (_options.headless)
        _config->graphics.Renderer.setValue(RENDERER_NULL); // TODO(captainurist): we shouldn't be writing to config here.

    // Finish logger init now that we know the desired log level.
    if (_options.logLevel) {
        _logger->setLevel(*_options.logLevel);
    } else {
        _logger->setLevel(_config->debug.LogLevel.value());
    }
    _logger->setSink(_defaultSink.get());
    _bufferSink->flush(_logger.get());

    // Validate data paths.
    initDataPath(_environment.get(), _platform.get(), _options.dataPath);

    // Create application & game.
    _application = std::make_unique<PlatformApplication>(_platform.get());
    _game = std::make_unique<Game>(_application.get(), _config);
}

GameStarter::~GameStarter() = default;

void GameStarter::resolveDefaults(Environment *environment, GameStarterOptions* options) {
    if (options->dataPath.empty())
        options->dataPath = resolveMm7Path(environment);

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
