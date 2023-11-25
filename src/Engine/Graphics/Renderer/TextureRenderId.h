#pragma once

#include <cstdint>

/**
 * Texture id, as used by `Renderer` API.
 *
 * We only have an OpenGL implementation right now, so this is essentially a wrapper atop a `GLuint` that removes
 * all the integer operations.
 */
class TextureRenderId {
 public:
    TextureRenderId() = default;
    explicit TextureRenderId(intptr_t id) : _value(id) {}

    [[nodiscard]] intptr_t value() const {
        return _value;
    }

    [[nodiscard]] bool isValid() const {
        return _value != -1;
    }

    explicit operator bool() const {
        return isValid();
    }

 private:
    intptr_t _value = -1;
};
