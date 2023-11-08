#include "GameStarter.h"

#include <utility>
#include <filesystem>
#include <string>
#include <vector>

#include "Engine/Engine.h"

#include "Library/Environment/Interface/Environment.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"
#include "Library/Logger/BufferLogSink.h"

#include "Library/Platform/Interface/Platform.h"
#include "Library/Platform/Null/NullPlatform.h"

#include "Utility/DataPath.h"

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
    resolvePaths(_environment.get(), &_options, _logger.get());

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

    // Init global data path.
    initDataPath(_platform.get(), _options.dataPath);

    // Create application & game.
    _application = std::make_unique<PlatformApplication>(_platform.get());
    _game = std::make_unique<Game>(_application.get(), _config);
}

GameStarter::~GameStarter() = default;

void GameStarter::resolvePaths(Environment *environment, GameStarterOptions* options, Logger *logger) {
    if (options->dataPath.empty()) {
        std::vector<std::string> candidates = resolveMm7Paths(environment);
        assert(!candidates.empty());

        if (candidates.size() > 1) {
            for (int i = 0; i < candidates.size(); i++) {
                std::string missingFile;
                if (!std::filesystem::exists(candidates[i])) {
                    logger->info("Data path candidate #{} ('{}') doesn't exist.", i + 1, candidates[i]);
                } else if (!validateDataPath(candidates[i], &missingFile)) {
                    logger->info("Data path candidate #{} ('{}') is missing file '{}'.", i + 1, candidates[i], missingFile);
                } else {
                    logger->info("Data path candidate #{} ('{}') is OK!", i + 1, candidates[i]);
                    options->dataPath = candidates[i];
                    break;
                }
            }
        }

        if (options->dataPath.empty()) // Only one candidate, or no valid candidates? Use the last one & re-check it later.
            options->dataPath = candidates.back();
    }

    assert(!options->dataPath.empty());
    logger->info("Using data path '{}'.", options->dataPath);

    if (options->useConfig && options->configPath.empty())
        options->configPath = options->dataPath + "/openenroth.ini";
}

void GameStarter::run() {
    _game->run();

    if (_options.useConfig) {
        _config->save(_options.configPath);
        logger->info("Configuration file '{}' saved!", _options.configPath);
    }
}
