// Class for loading in and building gl shaders for use
// shamelessly stolen from:
// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h

#pragma once

#include <string>

class OpenGLShader {
 public:
    unsigned int ID{};

    OpenGLShader() {
        ID = 0;
    }

    std::string sFilename{};

    // TODO(pskelton): consider map for uniform locations

    int build(std::string_view name, std::string_view filename, bool OpenGLES = false, bool reload = false);

    bool reload(std::string_view name, bool OpenGLES);

    // activate the shader
    void use();

 private:
    static std::string shaderTypeToExtension(int type);
    static std::string shaderTypeToName(int type);

    // utility function for checking shader compilation/linking errors.
    bool checkCompileErrors(int shader, std::string_view name, std::string_view type);

    int load(std::string_view name, std::string_view filename, int type, bool OpenGLES, bool nonFatal = false);
};
