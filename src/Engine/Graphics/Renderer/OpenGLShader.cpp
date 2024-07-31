#include "OpenGLShader.h"

#include <cassert>
#include <string>

#include <glad/gl.h> // NOLINT: this is not a C system include.

#include "Library/Logger/Logger.h"

#include "Utility/Streams/FileInputStream.h"

static std::string compileErrors(int shader) {
    GLint success = 1;
    GLchar infoLog[2048];
    if (glIsShader(shader)) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
            glGetShaderInfoLog(shader, 2048, NULL, infoLog);
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
            glGetProgramInfoLog(shader, 2048, NULL, infoLog);
    }

    if (!success) {
        if (infoLog[0]) {
            return infoLog;
        } else {
            return "Unknown error";
        }
    }

    return {};
}

OpenGLShader::~OpenGLShader() {
    release();
}

bool OpenGLShader::load(std::string_view vertPath, std::string_view fragPath, bool openGLES) {
    Blob vertSource, fragSource;
    try {
        vertSource = Blob::fromFile(vertPath);
        fragSource = Blob::fromFile(fragPath);
    } catch (const std::exception &e) {
        logger->error("Could not read shader source: {}", e.what()); // e.what() will contain file path.
        return false;
    }

    return load(vertSource, fragSource, openGLES);
}

bool OpenGLShader::load(const Blob &vertSource, const Blob &fragSource, bool openGLES) {
    GLuint vertex = loadShader(vertSource, GL_VERTEX_SHADER, openGLES);
    if (vertex == 0)
        return false;

    GLuint fragment = loadShader(fragSource, GL_FRAGMENT_SHADER, openGLES);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return false;
    }

    int result = glCreateProgram();
    glAttachShader(result, vertex);
    glAttachShader(result, fragment);
    glLinkProgram(result);

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    std::string errors = compileErrors(result);
    if (!errors.empty()) {
        logger->error("Could not link shader program ['{}', '{}']:\n{}", vertSource.displayPath(), fragSource.displayPath(), errors);
        glDeleteProgram(result);
        return false;
    }

    _id = result;
    return true;
}

void OpenGLShader::release() {
    if (_id == 0)
        return;

    glDeleteProgram(_id);
    _id = 0;
}

int OpenGLShader::uniformLocation(const char *name) {
    assert(isValid());

    return glGetUniformLocation(_id, name);
}

int OpenGLShader::attribLocation(const char *name) {
    assert(isValid());

    return glGetAttribLocation(_id, name);
}

void OpenGLShader::use() {
    assert(isValid());

    glUseProgram(_id);
}

unsigned OpenGLShader::loadShader(const Blob &source, int type, bool openGLES) {
    std::string_view version;
    if (!openGLES)
        version = "#version 410 core\n";
    else
        version = "#version 320 es\n";

    // compile shader
    const char *sources[2] = {version.data(), static_cast<const char *>(source.data())};
    const GLint lengths[2] = {static_cast<GLint>(version.size()), static_cast<GLint>(source.size())};

    GLuint result = glCreateShader(type);
    glShaderSource(result, 2, sources, lengths);
    glCompileShader(result);

    std::string errors = compileErrors(result);
    if (!errors.empty()) {
        logger->error("Could not compile shader '{}':\n{}", source.displayPath(), errors);
        glDeleteShader(result);
        return 0;
    }

    return result;
}
