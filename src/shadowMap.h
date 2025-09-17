#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"
#include "light.h"
#include "error.h"

class ShadowMap {
public:
    ShadowMap(unsigned int width = 1024, unsigned int height = 1024);
    ~ShadowMap();

    void bind();
    void unbind();
    
    // For rendering to the shadow map
    void bindForWriting();
    void bindForReading(GLenum textureUnit);
    
    // Calculate light space matrix for directional light
    glm::mat4 calculateLightSpaceMatrix(const Light& light, float nearPlane = 1.0f, float farPlane = 7.5f, float orthoSize = 10.0f);
    
    GLuint getDepthMap() const { return depthMap; }
    unsigned int getWidth() const { return shadowWidth; }
    unsigned int getHeight() const { return shadowHeight; }

private:
    GLuint depthMapFBO;
    GLuint depthMap;
    unsigned int shadowWidth, shadowHeight;
    
    void setupFramebuffer();
};

#endif // SHADOWMAP_H