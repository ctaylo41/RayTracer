#include "shadowBuffer.h"
#include "error.h"
#include <iostream>

ShadowBuffer::ShadowBuffer(unsigned int width, unsigned int height)
    : shadowWidth(width), shadowHeight(height), framebuffer(0), depthMap(0) {
    initializeFramebuffer();
}

ShadowBuffer::~ShadowBuffer() {
    if (depthMap != 0) {
        glDeleteTextures(1, &depthMap);
    }
    if (framebuffer != 0) {
        glDeleteFramebuffers(1, &framebuffer);
    }
}

void ShadowBuffer::initializeFramebuffer() {
    glGenFramebuffers(1, &framebuffer);
    checkGLError("generate shadow framebuffer");

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    checkGLError("Create shadow depth texture");

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadow framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError("Unbind shadow framebuffer");
}

void ShadowBuffer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, shadowWidth, shadowHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // DEBUG: Verify settings
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    std::cout << "Shadow viewport: " << viewport[2] << "x" << viewport[3] << std::endl;

}

void ShadowBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowBuffer::bindTexture(unsigned int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, depthMap);
}

glm::mat4 ShadowBuffer::getLightSpaceMatrix(const Light& light, const glm::vec3& sceneCenter, float sceneRadius) {
    if(light.getType() != LightType::Directional) {
        return glm::mat4(1.0f);
    }

    const LightProperties& props = light.getProperties();
    glm::vec3 lightDir = glm::normalize(props.direction);
if (glm::length(lightDir) < 0.001f) lightDir = glm::vec3(-1, -1, -1); // fallback

glm::vec3 up = glm::abs(lightDir.y) > 0.99f ? glm::vec3(0,0,1) : glm::vec3(0,1,0);

float lightDistance = sceneRadius * 2.0f;
glm::vec3 lightPos = sceneCenter - lightDir * lightDistance;

glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, up);

float orthoSize = sceneRadius * 1.2f;
float nearPlane = 0.1f;
float farPlane = lightDistance + sceneRadius;

glm::mat4 lightProjection = glm::ortho(
    -orthoSize, orthoSize,
    -orthoSize, orthoSize,
    nearPlane, farPlane
);

glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    return lightSpaceMatrix;


    // if(light.getType() != LightType::Directional) {
    //     return glm::mat4(1.0f);
    // }

    // const LightProperties& props = light.getProperties();
    // glm::vec3 lightDir = glm::normalize(props.direction);
    
    // // Position the light much further back
    // glm::vec3 lightPos = sceneCenter - lightDir * (sceneRadius * 4.0f);
    
    // // Create a proper up vector
    // glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    // glm::vec3 up = worldUp;
    
    // // If light direction is too close to up vector, use a different up
    // if (glm::abs(glm::dot(glm::normalize(lightDir), worldUp)) > 0.95f) {
    //     up = glm::vec3(0.0f, 0.0f, 1.0f);
    // }
    
    // // Create view matrix
    // glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, up);
    
    // // Create projection matrix with proper bounds
    // float left = -sceneRadius * 2.0f;
    // float right = sceneRadius * 2.0f;
    // float bottom = -sceneRadius * 2.0f;
    // float top = sceneRadius * 2.0f;
    // float nearPlane = 1.0f;
    // float farPlane = sceneRadius * 8.0f;
    
    // glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    
    // return lightProjection * lightView;
}



glm::mat4 ShadowBuffer::getSpotLightMatrix(const Light& light, float nearPlane, float farPlane) {
    if(light.getType() != LightType::Spot) {
        std::cerr << "getSpotLightMatrix is only valid for spot lights." << std::endl;
        return glm::mat4(1.0f);
    }

    const LightProperties& props = light.getProperties();

    float fov = glm::radians(props.outerCutoff * 2.0f);
    glm::mat4 lightProjection = glm::perspective(fov, 1.0f, nearPlane, farPlane);

    glm::vec3 target = props.position + props.direction;
    glm::mat4 lightView = glm::lookAt(props.position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    return lightProjection * lightView;
}

std::vector<glm::mat4> ShadowBuffer::getPointLightMatrices(const Light& light, float nearPlane, float farPlane) {
    if(light.getType() != LightType::Point) {
        std::cerr << "getPointLightMatrices is only valid for point lights." << std::endl;
        return {};
    }

    const LightProperties& props = light.getProperties();
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(props.position, props.position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(props.position, props.position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(props.position, props.position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(props.position, props.position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(props.position, props.position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(props.position, props.position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

    return shadowTransforms;
}
