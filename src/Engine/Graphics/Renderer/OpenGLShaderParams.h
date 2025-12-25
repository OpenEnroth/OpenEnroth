#pragma once

#include <array>

#include <glm/glm.hpp>

#include "Library/Color/Colorf.h"
#include "Library/Geometry/Vec.h"

class OpenGLShader;

struct LineVertex {
    Vec2f pos;
    Colorf color;
};

struct LineUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};

    void submit(const OpenGLShader &shader) const;
};

struct ForcePerVertex {
    Vec3f pos;
    float w = 0;
    Vec2f texuv;
    float texw = 0;
    float screenspace = 0;
    Colorf color;
    float texid = 0;
};

struct ForcePerUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};
    Colorf fogColor;
    float fogStart = 0;
    float fogMiddle = 0;
    float fogEnd = 0;
    int texture0 = 0;

    void submit(const OpenGLShader &shader) const;
};

struct TwoDVertex {
    Vec3f pos;
    Vec2f texuv;
    Colorf color;
    float texid = 0;
    float paletteid = 0;
};

struct TwoDUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};
    int paltex2D = 1;
    int texture0 = 0;

    void submit(const OpenGLShader &shader) const;
};

struct TextUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};
    int texture0 = 0;
    int texture1 = 1;

    void submit(const OpenGLShader &shader) const;
};

struct DecalVertex {
    Vec3f pos;
    Vec2f texuv;
    float texunit = 0;
    Colorf color;
};

struct DecalUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};
    Colorf fogColor;
    float fogStart = 0;
    float fogMiddle = 0;
    float fogEnd = 0;
    float decalBias = 0.002f;
    int texture0 = 0;

    void submit(const OpenGLShader &shader) const;
};

struct BillboardVertex {
    Vec3f pos;
    Vec2f texuv;
    Colorf color;
    float screenspace = 0;
    float texid = 0;
    float blend = 0;
    float paletteId = 0;
};

struct BillboardUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};
    Colorf fogColor;
    float fogStart = 0;
    float fogMiddle = 0;
    float fogEnd = 0;
    float gamma = 0;
    int paltex2D = 1;
    int texture0 = 0;

    void submit(const OpenGLShader &shader) const;
};

struct ShaderVertex {
    Vec3f pos;
    Vec2f texuv;
    float texturelayer = 0;
    Vec3f normal;
    float attribs = 0;
};

struct PointLightUniform {
    float type = 0; // 0 = unused, 1 = stationary, 2 = mobile
    Vec3f position;
    Colorf ambient;
    Colorf diffuse;
    Colorf specular;
    float radius = 0;
};

struct SunLightUniform {
    Vec3f direction;
    Colorf ambient;
    Colorf diffuse;
    Colorf specular;
};

struct TerrainUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};
    Vec3f cameraPos;
    Colorf fogColor;
    float fogStart = 0;
    float fogMiddle = 0;
    float fogEnd = 0;
    float gamma = 0;
    int waterframe = 0;
    int textureArray0 = 0;
    int textureArray1 = 1;

    SunLightUniform sun;
    std::array<PointLightUniform, 20> pointLights;

    void submit(const OpenGLShader &shader) const;
};

struct OutBuildUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};
    Vec3f cameraPos;
    Colorf fogColor;
    float fogStart = 0;
    float fogMiddle = 0;
    float fogEnd = 0;
    float gamma = 0;
    int waterframe = 0;
    int flowtimer = 0;
    int flowtimerms = 0;
    int textureArray0 = 0;
    int watertiles = 1;

    SunLightUniform sun;
    std::array<PointLightUniform, 20> pointLights;

    void submit(const OpenGLShader &shader) const;
};

struct BSPUniforms {
    glm::mat4 projection = {};
    glm::mat4 view = {};
    Vec3f cameraPos;
    float gamma = 0;
    int waterframe = 0;
    int flowtimer = 0;
    int flowtimerms = 0;
    int textureArray0 = 0;
    int textureArray1 = 1;
    int textureArray2 = 2;

    SunLightUniform sun;
    std::array<PointLightUniform, 40> pointLights;

    void submit(const OpenGLShader &shader) const;
};
