#include "GLShaderLoader.h"

#include "Library/Serialization/EnumSerialization.h"

namespace detail_extension {
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(GLenum, CASE_SENSITIVE, {
    {GL_VERTEX_SHADER, "vert"},
    {GL_FRAGMENT_SHADER, "frag"},
    {GL_GEOMETRY_SHADER, "geom"},
    {GL_COMPUTE_SHADER, "comp"},
    {GL_TESS_CONTROL_SHADER, "tesc"},
    {GL_TESS_EVALUATION_SHADER, "tese"}
})
} // namespace detail_extension

namespace detail_name {
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(GLenum, CASE_SENSITIVE, {
    {GL_VERTEX_SHADER, "vertex"},
    {GL_FRAGMENT_SHADER, "fragment"},
    {GL_GEOMETRY_SHADER, "geometry"},
    {GL_COMPUTE_SHADER, "compute"},
    {GL_TESS_CONTROL_SHADER, "tessellation control"},
    {GL_TESS_EVALUATION_SHADER, "tessellation evaluation"}
})
} // namespace detail_name

std::string GLShader::shaderTypeToExtension(GLenum type) {
    std::string result;
    detail_extension::serialize(type, &result);
    return result;
}

std::string GLShader::shaderTypeToName(GLenum type) {
    std::string result;
    detail_name::serialize(type, &result);
    return result;
}
