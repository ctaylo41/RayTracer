#include "shadowManager.h"


ShadowManager::ShadowManager() {
}

ShadowManager::~ShadowManager() {
}

void ShadowManager::addShadowMap(size_t lightIndex, LightType lightType, unsigned int width, unsigned int height) {
    if (findShadowMap(lightIndex)) {
        std::cerr << "Shadow map for light index " << lightIndex << " already exists." << std::endl;
        return;
    }
    shadowMaps.emplace_back(lightIndex, lightType, width, height);
}

void ShadowManager::removeShadowMap(size_t lightIndex) {
    auto it = std::remove_if(shadowMaps.begin(), shadowMaps.end(),
        [lightIndex](const ShadowMapInfo& info) {
            return info.lightIndex == lightIndex;
        });
    if (it != shadowMaps.end()) {
        shadowMaps.erase(it, shadowMaps.end());
    }
}

void ShadowManager::clearAllShadowMaps() {
    shadowMaps.clear();
}

ShadowMapInfo* ShadowManager::findShadowMap(size_t lightIndex) {
    auto it = std::find_if(shadowMaps.begin(), shadowMaps.end(),
        [lightIndex](const ShadowMapInfo& info) {
            return info.lightIndex == lightIndex;
        });
    return (it != shadowMaps.end()) ? &(*it) : nullptr;
}

void ShadowManager::enableShadows(size_t lightIndex, bool enabled) {
    ShadowMapInfo* shadowMap = findShadowMap(lightIndex);
    if (shadowMap) {
        shadowMap->enabled = enabled;
    } else {
        std::cerr << "No shadow map found for light index " << lightIndex << std::endl;
    }
}

void ShadowManager::renderShadowMaps(const LightManager& lightManager, Scene& scene, Shader& shadowShader) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    shadowShader.activate();

    for(auto& shadowInfo : shadowMaps) {
        if(!shadowInfo.enabled || shadowInfo.lightIndex >= lightManager.getLightCount()) {
            continue;
        }

        const Light& light = lightManager.getLight(shadowInfo.lightIndex);
        if(!light.getProperties().enabled) {
            continue;
        }

        switch (light.getType()) {
            case LightType::Directional:
                renderDirectionalLightShadow(light, scene, shadowShader, shadowInfo);
                break;
            case LightType::Spot:
                renderSpotLightShadow(light, scene, shadowShader, shadowInfo);
                break;
            case LightType::Point:
                renderPointLightShadow(light, scene, shadowShader, shadowInfo);
                break;
        }
    }

    shadowShader.deactivate();

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void ShadowManager::renderDirectionalLightShadow(const Light& light, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowMapInfo) {
    shadowMapInfo.lightSpaceMatrix = shadowMapInfo.shadowBuffer->getLightSpaceMatrix(light, sceneCenter, sceneRadius);

    shadowMapInfo.shadowBuffer->bind();

    shadowShader.setMat4("lightSpaceMatrix", glm::value_ptr(shadowMapInfo.lightSpaceMatrix));

    for(const Model& model : scene.getModels()) {
        shadowShader.setMat4("model", glm::value_ptr(model.getModelMatrix()));

        const_cast<Model&>(model).draw(shadowShader, scene.getCamera());
    }

    shadowMapInfo.shadowBuffer->unbind();
}

void ShadowManager::renderSpotLightShadow(const Light& light, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowInfo) {
    shadowInfo.lightSpaceMatrix = shadowInfo.shadowBuffer->getSpotLightMatrix(light);

    shadowInfo.shadowBuffer->bind();

    shadowShader.setMat4("lightSpaceMatrix", glm::value_ptr(shadowInfo.lightSpaceMatrix));

    for(const Model& model : scene.getModels()) {
        shadowShader.setMat4("model", glm::value_ptr(model.getModelMatrix()));

        const_cast<Model&>(model).draw(shadowShader, scene.getCamera());
    }

    shadowInfo.shadowBuffer->unbind();
}

void ShadowManager::renderPointLightShadow(const Light& light, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowInfo)
{
    std::cout << "not implemented yets" << std::endl;
}

void ShadowManager::bindShadowMapsForRendering(Shader& shader) {
    shader.activate();
    
    // Set shadow parameters
    shader.setFloat("shadowBias", shadowBias);
    shader.setFloat("shadowSoftness", shadowSoftness);

    int textureUnit = 10; // Start shadow maps at texture unit 10
    int shadowMapCount = 0;
    
    for (const auto& shadowInfo : shadowMaps) {
        if (!shadowInfo.enabled || shadowMapCount >= 4) { // Limit to 4 shadow maps for now
            continue;
        }
        
        shadowInfo.shadowBuffer->bindTexture(textureUnit);
        
        std::string uniformBase = "shadowMaps[" + std::to_string(shadowMapCount) + "]";
        shader.setFloat((uniformBase + ".textureUnit").c_str(), static_cast<float>(textureUnit));
        shader.setFloat((uniformBase + ".lightIndex").c_str(), static_cast<float>(shadowInfo.lightIndex));
        shader.setMat4((uniformBase + ".lightSpaceMatrix").c_str(), glm::value_ptr(shadowInfo.lightSpaceMatrix));

        textureUnit++;
        shadowMapCount++;
    }

    shader.setFloat("numShadowMaps", static_cast<float>(shadowMapCount));
}

const char* getLightTypeName(LightType type) {
    switch (type) {
        case LightType::Directional: return "Directional";
        case LightType::Point: return "Point";
        case LightType::Spot: return "Spot";
        default: return "Unknown";
    }
}
