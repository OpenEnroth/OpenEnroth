#pragma once

#include <unordered_map>
#include <thread>

#include "NullPlatformOptions.h"

class NullOpenGLContext;

class NullPlatformSharedState {
 public:
    NullPlatformOptions options;
    uintptr_t nextWinId = 1;
    std::unordered_map<std::thread::id, NullOpenGLContext *> contextByThreadId;
};
