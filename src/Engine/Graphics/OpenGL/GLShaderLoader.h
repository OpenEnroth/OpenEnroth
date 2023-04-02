// Class for loading in and building gl shaders for use
// shamelessly stolen from:
// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include "glad/gl.h"
#include <glm.hpp>

#include "Engine/EngineIocContainer.h"

#include "Library/Logger/Logger.h"

#include "Utility/DataPath.h"

class GLShader {
 public:
    unsigned int ID{};

    GLShader() {
        ID = 0;
    }

    std::string sFilename{};

    // TODO(pskelton): consider map for uniform locations
    // TODO(pskelton): split into .h/.cpp?

    int build(const std::string &name, const std::string &filename, bool OpenGLES = false, bool reload = false) {
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

        logger->warning("shader compilation failure: {}", filename);
        return 0;
    }

    bool reload(const std::string &name, bool OpenGLES) {
        int tryreload = build(name, sFilename, OpenGLES, true);

        if (tryreload) {
            glDeleteProgram(ID);
            ID = tryreload;
            return true;
        }

        logger->info("shader reload failed, reverting...");
        return false;
    }

    // activate the shader
    void use() {
        glUseProgram(ID);
    }

 private:
    static std::string shaderTypeToExtension(GLenum type);
    static std::string shaderTypeToName(GLenum type);

    // utility function for checking shader compilation/linking errors.
    bool checkCompileErrors(GLuint shader, const std::string &name, const std::string &type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "program") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                logger->warning("{} {} shader compilation error:", name, type);
                logger->warning("{}", infoLog);
                return false;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                logger->warning("{} {} linking error:", name, type);
                logger->warning("{}", infoLog);
                return false;
            }
        }
        return true;
    }

    GLuint load(const std::string &name, const std::string &filename, GLenum type, bool OpenGLES, bool nonFatal = false) {
        std::string directory = "shaders";
        std::string typeName = shaderTypeToName(type);
        std::string path = MakeDataPath(directory, filename + "." + shaderTypeToExtension(type));

        try {
            // open files
            if (!std::filesystem::exists(path)) {
                if (!nonFatal)
                    logger->warning("cannot find {} {} shader file at path {}", name, typeName, path);

                return 0;
            }

            std::ifstream shaderFile;
            shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            shaderFile.open(path);

            std::stringstream shaderStream;
            if (!OpenGLES)
                shaderStream << "#version 410 core" << std::endl;
            else
                shaderStream << "#version 320 es" << std::endl;

            // read file's buffer contents into stream
            shaderStream << shaderFile.rdbuf();

            // close file handler
            shaderFile.close();

            // compile shader
            std::string shaderString = shaderStream.str();
            const char *shaderChar = shaderString.c_str();
            GLuint shaderHandler = glCreateShader(type);
            glShaderSource(shaderHandler, 1, &shaderChar, NULL);
            glCompileShader(shaderHandler);
            checkCompileErrors(shaderHandler, name, shaderTypeToName(type));

            return shaderHandler;
        }
        catch (std::ifstream::failure &e) {
            logger->warning("error occured during reading {} {} shader file at path {}: {}", name, typeName, path, e.what());
            return 0;
        }
    }
};
