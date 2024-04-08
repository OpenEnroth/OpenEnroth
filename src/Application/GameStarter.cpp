#include "GameStarter.h"

#include <utility>
#include <filesystem>
#include <string>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Random/Random.h"
#include "Engine/Graphics/Renderer/RendererFactory.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/NuklearLogSink.h"
#include "Engine/Graphics/NuklearEventHandler.h"
#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Trace/EngineTraceSimplePlayer.h"
#include "Engine/Components/Trace/EngineTraceSimpleRecorder.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/Components/Random/EngineRandomComponent.h"

#include "Library/Environment/Interface/Environment.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"
#include "Library/Logger/DistLogSink.h"
#include "Library/Logger/BufferLogSink.h"
#include "Library/Platform/Interface/Platform.h"
#include "Library/Platform/Null/NullPlatform.h"

#include "Utility/DataPath.h"
#include "Utility/Exception.h"

#include "GamePathResolver.h"
#include "GameConfig.h"
#include "Game.h"
#include "GameKeyboardController.h"
#include "GameWindowHandler.h"
#include "GameTraceHandler.h"
#include "GameLuaBindings.h"

GameStarter::GameStarter(GameStarterOptions options): _options(std::move(options)) {
    // Init environment.
    _environment = Environment::createStandardEnvironment();

    // Init logger.
    _bufferLogSink = std::make_unique<BufferLogSink>();
    _defaultLogSink = std::make_unique<DistLogSink>();
    _defaultLogSink->addLogSink(LogSink::createDefaultSink());
    _logger = std::make_unique<Logger>(LOG_TRACE, _bufferLogSink.get());
    Engine::LogEngineBuildInfo();

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

    // Finish logger init now that we know the desired log level.
    if (_options.logLevel) {
        _logger->setLevel(*_options.logLevel);
    } else {
        _logger->setLevel(_config->debug.LogLevel.value());
    }
    _logger->setSink(_defaultLogSink.get());
    _bufferLogSink->flush(_logger.get());

    // Create platform.
    if (_options.headless) {
        _platform = std::make_unique<NullPlatform>(NullPlatformOptions());
    } else {
        _platform = Platform::createStandardPlatform(_logger.get());
    }

    // Prepare OpenAL settings. Unfortunately the only way to do this is by manipulating the env variables.
    // If we don't do this, game tests in GH action on macos-14 will hang for 9 min on init.
    if (options.headless)
        _environment->setenv("ALSOFT_DRIVERS", "null");

    // Init global data path.
    initDataPath(_platform.get(), _options.dataPath);

    // Create application.
    _application = std::make_unique<PlatformApplication>(_platform.get());
    ::application = _application.get();
    ::platform = _application->platform();
    ::eventLoop = _application->eventLoop();
    ::window = _application->window();
    ::eventHandler = _application->eventHandler();
    ::openGLContext = _application->openGLContext(); // OK to store into a global even if not yet initialized

    // Install & set up components.
    // It doesn't matter where to put control component as it's running the control routine after a call to `SwapBuffers`.
    // But the trace component should go after the deterministic component - deterministic component updates tick count,
    // and then trace component stores the updated value in a recorded `PaintEvent`.
    _application->installComponent(std::make_unique<GameKeyboardController>()); // This one should go before the window handler.
    _application->installComponent(std::make_unique<GameWindowHandler>());
    _application->installComponent(std::make_unique<EngineControlComponent>());
    _application->installComponent(std::make_unique<EngineTraceSimpleRecorder>());
    _application->installComponent(std::make_unique<EngineTraceSimplePlayer>());
    _application->installComponent(std::make_unique<EngineDeterministicComponent>());
    _application->installComponent(std::make_unique<EngineTraceRecorder>());
    _application->installComponent(std::make_unique<EngineTracePlayer>());
    _application->installComponent(std::make_unique<GameTraceHandler>());
    _application->installComponent(std::make_unique<EngineRandomComponent>());
    _application->component<EngineRandomComponent>()->setTracing(_options.tracingRng);

    // Init main window. Should happen before the renderer init, which depends on window dimensions & mode.
    _application->component<GameWindowHandler>()->UpdateWindowFromConfig(_config.get());

    // Init renderer.
    _renderer = RendererFactory().createRenderer(_options.headless ? RENDERER_NULL : _config->graphics.Renderer.value(), _config);
    ::render = _renderer.get();
    if (!_renderer->Initialize())
        throw Exception("Renderer failed to initialize"); // TODO(captainurist): Initialize should throw?

    // Init Nuklear - depends on renderer.
    _nuklear = Nuklear::Initialize();
    if (!_nuklear)
        logger->error("Nuklear failed to initialize");
    ::nuklear = _nuklear.get();
    if (_nuklear) {
        _application->installComponent(std::make_unique<NuklearEventHandler>());
        _nuklear->addInitLuaFile("init.lua");
        _nuklear->addInitLuaLibs(GameLuaBindings::init);
        _defaultLogSink->addLogSink(NuklearLogSink::createNuklearLogSink());
    }

    // Init io.
    ::keyboardActionMapping = std::make_shared<Io::KeyboardActionMapping>(_config);;
    ::keyboardInputHandler = std::make_shared<Io::KeyboardInputHandler>(_application->component<GameKeyboardController>(),
        keyboardActionMapping
    );
    ::mouse = EngineIocContainer::ResolveMouse();

    // Init engine.
    _engine = std::make_unique<Engine>(_config);
    ::engine = _engine.get();
    _engine->Initialize();

    // Init game.
    _game = std::make_unique<Game>(_application.get(), _config);
}

GameStarter::~GameStarter() {
    ::engine = nullptr;

    ::nuklear = nullptr;

    ::render = nullptr;

    ::application = nullptr;
    ::platform = nullptr;
    ::eventLoop = nullptr;
    ::window = nullptr;
    ::eventHandler = nullptr;
    ::openGLContext = nullptr;
}

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
        _application->component<GameWindowHandler>()->UpdateConfigFromWindow(_config.get());
        _config->save(_options.configPath);
        logger->info("Configuration file '{}' saved!", _options.configPath);
    }
}

void GameStarter::runInstrumented(std::function<void(EngineController *)> controlRoutine) {
    // Instrumentation implies that we'll be running traces, either hand-crafted, or from files. So calling
    // `prepareForPlayback` here makes sense. This also disables intro videos.
    EngineTraceStateAccessor::prepareForPlayback(_config.get(), {});

    _application->component<EngineControlComponent>()->runControlRoutine([controlRoutine = std::move(controlRoutine)] (EngineController *game) {
        game->tick(10); // Let the game thread initialize everything.

        controlRoutine(game);

        game->goToMainMenu();
        game->pressGuiButton("MainMenu_ExitGame");
    });

    run();
}
