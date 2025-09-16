#include "shadowManager.h"
#include "scene.h"

ShadowManager::ShadowManager() : shadowBias(0.005f), shadowSoftness(1.0f) {
    sceneCenter = glm::vec3(0.0f);
    sceneRadius = 50.0f;
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


void ShadowManager::bindShadowMapsForRendering(Shader& shader) {
    shader.activate();
    
    shader.setFloat("shadowBias", shadowBias);
    shader.setFloat("shadowSoftness", shadowSoftness);

    int shadowMapCount = 0;
    
    for (const auto& shadowInfo : shadowMaps) {
        if (!shadowInfo.enabled || shadowMapCount >= 4) {
            continue;
        }
        
        // Bind to texture units 10, 11, 12, 13 for shadow maps
        int textureUnit = 10 + shadowMapCount;
        shadowInfo.shadowBuffer->bindTexture(textureUnit);
        
        // Set the uniform for the specific shadow map sampler
        std::string samplerName = "shadowMap" + std::to_string(shadowMapCount);
        shader.setFloat(samplerName.c_str(), static_cast<float>(textureUnit));
        
        std::string uniformBase = "shadowMaps[" + std::to_string(shadowMapCount) + "]";
        shader.setFloat((uniformBase + ".textureUnit").c_str(), static_cast<float>(textureUnit));
        shader.setFloat((uniformBase + ".lightIndex").c_str(), static_cast<float>(shadowInfo.lightIndex));
        shader.setMat4((uniformBase + ".lightSpaceMatrix").c_str(), glm::value_ptr(shadowInfo.lightSpaceMatrix));

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


void ShadowManager::renderShadowMaps(const LightManager& lightManager, Scene& scene, Shader& shadowShader, const Camera& camera) {
    // Save current OpenGL state
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    std::cout << "Starting shadow map rendering with camera..." << std::endl;

    for(auto& shadowInfo : shadowMaps) {
        if(!shadowInfo.enabled || shadowInfo.lightIndex >= lightManager.getLightCount()) {
            continue;
        }

        const Light& light = lightManager.getLight(shadowInfo.lightIndex);
        if(!light.getProperties().enabled) {
            continue;
        }

        std::cout << "Rendering shadow for light " << shadowInfo.lightIndex << std::endl;

        shadowShader.activate();

        switch (light.getType()) {
            case LightType::Directional:
                renderDirectionalLightShadow(light, scene, shadowShader, shadowInfo, camera);
                break;
            case LightType::Spot:
                renderSpotLightShadow(light, scene, shadowShader, shadowInfo, camera);
                break;
            case LightType::Point:
                renderPointLightShadow(light, scene, shadowShader, shadowInfo, camera);
                break;
        }
        
        shadowShader.deactivate();
        
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after shadow light " << shadowInfo.lightIndex << ": " << error << std::endl;
        }
    }

    // Restore OpenGL state
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glUseProgram(currentProgram);
    
    std::cout << "Shadow map rendering complete." << std::endl;
}

void ShadowManager::renderDirectionalLightShadow(const Light& light, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowMapInfo, const Camera& camera) {
    std::cout << "=== Rendering Directional Light Shadow ===" << std::endl;
    
    // Calculate light space matrix using the camera
    shadowMapInfo.lightSpaceMatrix = shadowMapInfo.shadowBuffer->getLightSpaceMatrix(light, camera);
    
    // Bind shadow framebuffer
    shadowMapInfo.shadowBuffer->bind();
    
    // Clear and setup for shadow rendering
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT); // Reduce shadow acne
    
    // Set shadow shader uniforms
    shadowShader.setMat4("lightSpaceMatrix", glm::value_ptr(shadowMapInfo.lightSpaceMatrix));
    
    // Render each model for shadows
    int modelCount = 0;
    for(const Model& model : scene.getModels()) {
        shadowShader.setMat4("model", glm::value_ptr(model.getModelMatrix()));
        const_cast<Model&>(model).drawGeometryOnly();
        modelCount++;
    }
    
    std::cout << "Rendered " << modelCount << " models to shadow map" << std::endl;
    
    // Restore face culling
    glCullFace(GL_BACK);
    
    // Unbind shadow framebuffer
    shadowMapInfo.shadowBuffer->unbind();
}

void ShadowManager::renderSpotLightShadow(const Light& light, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowInfo, const Camera& camera) {
    // For spot lights, we can still use the existing method since it doesn't depend on scene bounds as much
    shadowInfo.lightSpaceMatrix = shadowInfo.shadowBuffer->getSpotLightMatrix(light);
    
    shadowInfo.shadowBuffer->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    
    shadowShader.setMat4("lightSpaceMatrix", glm::value_ptr(shadowInfo.lightSpaceMatrix));
    
    for(const Model& model : scene.getModels()) {
        shadowShader.setMat4("model", glm::value_ptr(model.getModelMatrix()));
        const_cast<Model&>(model).drawGeometryOnly();
    }
    
    glCullFace(GL_BACK);
    shadowInfo.shadowBuffer->unbind();
}

void ShadowManager::renderPointLightShadow(const Light&, Scene& scene, Shader& shadowShader, ShadowMapInfo& shadowInfo, const Camera& camera) {
    std::cout << "Point light shadows not implemented yet." << std::endl;
}
