// Class for loading in and building gl shaders for use
// shamelessly stolen from:
// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "glad/gl.h"
#include <glm.hpp>

class GLShader {
 public:
    unsigned int ID;

    GLShader() {
        ID = 0;
    }

    // TODO(pskelton): save paths on construction for on the fly reloading
    // TODO(pskelton): consider map for uniform locations
    // TODO(pskelton): split into .h/.cpp?
    // TODO(pskelton): errors -> logger

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    GLShader(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr) {
        ID = 0;
        build(vertexPath, fragmentPath, geometryPath);
    }

    // or build to reconstruct
    bool build(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr) {
        ID = 0;
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            // if geometry shader path is present, also load a geometry shader
            if (geometryPath != nullptr) {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure &e) {
            logger->Warning("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ");
            return false;
        }

        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();

        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if (geometryPath != nullptr) {
            const char *gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }

        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (geometryPath != nullptr)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr)
            glDeleteShader(geometry);

        std::cout << "SHADER success" << std::endl;

        return ID != 0;
    }


    // activate the shader
    void use() {
        glUseProgram(ID);
    }



    //// utility uniform functions
    //// ------------------------------------------------------------------------
    //void setBool(const std::string &name, bool value) const {
    //    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    //}
    //// ------------------------------------------------------------------------
    //void setInt(const std::string &name, int value) const {
    //    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    //}
    //// ------------------------------------------------------------------------
    //void setFloat(const std::string &name, float value) const {
    //    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    //}
    // ------------------------------------------------------------------------
    //void setVec2(const std::string& name, const glm::vec2& value) const
    //{
    //    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    //}
    //void setVec2(const std::string& name, float x, float y) const
    //{
    //    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    //}
    //// ------------------------------------------------------------------------
    //void setVec3(const std::string& name, const glm::vec3& value) const
    //{
    //    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    //}
    //void setVec3(const std::string& name, float x, float y, float z) const
    //{
    //    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    //}
    //// ------------------------------------------------------------------------
    //void setVec4(const std::string& name, const glm::vec4& value) const
    //{
    //    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    //}
    //void setVec4(const std::string& name, float x, float y, float z, float w)
    //{
    //    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    //}
    //// ------------------------------------------------------------------------
    //void setMat2(const std::string& name, const glm::mat2& mat) const
    //{
    //    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    //}
    //// ------------------------------------------------------------------------
    //void setMat3(const std::string& name, const glm::mat3& mat) const
    //{
    //    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    //}
    //// ------------------------------------------------------------------------
    //void setMat4(const std::string& name, const glm::mat4& mat) const
    //{
    //    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    //}

 private:
    // utility function for checking shader compilation/linking errors.
    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                ID = 0;
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                ID = 0;
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif
