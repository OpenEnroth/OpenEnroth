#include "RendererEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(RendererType, CASE_INSENSITIVE, {
    {RENDERER_OPENGL,       "OpenGL"},
    {RENDERER_OPENGL_ES,    "OpenGLES"},
    {RENDERER_OPENGL_ES,    "OpenGL_ES"},
    {RENDERER_NULL,         "Null"}
})
