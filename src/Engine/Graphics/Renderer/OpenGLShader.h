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

    int build(const std::string &name, const std::string &filename, bool OpenGLES = false, bool reload = false);

    bool reload(const std::string &name, bool OpenGLES);

    // activate the shader
    void use();

 private:
    static std::string shaderTypeToExtension(int type);
    static std::string shaderTypeToName(int type);

    // utility function for checking shader compilation/linking errors.
    bool checkCompileErrors(int shader, const std::string &name, const std::string &type);

    int load(const std::string &name, const std::string &filename, int type, bool OpenGLES, bool nonFatal = false);
};
