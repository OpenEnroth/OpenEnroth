#include "TraceGenerator.h"

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Trace/EngineTraceRecording.h"
#include "Engine/EngineGlobals.h"

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/Platform/Application/PlatformApplication.h"

#include "Utility/String/Format.h"

void generateTrace(EngineController &game, FileSystem *fs, std::string_view baseName,
                   const std::function<void()> &routine) {
    EngineTraceRecorder *recorder = application->component<EngineTraceRecorder>();

    recorder->startRecording(&game);
    routine();
    EngineTraceRecording recording = recorder->finishRecording(&game);

    fs->write(fmt::format("{}.json", baseName), recording.trace);
    fs->write(fmt::format("{}.mm7", baseName), recording.save);
}
