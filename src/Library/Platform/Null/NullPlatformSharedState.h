#pragma once

#include <cassert>
#include <unordered_map>
#include <thread>
#include <utility>

#include "NullPlatformOptions.h"

class NullOpenGLContext;

class NullPlatformSharedState {
 public:
    explicit NullPlatformSharedState(NullPlatformOptions options) {
        assert(!options.displayGeometries.empty());

        this->options = std::move(options);
    }

    NullPlatformOptions options;
    std::unordered_map<std::thread::id, NullOpenGLContext *> contextByThreadId;
    bool cursorShown = true;
};
