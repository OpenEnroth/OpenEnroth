#pragma once

#include "Utility/Flags.h"

enum class EngineTracePlaybackFlag {
    TRACE_PLAYBACK_SKIP_RANDOM_CHECKS = 0x1,
    TRACE_PLAYBACK_SKIP_TIME_CHECKS = 0x2,
    TRACE_PLAYBACK_SKIP_STATE_CHECKS = 0x4,
};
using enum EngineTracePlaybackFlag;
MM_DECLARE_FLAGS(EngineTracePlaybackFlags, EngineTracePlaybackFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(EngineTracePlaybackFlags)

enum class EngineTraceRecordingFlag {
    TRACE_RECORDING_LOAD_EXISTING_SAVE = 0x1
};
using enum EngineTraceRecordingFlag;
MM_DECLARE_FLAGS(EngineTraceRecordingFlags, EngineTraceRecordingFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(EngineTraceRecordingFlags)
