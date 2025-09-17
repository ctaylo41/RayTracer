#include "shadowMap.h"
#include <iostream>

ShadowMap::ShadowMap(unsigned int width, unsigned int height) 
    : shadowWidth(width), shadowHeight(height), depthMapFBO(0), depthMap(0) {
    setupFramebuffer();
}

ShadowMap::~ShadowMap() {
    if (depthMap != 0) {
        glDeleteTextures(1, &depthMap);
    }
    if (depthMapFBO != 0) {
        glDeleteFramebuffers(1, &depthMapFBO);
    }
}

void ShadowMap::setupFramebuffer() {
    // Generate framebuffer
    glGenFramebuffers(1, &depthMapFBO);
    checkGLError("glGenFramebuffers");
    
    // Generate depth texture
    glGenTextures(1, &depthMap);
    checkGLError("glGenTextures");
    
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    // Set border color to white (outside shadow map area is not in shadow)
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    checkGLError("glTexParameterfv");
    
    // Bind framebuffer and attach depth texture
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    
    // Tell OpenGL we don't need color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Shadow map framebuffer not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError("setupFramebuffer complete");
}

void ShadowMap::bind() {
    bindForWriting();
}

void ShadowMap::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::bindForWriting() {
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glViewport(0, 0, shadowWidth, shadowHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::bindForReading(GLenum textureUnit) {
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, depthMap);
}

glm::mat4 ShadowMap::calculateLightSpaceMatrix(const Light& light, float nearPlane, float farPlane, float orthoSize) {
    if (light.getType() != LightType::Directional) {
        std::cerr << "Warning: Shadow mapping currently only supports directional lights!" << std::endl;
        return glm::mat4(1.0f);
    }
    
    const LightProperties& props = light.getProperties();
    
    // Create light view matrix - position light far away in opposite direction
    glm::vec3 lightPos = -props.direction * farPlane * 0.5f;
    glm::vec3 lightTarget = glm::vec3(0.0f); // Look at scene center
    glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // If light direction is too close to up vector, use different up vector
    if (abs(glm::dot(glm::normalize(props.direction), lightUp)) > 0.95f) {
        lightUp = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, lightUp);
    
    // Create orthographic projection for directional light
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
    
    return lightProjection * lightView;
}