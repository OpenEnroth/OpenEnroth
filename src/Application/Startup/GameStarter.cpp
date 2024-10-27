#include "GameStarter.h"

#include <utility>
#include <filesystem>
#include <string>
#include <vector>
#include <memory>

#include "Application/GameConfig.h"
#include "Application/Game.h"
#include "Application/GameKeyboardController.h"
#include "Application/GameWindowHandler.h"
#include "Application/GameTraceHandler.h"

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/Graphics/Renderer/RendererFactory.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Trace/EngineTraceSimplePlayer.h"
#include "Engine/Components/Trace/EngineTraceSimpleRecorder.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/Components/Random/EngineRandomComponent.h"

#include "GUI/Overlay/OverlaySystem.h"

#include "Library/Environment/Interface/Environment.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"
#include "Library/Logger/DistLogSink.h"
#include "Library/Logger/BufferLogSink.h"
#include "Library/Platform/Interface/Platform.h"
#include "Library/Platform/Null/NullPlatform.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"

#include "Scripting/AudioBindings.h"
#include "Scripting/ConfigBindings.h"
#include "Scripting/OverlayBindings.h"
#include "Scripting/GameBindings.h"
#include "Scripting/InputBindings.h"
#include "Scripting/InputScriptEventHandler.h"
#include "Scripting/LoggerBindings.h"
#include "Scripting/PlatformBindings.h"
#include "Scripting/RendererBindings.h"
#include "Scripting/ScriptingSystem.h"

#include "Utility/Exception.h"

#include "PathResolver.h"

constexpr std::string_view configName = "openenroth.ini";

GameStarter::GameStarter(GameStarterOptions options): _options(std::move(options)) {
    // Init logging.
    _logStarter.initPrimary();
    Engine::LogEngineBuildInfo();

    try {
        initWithLogger();
    } catch (const std::exception &e) {
        logger->critical("Terminated with exception: {}", e.what());
        throw;
    }
}

void GameStarter::initWithLogger() {
    // Init environment.
    _environment = Environment::createStandardEnvironment();

    // Resolve user path, create user fs & file logger.
    resolveUserPath(_environment.get(), &_options);
    _fsStarter.initUserFs(_options.ramFsUserData, _options.userPath);
    _logStarter.initSecondary(ufs);

    // Resolve data path, create data fs.
    // TODO(captainurist): actually move datapath to config?
    resolveDataPath(_environment.get(), &_options);
    _fsStarter.initDataFs(_options.dataPath);

    // Migrate saves & config if needed.
    if (!_options.ramFsUserData && _options.dataPath != _options.userPath)
        migrateUserData();

    // Init config.
    _config = std::make_shared<GameConfig>();
    if (ufs->exists(configName)) {
        _config->load(ufs->openForReading(configName).get());
        logger->info("Configuration file '{}' loaded!", ufs->displayPath(configName));
    } else {
        _config->reset();
        logger->info("Could not read configuration file '{}'! Loaded default configuration instead!", ufs->displayPath(configName));
    }

    // Finish logger init now that we know the desired log level.
    _logStarter.initFinal(_options.logLevel ? *_options.logLevel : _config->debug.LogLevel.value());

    // Create platform.
    if (_options.headless) {
        _platform = std::make_unique<NullPlatform>(NullPlatformOptions());
    } else {
        _platform = Platform::createStandardPlatform();
    }

    // Prepare OpenAL settings. Unfortunately the only way to do this is by manipulating the env variables.
    // If we don't do this, game tests in GH action on macos-14 will hang for 9 min on init.
    if (_options.headless)
        _environment->setenv("ALSOFT_DRIVERS", "null");

    // Can validate the resolved data path now.
    failOnInvalidPath(_options.dataPath, _platform.get());

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
    RendererType rendererType = _options.headless ? RENDERER_NULL : _config->graphics.Renderer.value();
    _renderer = RendererFactory().createRenderer(rendererType, _config);
    ::render = _renderer.get();
    if (!_renderer->Initialize())
        throw Exception("Renderer failed to initialize"); // TODO(captainurist): Initialize should throw?

    // Init overlays.
    _overlaySystem = std::make_unique<OverlaySystem>(*_renderer, *_application);

    // Init io.
    ::keyboardActionMapping = std::make_shared<Io::KeyboardActionMapping>(_config);;
    ::keyboardInputHandler = std::make_shared<Io::KeyboardInputHandler>(_application->component<GameKeyboardController>(),
        keyboardActionMapping
    );
    ::mouse = EngineIocContainer::ResolveMouse();

    // Init engine.
    _engine = std::make_unique<Engine>(_config, *_overlaySystem);
    ::engine = _engine.get();
    _engine->Initialize();

    // Init game.
    _game = std::make_unique<Game>(_application.get(), _config);

    // Init scripting system.
    _scriptingSystem = std::make_unique<ScriptingSystem>("scripts", "init.lua", *_application, *_logStarter.rootSink());
    _scriptingSystem->addBindings<LoggerBindings>("log");
    _scriptingSystem->addBindings<GameBindings>("game");
    _scriptingSystem->addBindings<ConfigBindings>("config");
    _scriptingSystem->addBindings<PlatformBindings>("platform", *_application);
    _scriptingSystem->addBindings<InputBindings>("input", *_application->component<InputScriptEventHandler>());
    _scriptingSystem->addBindings<OverlayBindings>("overlay", *_overlaySystem);
    _scriptingSystem->addBindings<AudioBindings>("audio");
    _scriptingSystem->addBindings<RendererBindings>("renderer");
    _scriptingSystem->executeEntryPoint();
}

GameStarter::~GameStarter() {
    _application->removeComponent<EngineControlComponent>(); // Join the control thread first.

    ::engine = nullptr;
    ::render = nullptr;
    ::application = nullptr;
    ::platform = nullptr;
    ::eventLoop = nullptr;
    ::window = nullptr;
    ::eventHandler = nullptr;
    ::openGLContext = nullptr;
}

void GameStarter::resolveUserPath(Environment *environment, GameStarterOptions *options) {
    if (options->userPath.empty())
        options->userPath = resolveMm7UserPath(environment);
    logger->info("Using user path '{}'.", options->userPath);
}

void GameStarter::resolveDataPath(Environment *environment, GameStarterOptions *options) {
    std::vector<std::string> candidates;
    if (!options->dataPath.empty()) {
        candidates.push_back(options->dataPath);
    } else {
        candidates = resolveMm7Paths(environment);
    }
    assert(!candidates.empty());

    for (int i = 0; i < candidates.size(); i++) {
        std::string missingFile;
        if (!std::filesystem::exists(candidates[i])) {
            logger->info("Data path #{} ('{}') doesn't exist.", i + 1, candidates[i]);
        } else if (!validateMm7Path(candidates[i], &missingFile)) {
            logger->info("Data path #{} ('{}') is missing file '{}'.", i + 1, candidates[i], missingFile);
        } else {
            logger->info("Data path #{} ('{}') is OK!", i + 1, candidates[i]);
            options->dataPath = candidates[i];
            break;
        }
    }

    // Just use the last data path if all paths are invalid. We'll throw later.
    if (options->dataPath.empty())
        options->dataPath = candidates.back();
    logger->info("Using data path '{}'.", options->dataPath);
}

void GameStarter::failOnInvalidPath(std::string_view dataPath, Platform *platform) {
    std::string missingFile;
    if (validateMm7Path(dataPath, &missingFile))
        return;

    std::string message = fmt::format(
        "Required file '{}' not found.\n"
        "\n"
        "You should acquire licensed copy of M&M VII and copy its resources to {}.",
        missingFile,
        dataPath
    );
    platform->showMessageBox("CRITICAL ERROR: missing resources", message);
    throw Exception("Data folder '{}' validation failed", dataPath);
}

void GameStarter::migrateUserData() {
    logger->info("Migrating user data from '{}' to '{}'...", dfs->displayPath(""), ufs->displayPath(""));

    if (ufs->exists("saves") && !ufs->ls("saves").empty()) {
        logger->info("    Target saves directory is not empty, skipping saves migration.");
    } else if (!dfs->exists("saves")) {
        logger->info("    No save files to migrate.");
    } else {
        for (const DirectoryEntry &entry : dfs->ls("saves")) {
            if (entry.type == FILE_REGULAR) {
                std::string path = fmt::format("saves/{}", entry.name);
                ufs->write(path, dfs->read(path));
                logger->info("    Copied '{}'.", entry.name);
            }
        }
    }

    if (ufs->exists(configName)) {
        logger->info("    Target config exists, skipping config migration.");
    } else if (!dfs->exists(configName)) {
        logger->info("    No config file to migrate.");
    } else {
        ufs->write(configName, dfs->read(configName));
        logger->info("    Copied '{}'.", configName);
    }
}

void GameStarter::run() {
    try {
        _game->run();

        _application->component<GameWindowHandler>()->UpdateConfigFromWindow(_config.get());
        _config->save(ufs->openForWriting(configName).get());
        logger->info("Configuration file '{}' saved!", configName);
    } catch (const std::exception &e) {
        // Log the exception so that it goes to all registered loggers.
        logger->critical("Terminated with exception: {}", e.what());
        throw;
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
