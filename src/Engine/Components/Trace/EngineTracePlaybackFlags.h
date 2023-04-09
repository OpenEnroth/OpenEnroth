#pragma once

#include "Utility/Flags.h"

enum class EngineTracePlaybackFlag {
    TRACE_PLAYBACK_SKIP_RANDOM_CHECKS = 0x1,
    TRACE_PLAYBACK_SKIP_TIME_CHECKS = 0x2,
    TRACE_PLAYBACK_DONT_ADVANCE_RANDOM_STATE = 0x4, // TODO(captainurist): just retrace everything & use this by default
};
using enum EngineTracePlaybackFlag;
MM_DECLARE_FLAGS(EngineTracePlaybackFlags, EngineTracePlaybackFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(EngineTracePlaybackFlags)
