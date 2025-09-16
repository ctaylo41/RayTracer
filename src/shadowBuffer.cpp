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

std::vector<glm::vec4> ShadowBuffer::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view) {
    const auto inv = glm::inverse(proj * view);
    
    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x) {
        for (unsigned int y = 0; y < 2; ++y) {
            for (unsigned int z = 0; z < 2; ++z) {
                const glm::vec4 pt = inv * glm::vec4(
                    2.0f * x - 1.0f,
                    2.0f * y - 1.0f, 
                    2.0f * z - 1.0f,
                    1.0f
                );
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }
    return frustumCorners;
}

glm::mat4 ShadowBuffer::getLightSpaceMatrix(const Light& light, const Camera& camera) {
    if (light.getType() != LightType::Directional) {
        return glm::mat4(1.0f);
    }

    const LightProperties& props = light.getProperties();
    glm::vec3 lightDir = glm::normalize(props.direction);
    
    std::cout << "=== Camera-Based Shadow Setup ===" << std::endl;
    std::cout << "Light Direction: (" << lightDir.x << ", " << lightDir.y << ", " << lightDir.z << ")" << std::endl;
    
    // Get camera matrices
    glm::mat4 cameraView = camera.getViewMatrix();
    glm::mat4 cameraProj = camera.getProjectionMatrix();
    glm::vec3 cameraPos = camera.getPosition();
    
    std::cout << "Camera Position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    
    // Get camera frustum corners in world space
    auto frustumCorners = getFrustumCornersWorldSpace(cameraProj, cameraView);
    
    // Calculate the center of the frustum
    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : frustumCorners) {
        center += glm::vec3(v);
    }
    center /= frustumCorners.size();
    
    std::cout << "Frustum Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
    
    // Position the light to look at the frustum center
    // Use a reasonable distance based on frustum size
    float maxDistance = 0.0f;
    for (const auto& v : frustumCorners) {
        float dist = glm::length(glm::vec3(v) - center);
        maxDistance = std::max(maxDistance, dist);
    }
    
    float lightDistance = maxDistance * 2.0f; // Position light far enough back
    glm::vec3 lightPos = center - lightDir * lightDistance;
    
    std::cout << "Max frustum distance: " << maxDistance << std::endl;
    std::cout << "Light distance: " << lightDistance << std::endl;
    std::cout << "Light position: (" << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << ")" << std::endl;
    
    // Create up vector
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(lightDir, up)) > 0.95f) {
        up = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    // Create light view matrix
    glm::mat4 lightView = glm::lookAt(lightPos, center, up);
    
    // Transform frustum corners to light space to find bounds
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    
    for (const auto& v : frustumCorners) {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }
    
    // Extend the Z bounds to capture more of the scene
    float zMult = 10.0f;
    if (minZ < 0) {
        minZ *= zMult;
    } else {
        minZ /= zMult;
    }
    if (maxZ < 0) {
        maxZ /= zMult;
    } else {
        maxZ *= zMult;
    }
    
    std::cout << "Light space bounds:" << std::endl;
    std::cout << "  X: [" << minX << ", " << maxX << "]" << std::endl;
    std::cout << "  Y: [" << minY << ", " << maxY << "]" << std::endl;
    std::cout << "  Z: [" << minZ << ", " << maxZ << "]" << std::endl;
    
    // Create orthographic projection matrix
    glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    
    // Debug output
    std::cout << "=== Light Space Matrix ===" << std::endl;
    for(int i = 0; i < 4; i++) {
        std::cout << "[" << lightSpaceMatrix[i][0] << ", " << lightSpaceMatrix[i][1] 
                  << ", " << lightSpaceMatrix[i][2] << ", " << lightSpaceMatrix[i][3] << "]" << std::endl;
    }
    
    return lightSpaceMatrix;
}

glm::mat4 ShadowBuffer::getLightSpaceMatrixForBounds(const Light& light, const glm::vec3& sceneMin, const glm::vec3& sceneMax) {
    if (light.getType() != LightType::Directional) {
        return glm::mat4(1.0f);
    }

    const LightProperties& props = light.getProperties();
    glm::vec3 lightDir = glm::normalize(props.direction);
    
    std::cout << "=== Bounds-Based Shadow Setup ===" << std::endl;
    std::cout << "Scene Min: (" << sceneMin.x << ", " << sceneMin.y << ", " << sceneMin.z << ")" << std::endl;
    std::cout << "Scene Max: (" << sceneMax.x << ", " << sceneMax.y << ", " << sceneMax.z << ")" << std::endl;
    std::cout << "Light Direction: (" << lightDir.x << ", " << lightDir.y << ", " << lightDir.z << ")" << std::endl;
    
    // Calculate scene center and size
    glm::vec3 sceneCenter = (sceneMin + sceneMax) * 0.5f;
    glm::vec3 sceneSize = sceneMax - sceneMin;
    float sceneRadius = glm::length(sceneSize) * 0.5f;
    
    std::cout << "Scene Center: (" << sceneCenter.x << ", " << sceneCenter.y << ", " << sceneCenter.z << ")" << std::endl;
    std::cout << "Scene Size: (" << sceneSize.x << ", " << sceneSize.y << ", " << sceneSize.z << ")" << std::endl;
    std::cout << "Scene Radius: " << sceneRadius << std::endl;
    
    // Position light
    float lightDistance = sceneRadius * 2.0f;
    glm::vec3 lightPos = sceneCenter - lightDir * lightDistance;
    
    // Create up vector
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(lightDir, up)) > 0.95f) {
        up = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    // Create matrices
    glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, up);
    
    // Use scene bounds for orthographic projection
    float padding = sceneRadius * 0.1f; // 10% padding
    glm::mat4 lightProjection = glm::ortho(
        -sceneRadius - padding, sceneRadius + padding,
        -sceneRadius - padding, sceneRadius + padding,
        0.1f, lightDistance + sceneRadius + padding
    );
    
    return lightProjection * lightView;
}
