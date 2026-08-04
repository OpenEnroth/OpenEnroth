#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "Application/Startup/GameStarter.h"

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Trace/EngineTraceSimplePlayer.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Engine.h"

#include "Library/StackTrace/StackTraceOnCrash.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Trace/EventTrace.h"

#include "Utility/String/Format.h"
#include "Utility/String/Transformations.h"
#include "Utility/String/Split.h"
#include "Utility/UnicodeCrt.h"

#include "Bin/GameTest/GameTestOptions.h"

static EngineController *g_game = nullptr;
static PlatformApplication *g_application = nullptr;

static std::string normalizeText(std::string_view text) {
    // Normalize to UNIX line endings. Need this b/c git on Windows checks out CRLF line endings.
    std::string result = replaceAll(text, "\r\n", "\n");

    // Also drop trailing newlines. Vim always adds a newline, but retracing removes it.
    while (result.ends_with('\n'))
        result.pop_back();

    return result;
}

static std::string firstDiffContext(std::string_view current, std::string_view canonical) {
    std::vector<std::string_view> canonicalLines = split(canonical).by('\n');
    std::vector<std::string_view> currentLines = split(current).by('\n');

    size_t line = 0;
    while (line < canonicalLines.size() && line < currentLines.size() && canonicalLines[line] == currentLines[line])
        line++;

    std::string result;
    auto printLines = [&](std::string_view title, const std::vector<std::string_view> &lines) {
        result += fmt::format("{}:\n", title);
        // TODO(captainurist): #cpp26 use std::sat_sub
        for (size_t i = line > 4 ? line - 4 : 0; i < std::min(lines.size(), line + 5); i++)
            result += fmt::format("{:>5}: {}\n", i + 1, lines[i]);
    };
    printLines("Canonical", canonicalLines);
    printLines("Current", currentLines);
    return result;
}

// One test per trace file, registered at runtime from the --test-path dir.
class RetraceTest : public testing::Test {
 public:
    explicit RetraceTest(std::string tracePath) : _tracePath(std::move(tracePath)) {}

    void TestBody() override {
        std::string savePath = _tracePath.substr(0, _tracePath.length() - 5) + ".mm7";
        Blob oldTraceBlob = Blob::fromFile(_tracePath);
        Blob oldSaveBlob = Blob::fromFile(savePath);

        EventTrace oldTrace = EventTrace::fromJsonBlob(oldTraceBlob, g_application->window());

        EngineTraceSimplePlayer *player = g_application->component<EngineTraceSimplePlayer>();
        EngineTraceRecorder *recorder = g_application->component<EngineTraceRecorder>();

        EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), oldTrace.header.config);
        recorder->startRecording(g_game, oldSaveBlob);
        engine->config->graphics.FPSLimit.setValue(0);
        player->playTrace(g_game, std::move(oldTrace.events), _tracePath, TRACE_PLAYBACK_SKIP_RANDOM_CHECKS | TRACE_PLAYBACK_SKIP_STATE_CHECKS);
        EngineTraceRecording recording = recorder->finishRecording(g_game);

        std::string oldTraceJson = normalizeText(oldTraceBlob.str());
        std::string newTraceJson = normalizeText(recording.trace.str());
        if (oldTraceJson != newTraceJson)
            ADD_FAILURE() << "Trace '" << _tracePath << "' is not in canonical representation.\n"
                          << firstDiffContext(newTraceJson, oldTraceJson);
    }

 private:
    std::string _tracePath;
};

int platformMain(int argc, char **argv) {
    try {
        StackTraceOnCrash st;
        UnicodeCrt _(argc, argv);
        GameTestOptions opts = GameTestOptions::parse(argc, argv);
        if (opts.helpPrinted)
            return 1;

        std::vector<std::string> tracePaths;
        for (const auto &entry : std::filesystem::directory_iterator(opts.testPath))
            if (entry.path().extension() == ".json")
                tracePaths.push_back(entry.path().string());
        std::ranges::sort(tracePaths);

        for (const std::string &tracePath : tracePaths)
            testing::RegisterTest("Retrace", std::filesystem::path(tracePath).stem().string().c_str(),
                                  nullptr, nullptr, __FILE__, __LINE__,
                                  [tracePath] () -> testing::Test * { return new RetraceTest(tracePath); });

        testing::InitGoogleTest(&argc, argv);
        if (opts.listRequested)
            return RUN_ALL_TESTS();

        GameStarter starter(opts);
        int exitCode = 0;
        starter.runInstrumented([&] (EngineController *game) {
            g_game = game;
            g_application = starter.application();
            exitCode = RUN_ALL_TESTS();
        });
        return exitCode;
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
