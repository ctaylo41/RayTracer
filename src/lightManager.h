#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include "light.h"
#include <vector>
#include <memory>

struct LightStats {
        int totalLights = 0;
        int enabledLights = 0;
        int directionalLights = 0;
        int pointLights = 0;
        int spotLights = 0;
        float averageIntensity = 0.0f;
};

class LightManager {
public:
    LightManager();
    ~LightManager();


    size_t addLight(const Light& light);
    size_t addDirectionalLight(const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f);
    size_t addPointLight(const glm::vec3& position, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f);
    size_t addSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f, float innerCutoff = 12.5f, float outerCutoff = 17.5f);

    Light& getLight(size_t index) { return lights[index]; }
    const Light& getLight(size_t index) const { return lights[index]; }
    size_t getLightCount() const { return lights.size(); }
    std::vector<Light>& getLights() { return lights; }
    const std::vector<Light>& getLights() const { return lights; }

    void removeLight(size_t index);
    void removeAllLights();

    void enableLight(size_t index, bool enabled = true);
    void enableAllLights(bool enabled = true);

    void uploadToShader(Shader& shader) const;
    void updateShaderUniforms(Shader& shader) const;

    std::vector<size_t> getLightsByType(LightType type) const;

    std::vector<size_t> getLightsInRange(const glm::vec3& position, float radius = 0.0f) const;

    void printLightInfo() const;
    LightStats getStats() const;
    void createStudioLighting(const glm::vec3& center = glm::vec3(0.0f));
    void createNightLighting(const glm::vec3& center = glm::vec3(0.0f));
    void createGoldenHourLighting();
private:
    std::vector<Light> lights;

    void uploadDirectionalLights(Shader& shader) const;
    void uploadPointLights(Shader& shader) const;
    void uploadSpotLights(Shader& shader) const;
};

#endif // LIGHT_MANAGER_H
