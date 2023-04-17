#pragma once

#include <filesystem>
#include <string>
#include <functional>

#include "Engine/Components/Trace/EngineTracePlaybackFlags.h"

class EngineController;
class EngineTracer;

class TestController {
 public:
    TestController(EngineController *controller, const std::string &testDataPath);

    std::string fullPathInTestData(const std::string &fileName);

    void loadGameFromTestData(const std::string &name);
    void playTraceFromTestData(const std::string &saveName, const std::string &traceName, std::function<void()> postLoadCallback = {});
    void playTraceFromTestData(const std::string &saveName, const std::string &traceName, EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback = {});

    void prepareForNextTest();

 private:
    EngineController *_controller;
    std::filesystem::path _testDataPath;
};
