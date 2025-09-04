#ifndef SHADOW_BUFFER_H
#define SHADOW_BUFFER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "shader.h"
#include "light.h"

class ShadowBuffer {
public:
    ShadowBuffer(unsigned int width = 2048, unsigned int height = 2048);
    ~ShadowBuffer();

    ShadowBuffer(const ShadowBuffer&) = delete;
    ShadowBuffer& operator=(const ShadowBuffer&) = delete;

    void bind();
    void unbind();
    void bindTexture(unsigned int unit);

    glm::mat4 getLightSpaceMatrix(const Light& light, const glm::vec3& sceneCenter, float sceneRadius);

    std::vector<glm::mat4> getPointLightMatrices(const Light& light, float nearPlane = 0.1f, float farPlane = 100.0f);

    glm::mat4 getSpotLightMatrix(const Light& light, float nearPlane = 0.1f, float farPlane = 100.0f);

    GLuint getDepthMap() const { return depthMap; }
    GLuint getFramebuffer() const { return framebuffer; }
    unsigned int getWidth() const { return shadowWidth; }
    unsigned int getHeight() const { return shadowHeight; }

private:
    unsigned int shadowWidth;
    unsigned int shadowHeight;
    GLuint framebuffer;
    GLuint depthMap;

    void initializeFramebuffer();
};

#endif