#pragma once

#include <filesystem>
#include <string>
#include <functional>

class EngineController;
class EngineTracer;

class TestController {
 public:
    TestController(EngineController *controller, const std::string &testDataPath);

    void loadGameFromTestData(const std::string &name);
    void playTraceFromTestData(const std::string &saveName, const std::string &traceName, std::function<void()> postLoadCallback = {});

    void prepareForNextTest();

 private:
    EngineController *_controller;
    std::filesystem::path _testDataPath;
};
