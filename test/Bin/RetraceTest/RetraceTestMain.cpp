#include <gtest/gtest.h>

#include <cassert>

#include <algorithm>
#include <filesystem>
#include <ranges>
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

#include "RetraceTestOptions.h"

static std::string normalizeText(std::string_view text) {
    // Normalize to UNIX line endings. Need this b/c git on Windows checks out CRLF line endings.
    std::string result = replaceAll(text, "\r\n", "\n");

    // Also drop trailing newlines. Vim always adds a newline, but retracing removes it.
    while (result.ends_with('\n'))
        result.pop_back();

    return result;
}

static void printLines(const std::vector<std::string_view> &lines, size_t line, size_t delta) {
    // TODO(captainurist): #cpp26 use std::sat_sub
    for (size_t i = line > delta ? line - delta : 0; i < std::min(lines.size(), line + delta + 1); i++)
        fmt::println(stderr, "{:>5}: {}", i + 1, lines[i]);
}

static void printTraceDiff(std::string_view current, std::string_view canonical) {
    assert(canonical != current);

    std::vector<std::string_view> canonicalLines = split(canonical).by('\n');
    std::vector<std::string_view> currentLines = split(current).by('\n');

    size_t line = std::ranges::mismatch(canonicalLines, currentLines).in1 - canonicalLines.begin() + 1; // Lines are 1-indexed.

    fmt::println(stderr, "Canonical:");
    printLines(canonicalLines, line, 4);
    fmt::println(stderr, "Current:");
    printLines(currentLines, line, 4);
}

// One test per trace file, registered at runtime from the --test-path dir.
// Same static bridge as GameTest: gtest constructs fixtures through no-arg
// factories, so main() can only pass state in through init().
class RetraceTest : public testing::Test {
 public:
    explicit RetraceTest(std::string tracePath) : _tracePath(std::move(tracePath)) {}

    static void init(EngineController *game, PlatformApplication *application) {
        assert(!_game && !_application);
        assert(game && application);
        _game = game;
        _application = application;
    }

    void TestBody() override {
        std::string savePath = _tracePath.substr(0, _tracePath.length() - 5) + ".mm7";
        Blob oldTraceBlob = Blob::fromFile(_tracePath);
        Blob oldSaveBlob = Blob::fromFile(savePath);

        EventTrace oldTrace = EventTrace::fromJsonBlob(oldTraceBlob, _application->window());

        EngineTraceSimplePlayer *player = _application->component<EngineTraceSimplePlayer>();
        EngineTraceRecorder *recorder = _application->component<EngineTraceRecorder>();

        EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), oldTrace.header.config);
        recorder->startRecording(_game, oldSaveBlob);
        engine->config->graphics.FPSLimit.setValue(0);
        player->playTrace(_game, std::move(oldTrace.events), _tracePath, TRACE_PLAYBACK_SKIP_RANDOM_CHECKS | TRACE_PLAYBACK_SKIP_STATE_CHECKS);
        EngineTraceRecording recording = recorder->finishRecording(_game);

        std::string oldTraceJson = normalizeText(oldTraceBlob.str());
        std::string newTraceJson = normalizeText(recording.trace.str());
        if (oldTraceJson != newTraceJson) {
            printTraceDiff(newTraceJson, oldTraceJson);
            ADD_FAILURE() << "Trace '" << _tracePath << "' is not in canonical representation.";
        }
    }

 private:
    static EngineController *_game;
    static PlatformApplication *_application;

    std::string _tracePath;
};

EngineController *RetraceTest::_game = nullptr;
PlatformApplication *RetraceTest::_application = nullptr;

int platformMain(int argc, char **argv) {
    try {
        StackTraceOnCrash st;
        UnicodeCrt _(argc, argv);
        RetraceTestOptions opts = RetraceTestOptions::parse(argc, argv);
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
                                  [tracePath] { return new RetraceTest(tracePath); });

        testing::InitGoogleTest(&argc, argv);
        if (opts.listRequested)
            return RUN_ALL_TESTS();

        GameStarter starter(opts);
        int exitCode = 0;
        starter.runInstrumented([&] (EngineController *game) {
            RetraceTest::init(game, starter.application());
            exitCode = RUN_ALL_TESTS();
        });
        return exitCode;
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
