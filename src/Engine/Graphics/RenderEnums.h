#pragma once

#include "Library/Serialization/SerializationFwd.h"

enum class RendererType {
    RENDERER_OPENGL,
    RENDERER_OPENGL_ES,
    RENDERER_NULL
};
using enum RendererType;
MM_DECLARE_SERIALIZATION_FUNCTIONS(RendererType)
