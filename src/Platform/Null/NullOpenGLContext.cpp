#include "NullOpenGLContext.h"

#include <cassert>
#include <thread>
#include <unordered_map>

#include "Utility/MapAccess.h"
#include "NullPlatformSharedState.h"

NullOpenGLContext::NullOpenGLContext(NullPlatformSharedState *state) : _state(state) {
    assert(state);

    bind();
}

NullOpenGLContext::~NullOpenGLContext() {
    std::erase_if(_state->contextByThreadId, [this] (const auto &pair) {
        return pair.second == this;
    });
}

bool NullOpenGLContext::bind() {
    _state->contextByThreadId[std::this_thread::get_id()] = this;
    return true;
}

bool NullOpenGLContext::unbind() {
    auto id = std::this_thread::get_id();
    if (valueOr(_state->contextByThreadId, id, nullptr) == this) {
        _state->contextByThreadId.erase(id);
        return true;
    }

    return false;
}

void NullOpenGLContext::swapBuffers() {
    // Do nothing.
}

void *NullOpenGLContext::getProcAddress(const char *name) {
    return nullptr; // No OpenGL support.
}
