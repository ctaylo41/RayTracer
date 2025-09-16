// shadowManager.h - Updated interface
#ifndef SHADOW_MANAGER
#define SHADOW_MANAGER

#include "shadowBuffer.h"
#include "lightManager.h"
#include "shader.h"
#include <vector>
#include <memory>
#include "model.h"
#include "camera.h"

class Scene;

struct ShadowMapInfo {
    size_t lightIndex;
    LightType lightType;
    std::unique_ptr<ShadowBuffer> shadowBuffer;
    glm::mat4 lightSpaceMatrix;
    bool enabled;

    ShadowMapInfo(size_t idx, LightType type, unsigned int width = 2048, unsigned int height = 2048) 
        : lightIndex(idx), lightType(type), shadowBuffer(std::make_unique<ShadowBuffer>(width, height)), 
          lightSpaceMatrix(1.0f), enabled(true) {}
};

class ShadowManager {
public:
    ShadowManager();
    ~ShadowManager();

    void addShadowMap(size_t lightIndex, LightType lightType, unsigned int width = 2048, unsigned int height = 2048);
    void removeShadowMap(size_t lightIndex);
    void clearAllShadowMaps();

    // Updated to use camera
    void renderShadowMaps(const LightManager& lightManager, Scene& scene, Shader& shadowShader, const Camera& camera);

    void bindShadowMapsForRendering(Shader& mainShader);
    void enableShadows(size_t lightIndex, bool enabled);

    void setShadowBias(float bias) { shadowBias = bias; }
    void setShadowSoftness(float softness) { shadowSoftness = softness; }

    float getShadowBias() const { return shadowBias; }
    float getShadowSoftness() const { return shadowSoftness; }
    size_t getShadowMapCount() const { return shadowMaps.size(); }

    // Keep these for backward compatibility if needed
    void setSceneBounds(const glm::vec3& center, float radius) {
        sceneCenter = center;
        sceneRadius = radius;
    }

private:
    std::vector<ShadowMapInfo> shadowMaps;
    float shadowBias;
    float shadowSoftness;

    // Keep these as fallback
    glm::vec3 sceneCenter = glm::vec3(0.0f);
    float sceneRadius = 50.0f;

    ShadowMapInfo* findShadowMap(size_t lightIndex);
    
    // Updated to use camera
    void renderDirectionalLightShadow(const Light& light, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowMapInfo, const Camera& camera);
    void renderSpotLightShadow(const Light& light, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowInfo, const Camera& camera);
    void renderPointLightShadow(const Light& light, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowInfo, const Camera& camera);
};

#endif // SHADOW_MANAGER