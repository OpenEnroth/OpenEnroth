#include "OpenGLShader.h"

#include <string>

#include <glad/gl.h> // NOLINT: this is not a C system include.

#include "Library/Logger/Logger.h"
#include "Library/Serialization/EnumSerialization.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/DataPath.h"
#include "Utility/Exception.h"

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

int OpenGLShader::build(std::string_view name, std::string_view filename, bool OpenGLES, bool reload) {
    // 1. retrieve the vertex/fragment source code from filePath
    GLuint vertex = load(name, filename, GL_VERTEX_SHADER, OpenGLES);
    if (vertex == 0)
        return 0;

    GLuint fragment = load(name, filename, GL_FRAGMENT_SHADER, OpenGLES);
    if (fragment == 0)
        return 0;

    GLuint geometry = load(name, filename, GL_GEOMETRY_SHADER, OpenGLES, true);

    // shader Program
    int tempID = glCreateProgram();
    glAttachShader(tempID, vertex);
    glAttachShader(tempID, fragment);
    if (geometry != 0)
        glAttachShader(tempID, geometry);
    glLinkProgram(tempID);
    bool NOerror = checkCompileErrors(tempID, name, "program");
    if (!NOerror)
        tempID = 0;

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry != 0)
        glDeleteShader(geometry);

    if (tempID) {
        // set var members on first load
        if (reload == false) {
            ID = tempID;
            sFilename = filename;
        }
        return tempID;
    }

    logger->error("shader compilation failure: {}", filename);
    return 0;
}

bool OpenGLShader::reload(std::string_view name, bool OpenGLES) {
    int tryreload = build(name, sFilename, OpenGLES, true);

    if (tryreload) {
        glDeleteProgram(ID);
        ID = tryreload;
        return true;
    }

    logger->info("shader reload failed, reverting...");
    return false;
}

void OpenGLShader::use() {
    glUseProgram(ID);
}

std::string OpenGLShader::shaderTypeToExtension(int type) {
    std::string result;
    detail_extension::serialize(type, &result);
    return result;
}

std::string OpenGLShader::shaderTypeToName(int type) {
    std::string result;
    detail_name::serialize(type, &result);
    return result;
}

bool OpenGLShader::checkCompileErrors(int shader, std::string_view name, std::string_view type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "program") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            logger->error("{} {} shader compilation error:\n{}", name, type, infoLog);
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            logger->error("{} {} linking error:\n{}", name, type, infoLog);
            return false;
        }
    }
    return true;
}

int OpenGLShader::load(std::string_view name, std::string_view filename, int type, bool OpenGLES, bool nonFatal) {
    std::string directory = "shaders";
    std::string typeName = shaderTypeToName(type);
    std::string path = makeDataPath(directory, fmt::format("{}.{}", filename, shaderTypeToExtension(type)));

    try {
        std::string shaderString;
        if (!OpenGLES)
            shaderString = "#version 410 core\n";
        else
            shaderString = "#version 320 es\n";

        FileInputStream stream(path);
        shaderString += stream.readAll();

        // compile shader
        const char *shaderChar = shaderString.c_str();
        GLuint shaderHandler = glCreateShader(type);
        glShaderSource(shaderHandler, 1, &shaderChar, NULL);
        glCompileShader(shaderHandler);
        checkCompileErrors(shaderHandler, name, shaderTypeToName(type));

        return shaderHandler;
    } catch (const Exception &e) {
        if (!nonFatal)
            logger->error("Error occured during reading {} {} shader file at path {}: {}", name, typeName, path, e.what());
        return 0;
    }
}
