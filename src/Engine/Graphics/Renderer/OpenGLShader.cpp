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
    release();

    GLuint vertex = loadShader(vertPath, GL_VERTEX_SHADER, openGLES);
    if (vertex == 0)
        return false;

    GLuint fragment = loadShader(fragPath, GL_FRAGMENT_SHADER, openGLES);
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
        logger->error("Could not link shader program ['{}', '{}']:\n{}", vertPath, fragPath, errors);
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

unsigned OpenGLShader::loadShader(std::string_view path, int type, bool openGLES) {
    std::string source;
    try {
        source = FileInputStream(path).readAll();
    } catch (const std::exception &e) {
        logger->error("Could not read shader source: {}", e.what()); // e.what() will contain file path.
        return 0;
    }

    if (!openGLES)
        source = "#version 410 core\n" + source;
    else
        source = "#version 320 es\n" + source;

    // compile shader
    const char *sources[1] = {source.data()};
    const GLint lengths[1] = {static_cast<GLint>(source.size())};

    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, sources, lengths);
    glCompileShader(result);

    std::string errors = compileErrors(result);
    if (!errors.empty()) {
        logger->error("Could not compile shader '{}':\n{}", path, errors);
        glDeleteShader(result);
        return 0;
    }

    return result;
}
