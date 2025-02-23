#pragma once

#include <memory>
#include <functional>

#include "FileSystemStarter.h"
#include "GameStarterOptions.h"
#include "LogStarter.h"

class MemoryFileSystem;
class EngineFileSystem;
class Platform;
class Environment;
class Logger;
class BufferLogSink;
class DistLogSink;
class LogSink;
class PlatformApplication;
class GameConfig;
class Renderer;
class Engine;
class Game;
class GameBindings;
class OverlaySystem;
class ScriptingSystem;
class EngineController;

class GameStarter {
 public:
    explicit GameStarter(GameStarterOptions options);
    ~GameStarter();

    PlatformApplication *application() const {
        return _application.get();
    }

    GameConfig *config() const {
        return _config.get();
    }

    void run();

    void runInstrumented(std::function<void(EngineController *)> controlRoutine);

 private:
    void initWithLogger();

    static void resolveUserPath(Environment *environment, GameStarterOptions *options);
    static void resolveDataPath(Environment *environment, GameStarterOptions *options);
    static void failOnInvalidPath(std::string_view dataPath, Platform *platform);
    static void migrateSaves();

 private:
    GameStarterOptions _options;
    FileSystemStarter _fsStarter;
    LogStarter _logStarter;
    std::unique_ptr<Environment> _environment;
    std::shared_ptr<GameConfig> _config;
    std::unique_ptr<Platform> _platform;
    std::unique_ptr<PlatformApplication> _application;
    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<ScriptingSystem> _scriptingSystem;
    std::unique_ptr<OverlaySystem> _overlaySystem;
    std::unique_ptr<Engine> _engine;
    std::shared_ptr<Game> _game;
};
