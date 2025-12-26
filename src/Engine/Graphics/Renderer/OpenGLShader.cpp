#include "OpenGLShader.h"

#include <cassert>
#include <string>

#include <glad/gl.h> // NOLINT: this is not a C system include.

#include "Library/Logger/Logger.h"
#include "Library/Preprocessor/Preprocessor.h"

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

bool OpenGLShader::load(const Blob &vertSource, const Blob &fragSource, bool openGLES, const FileSystem *pwd) {
    GLuint vertex = loadShader(vertSource, GL_VERTEX_SHADER, openGLES, pwd);
    if (vertex == 0)
        return false;

    GLuint fragment = loadShader(fragSource, GL_FRAGMENT_SHADER, openGLES, pwd);
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

unsigned OpenGLShader::loadShader(const Blob &source, int type, bool openGLES, const FileSystem *pwd) {
    assert(pwd);

    // Preprocess source with version and GL_ES define in preamble.
    std::string_view preamble = openGLES ? "#version 320 es\n#define GL_ES\n" : "#version 410 core\n";
    Blob preprocessedSource;
    try {
        static constexpr std::string_view glslDirectives[] = {"version", "extension"};
        preprocessedSource = pp::preprocess(source, pwd, preamble, glslDirectives);
    } catch (const std::exception &e) {
        logger->error("Could not preprocess shader '{}': {}", source.displayPath(), e.what());
        return 0;
    }

    // Compile shader.
    const char *sources[1] = {static_cast<const char *>(preprocessedSource.data())};
    const GLint lengths[1] = {static_cast<GLint>(preprocessedSource.size())};

    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, sources, lengths);
    glCompileShader(result);

    std::string errors = compileErrors(result);
    if (!errors.empty()) {
        logger->error("Could not compile shader '{}':\n{}", source.displayPath(), errors);
        glDeleteShader(result);
        return 0;
    }

    logger->info("Loaded shader '{}'.", source.displayPath());
    return result;
}
