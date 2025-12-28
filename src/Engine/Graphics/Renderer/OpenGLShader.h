#pragma once

#include <string>

#include "Utility/Memory/Blob.h"

class FileSystem;

/**
 * Class for loading in and building gl shaders.
 *
 * Code derived from:
 * https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h
 */
class OpenGLShader {
 public:
    OpenGLShader() = default;
    ~OpenGLShader();

    [[nodiscard]] bool isValid() const {
       return _id != 0;
    }

    [[nodiscard]] bool load(const Blob &vertSource, const Blob &fragSource, bool openGLES, const FileSystem *pwd);
    void release();

    [[nodiscard]] int uniformLocation(const char *name) const;
    // No attribLocation() - use vertex buffers.

    void use();
    void unuse();

 private:
    [[nodiscard]] unsigned loadShader(const Blob &source, int type, bool openGLES, const FileSystem *pwd);

 private:
    unsigned _id = 0;
    std::string _paths;
};
