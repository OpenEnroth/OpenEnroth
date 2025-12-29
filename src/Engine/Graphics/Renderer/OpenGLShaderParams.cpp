#include "OpenGLShaderParams.h"

#include <string>

#include <glad/gl.h> // NOLINT: this is not a C system include.

#include "OpenGLShader.h"

void LineUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
}

void ForcePerUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    glUniform3f(shader.uniformLocation("fog.color"), fogColor.r, fogColor.g, fogColor.b);
    glUniform1f(shader.uniformLocation("fog.fogstart"), fogStart);
    glUniform1f(shader.uniformLocation("fog.fogmiddle"), fogMiddle);
    glUniform1f(shader.uniformLocation("fog.fogend"), fogEnd);
    glUniform1i(shader.uniformLocation("texture0"), texture0);
}

void TwoDUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    glUniform1i(shader.uniformLocation("paltex2D"), paltex2D);
    glUniform1i(shader.uniformLocation("texture0"), texture0);
}

void TextUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    glUniform1i(shader.uniformLocation("texture0"), texture0);
    glUniform1i(shader.uniformLocation("texture1"), texture1);
}

void DecalUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    // Fog color is optimised out in decal shader
    // glUniform3f(shader.uniformLocation("fog.color"), fogColor.r, fogColor.g, fogColor.b);
    glUniform1f(shader.uniformLocation("fog.fogstart"), fogStart);
    glUniform1f(shader.uniformLocation("fog.fogmiddle"), fogMiddle);
    glUniform1f(shader.uniformLocation("fog.fogend"), fogEnd);
    glUniform1f(shader.uniformLocation("decalbias"), decalBias);
    glUniform1i(shader.uniformLocation("texture0"), texture0);
}

void BillboardUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    glUniform3f(shader.uniformLocation("fog.color"), fogColor.r, fogColor.g, fogColor.b);
    glUniform1f(shader.uniformLocation("fog.fogstart"), fogStart);
    glUniform1f(shader.uniformLocation("fog.fogmiddle"), fogMiddle);
    glUniform1f(shader.uniformLocation("fog.fogend"), fogEnd);
    glUniform1f(shader.uniformLocation("gamma"), gamma);
    glUniform1i(shader.uniformLocation("paltex2D"), paltex2D);
    glUniform1i(shader.uniformLocation("texture0"), texture0);
}

static void submitSunLight(const OpenGLShader &shader, const SunLightUniform &sun) {
    glUniform3f(shader.uniformLocation("sun.direction"), sun.direction.x, sun.direction.y, sun.direction.z);
    glUniform3f(shader.uniformLocation("sun.ambient"), sun.ambient.r, sun.ambient.g, sun.ambient.b);
    glUniform3f(shader.uniformLocation("sun.diffuse"), sun.diffuse.r, sun.diffuse.g, sun.diffuse.b);
    glUniform3f(shader.uniformLocation("sun.specular"), sun.specular.r, sun.specular.g, sun.specular.b);
}

static void submitPointLight(const OpenGLShader &shader, const PointLightUniform &light, int index) {
    std::string prefix = "fspointlights[" + std::to_string(index) + "].";
    glUniform1f(shader.uniformLocation((prefix + "type").c_str()), light.type);
    glUniform3f(shader.uniformLocation((prefix + "position").c_str()), light.position.x, light.position.y, light.position.z);
    glUniform3f(shader.uniformLocation((prefix + "ambient").c_str()), light.ambient.r, light.ambient.g, light.ambient.b);
    glUniform3f(shader.uniformLocation((prefix + "diffuse").c_str()), light.diffuse.r, light.diffuse.g, light.diffuse.b);
    glUniform3f(shader.uniformLocation((prefix + "specular").c_str()), light.specular.r, light.specular.g, light.specular.b);
    glUniform1f(shader.uniformLocation((prefix + "radius").c_str()), light.radius);
}

template<size_t N>
static void submitPointLights(const OpenGLShader &shader, const std::array<PointLightUniform, N> &lights) {
    for (size_t i = 0; i < N; ++i)
        submitPointLight(shader, lights[i], static_cast<int>(i));
}

void TerrainUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    glUniform3f(shader.uniformLocation("CameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform3f(shader.uniformLocation("fog.color"), fogColor.r, fogColor.g, fogColor.b);
    glUniform1f(shader.uniformLocation("fog.fogstart"), fogStart);
    glUniform1f(shader.uniformLocation("fog.fogmiddle"), fogMiddle);
    glUniform1f(shader.uniformLocation("fog.fogend"), fogEnd);
    glUniform1f(shader.uniformLocation("gamma"), gamma);
    glUniform1i(shader.uniformLocation("waterframe"), waterframe);
    glUniform1i(shader.uniformLocation("textureArray0"), textureArray0);
    glUniform1i(shader.uniformLocation("textureArray1"), textureArray1);
    submitSunLight(shader, sun);
    submitPointLights(shader, pointLights);
}

void OutBuildUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    glUniform3f(shader.uniformLocation("CameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform3f(shader.uniformLocation("fog.color"), fogColor.r, fogColor.g, fogColor.b);
    glUniform1f(shader.uniformLocation("fog.fogstart"), fogStart);
    glUniform1f(shader.uniformLocation("fog.fogmiddle"), fogMiddle);
    glUniform1f(shader.uniformLocation("fog.fogend"), fogEnd);
    glUniform1f(shader.uniformLocation("gamma"), gamma);
    glUniform1i(shader.uniformLocation("waterframe"), waterframe);
    glUniform1i(shader.uniformLocation("flowtimer"), flowtimer);
    glUniform1i(shader.uniformLocation("flowtimerms"), flowtimerms);
    glUniform1i(shader.uniformLocation("textureArray0"), textureArray0);
    glUniform1i(shader.uniformLocation("watertiles"), watertiles);
    submitSunLight(shader, sun);
    submitPointLights(shader, pointLights);
}

void BSPUniforms::submit(const OpenGLShader &shader) const {
    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    glUniform3f(shader.uniformLocation("CameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform1f(shader.uniformLocation("gamma"), gamma);
    glUniform1i(shader.uniformLocation("waterframe"), waterframe);
    glUniform1i(shader.uniformLocation("flowtimer"), flowtimer);
    glUniform1i(shader.uniformLocation("flowtimerms"), flowtimerms);
    glUniform1i(shader.uniformLocation("textureArray0"), textureArray0);
    submitSunLight(shader, sun);
    submitPointLights(shader, pointLights);
}
