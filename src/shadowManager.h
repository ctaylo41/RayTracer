#ifndef SHADOWMANAGER_H
#define SHADOWMANAGER_H

#include "shadowMap.h"
#include "shader.h"
#include "lightManager.h"
#include "scene.h"
#include <memory>

class ShadowManager {
public:
    ShadowManager(unsigned int shadowMapWidth = 1024, unsigned int shadowMapHeight = 1024);
    ~ShadowManager();

    // Initialize shadow system
    void initialize();
    
    // Render shadows for the scene
    void renderShadows(Scene& scene, LightManager& lightManager);
    
    // Bind shadow map for reading in main render pass
    void bindShadowMapForReading(GLenum textureUnit);
    
    // Get light space matrix for the current shadow-casting light
    glm::mat4 getLightSpaceMatrix() const { return lightSpaceMatrix; }
    
    // Enable/disable shadows
    void setShadowsEnabled(bool enabled) { shadowsEnabled = enabled; }
    bool getShadowsEnabled() const { return shadowsEnabled; }
    
    // Get shadow map for debugging
    ShadowMap& getShadowMap() { return *shadowMap; }

private:
    std::unique_ptr<ShadowMap> shadowMap;
    std::unique_ptr<Shader> depthShader;
    glm::mat4 lightSpaceMatrix;
    bool shadowsEnabled;
    bool initialized;
    
    void setupDepthShader();
};

#endif // SHADOWMANAGER_H