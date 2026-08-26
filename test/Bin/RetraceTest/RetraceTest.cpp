#include "RetraceTest.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Trace/EngineTraceSimplePlayer.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Engine.h"

#include "Core/Trace/EventTrace.h"

#include "Library/Platform/Application/PlatformApplication.h"

#include "Utility/String/Format.h"
#include "Utility/String/Split.h"
#include "Utility/System/Path.h"

static EngineController *globalEngineController = nullptr;
static PlatformApplication *globalApplication = nullptr;

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

void RetraceTest::init(EngineController *game, PlatformApplication *application) {
    assert(!globalEngineController && !globalApplication);
    assert(game && application);
    globalEngineController = game;
    globalApplication = application;
}

void RetraceTest::TestBody() {
    Path savePath = _tracePath.withExtension(".mm7");
    Blob oldTraceBlob = Blob::fromFile(_tracePath);
    Blob oldSaveBlob = Blob::fromFile(savePath);

    EventTrace oldTrace = EventTrace::fromJsonBlob(oldTraceBlob, globalApplication->window());

    EngineTraceSimplePlayer *player = globalApplication->component<EngineTraceSimplePlayer>();
    EngineTraceRecorder *recorder = globalApplication->component<EngineTraceRecorder>();

    EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), oldTrace.header.config);
    recorder->startRecording(globalEngineController, oldSaveBlob);
    engine->config->graphics.FPSLimit.setValue(0);
    player->playTrace(globalEngineController, std::move(oldTrace.events), _tracePath.string(), TRACE_PLAYBACK_SKIP_RANDOM_CHECKS | TRACE_PLAYBACK_SKIP_STATE_CHECKS);
    EngineTraceRecording recording = recorder->finishRecording(globalEngineController);

    std::string oldTraceJson = EventTrace::normalizeJson(oldTraceBlob.str());
    std::string newTraceJson = EventTrace::normalizeJson(recording.trace.str());
    if (oldTraceJson != newTraceJson) {
        printTraceDiff(newTraceJson, oldTraceJson);
        ADD_FAILURE() << "Trace '" << _tracePath.string() << "' is not in canonical representation.";
    }
}
