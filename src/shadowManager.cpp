#include "shadowManager.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

ShadowManager::ShadowManager(unsigned int shadowMapWidth, unsigned int shadowMapHeight)
    : shadowsEnabled(true), initialized(false), lightSpaceMatrix(1.0f) {
    shadowMap = std::make_unique<ShadowMap>(shadowMapWidth, shadowMapHeight);
}

ShadowManager::~ShadowManager() {
    // Smart pointers will automatically clean up
}

void ShadowManager::initialize() {
    if (initialized) return;
    
    setupDepthShader();
    initialized = true;
    
    std::cout << "Shadow Manager initialized successfully" << std::endl;
}

void ShadowManager::setupDepthShader() {
    try {
        // You'll need to create these shader files in your shaders directory
        depthShader = std::make_unique<Shader>(
            "/Users/colintaylortaylor/Documents/raytracer/src/shaders/shadow.vert",
            "/Users/colintaylortaylor/Documents/raytracer/src/shaders/shadow.frag"
        );
        std::cout << "Depth shader loaded successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load depth shader: " << e.what() << std::endl;
        shadowsEnabled = false;
    }
}

void ShadowManager::renderShadows(Scene& scene, LightManager& lightManager) {
    if (!shadowsEnabled || !initialized || !depthShader) {
        return;
    }
    
    // Find the first enabled directional light to cast shadows
    const auto& lights = lightManager.getLights();
    const Light* shadowCastingLight = nullptr;
    
    for (const auto& light : lights) {
        if (light.getType() == LightType::Directional && 
            light.getProperties().enabled && 
            light.getProperties().castsShadows) {
            shadowCastingLight = &light;
            break;
        }
    }
    
    if (!shadowCastingLight) {
        return; // No shadow-casting directional light found
    }
    
    // Calculate light space matrix
    lightSpaceMatrix = shadowMap->calculateLightSpaceMatrix(*shadowCastingLight);
    
    // Store current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // 1. Render scene to shadow map
    shadowMap->bindForWriting();
    
    // Configure OpenGL for depth rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Render only back faces to reduce shadow acne
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    depthShader->activate();
    depthShader->setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
    
    // Use Scene's drawDepthOnly method for cleaner rendering
    scene.drawDepthOnly(*depthShader, lightSpaceMatrix);
    
    depthShader->deactivate();
    
    // Restore face culling
    glCullFace(GL_BACK);
    
    shadowMap->unbind();
    
    // Restore viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    
    checkGLError("renderShadows complete");
}

void ShadowManager::bindShadowMapForReading(GLenum textureUnit) {
    if (!shadowsEnabled || !initialized) {
        return;
    }
    
    shadowMap->bindForReading(textureUnit);
}