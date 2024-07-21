#pragma once

#include <string>

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

    [[nodiscard]] bool load(std::string_view vertPath, std::string_view fragPath, bool openGLES);
    void release();

    [[nodiscard]] int uniformLocation(const char *name);
    [[nodiscard]] int attribLocation(const char *name);

    void use();

 private:
    [[nodiscard]] unsigned loadShader(std::string_view path, int type, bool openGLES);

 private:
    unsigned _id = 0;
};
