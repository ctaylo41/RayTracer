#include "lightManager.h"
#include "../../RayTracer/include/glm/gtc/type_ptr.hpp"

size_t MAX_DIRECTIONAL_LIGHTS = 4;
size_t MAX_POINT_LIGHTS = 32;
size_t MAX_SPOT_LIGHTS = 4;

LightManager::LightManager() {}
LightManager::~LightManager() {}

size_t LightManager::addLight(const Light &light)
{
    lights.push_back(light);
    return lights.size() - 1;
}

size_t LightManager::addDirectionalLight(const glm::vec3 &direction, const glm::vec3 &color, float intensity)
{
    LightProperties properties;
    properties.direction = glm::normalize(direction);
    properties.color = color;
    properties.intensity = intensity;
    return addLight(Light(LightType::Directional, properties));
}

size_t LightManager::addPointLight(const glm::vec3 &position, const glm::vec3 &color, float intensity)
{
    LightProperties properties;
    properties.position = position;
    properties.color = color;
    properties.intensity = intensity;
    return addLight(Light(LightType::Point, properties));
}

size_t LightManager::addSpotLight(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &color, float intensity, float innerCutoff, float outerCutoff)
{
    LightProperties properties;
    properties.position = position;
    properties.direction = glm::normalize(direction);
    properties.color = color;
    properties.intensity = intensity;
    properties.innerCutoff = innerCutoff;
    properties.outerCutoff = outerCutoff;
    return addLight(Light(LightType::Spot, properties));
}

void LightManager::removeLight(size_t index)
{
    if (index < lights.size())
    {
        lights.erase(lights.begin() + index);
    }
}

void LightManager::enableLight(size_t index, bool enabled)
{
    if (index < lights.size())
    {
        lights[index].getProperties().enabled = enabled;
    }
}

void LightManager::enableAllLights(bool enabled)
{
    for (auto &light : lights)
    {
        light.getProperties().enabled = enabled;
    }
}

std::vector<size_t> LightManager::getLightsByType(LightType type) const
{
    std::vector<size_t> indices;
    for (size_t i = 0; i < lights.size(); i++)
    {
        if (lights[i].getType() == type)
        {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> LightManager::getLightsInRange(const glm::vec3 &position, float radius) const
{
    std::vector<size_t> indices;
    for (size_t i = 0; i < lights.size(); i++)
    {
        const Light &light = lights[i];
        if (!light.getProperties().enabled)
            continue;

        if (light.getType() == LightType::Directional)
        {
            indices.push_back(i);
        }
        else
        {
            float distance = glm::distance(light.getProperties().position, position);
            float lightRange = light.calculateRange();
            if (distance <= radius + lightRange)
            {
                indices.push_back(i);
            }
        }
    }
    return indices;
}

void LightManager::updateShaderUniforms(Shader &shader) const
{
    uploadDirectionalLights(shader);
    uploadPointLights(shader);
    uploadSpotLights(shader);
}

void LightManager::uploadDirectionalLights(Shader &shader) const
{
    std::vector<size_t> directionalLights = getLightsByType(LightType::Directional);
    shader.setFloat("numDirectionalLights", static_cast<float>(directionalLights.size()));

    for (size_t i = 0; i < directionalLights.size() && i < MAX_DIRECTIONAL_LIGHTS; i++)
    {
        const Light &light = lights[directionalLights[i]];
        const LightProperties &properties = light.getProperties();

        std::string base = "directionalLights[" + std::to_string(i) + "]";

        shader.setVec4((base + ".direction"), glm::value_ptr(glm::vec4(properties.direction, 0.0f)));
        shader.setVec4((base + ".color"), glm::value_ptr(glm::vec4(properties.color, properties.intensity)));
        shader.setBool((base + ".enabled"), properties.enabled);
    }
}

void LightManager::uploadPointLights(Shader &shader) const
{
    std::vector<size_t> pointLights = getLightsByType(LightType::Point);
    shader.setFloat("numPointLights", static_cast<float>(pointLights.size()));

    for (size_t i = 0; i < pointLights.size() && i < MAX_POINT_LIGHTS; i++)
    {
        const Light &light = lights[pointLights[i]];
        const LightProperties &properties = light.getProperties();

        std::string base = "pointLights[" + std::to_string(i) + "]";

        shader.setVec4((base + ".position"), glm::value_ptr(glm::vec4(properties.position, 1.0f)));
        shader.setVec4((base + ".color"), glm::value_ptr(glm::vec4(properties.color, properties.intensity)));
        shader.setVec4((base + ".attenuation"), glm::value_ptr(glm::vec4(properties.constant, properties.linear, properties.quadratic, 0.0f)));
        shader.setBool((base + ".enabled"), properties.enabled);
    }
}

void LightManager::uploadSpotLights(Shader &shader) const
{
    auto spotLights = getLightsByType(LightType::Spot);

    shader.setFloat("numSpotLights", static_cast<float>(spotLights.size()));

    for (size_t i = 0; i < spotLights.size() && i < 16; ++i)
    { // Limit to 16 spot lights
        const Light &light = lights[spotLights[i]];
        const LightProperties &properties = light.getProperties();

        std::string base = "spotLights[" + std::to_string(i) + "]";

        shader.setVec4((base + ".position"), glm::value_ptr(glm::vec4(properties.position, 1.0f)));
        shader.setVec4((base + ".direction"), glm::value_ptr(glm::vec4(properties.direction, 0.0f)));
        shader.setVec4((base + ".color"), glm::value_ptr(glm::vec4(properties.color, properties.intensity)));
        shader.setVec4((base + ".attenuation"), glm::value_ptr(glm::vec4(properties.constant, properties.linear, properties.quadratic, 0.0f)));
        shader.setVec4((base + ".cutoff"), glm::value_ptr(glm::vec4(cos(glm::radians(properties.innerCutoff)), cos(glm::radians(properties.outerCutoff)), 0.0f, 0.0f)));
        shader.setBool((base + ".enabled"), properties.enabled);
    }
}

void LightManager::printLightInfo() const
{
    std::cout << "=== Light Manager Info ===" << std::endl;
    std::cout << "Total lights: " << lights.size() << std::endl;

    for (size_t i = 0; i < lights.size(); ++i)
    {
        std::cout << "Light " << i << ":\n"
                  << lights[i].getDebugInfo() << std::endl;
    }
}

LightStats LightManager::getStats() const
{
    LightStats stats;
    stats.totalLights = lights.size();

    float totalIntensity = 0.0f;

    for (const auto &light : lights)
    {
        if (light.getProperties().enabled)
        {
            stats.enabledLights++;
            totalIntensity += light.getProperties().intensity;
        }

        switch (light.getType())
        {
        case LightType::Directional:
            stats.directionalLights++;
            break;
        case LightType::Point:
            stats.pointLights++;
            break;
        case LightType::Spot:
            stats.spotLights++;
            break;
        }
    }

    if (stats.enabledLights > 0)
    {
        stats.averageIntensity = totalIntensity / stats.enabledLights;
    }

    return stats;
}

void LightManager::createStudioLighting(const glm::vec3 &center)
{
    removeAllLights();

    // Key light (main light)
    addDirectionalLight(
        glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)),
        glm::vec3(1.0f, 1.0f, 1.0f),
        2.0f);

    // Fill light (softer, opposite side)
    addDirectionalLight(
        glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f)),
        glm::vec3(0.8f, 0.9f, 1.0f),
        0.8f);

    // Rim light (from behind)
    addDirectionalLight(
        glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::vec3(1.0f, 0.9f, 0.8f),
        0.5f);
}

void LightManager::createNightLighting(const glm::vec3 &center)
{
    removeAllLights();

    // Moonlight
    addDirectionalLight(
        glm::normalize(glm::vec3(0.2f, -0.8f, 0.3f)),
        glm::vec3(0.6f, 0.7f, 1.0f),
        0.3f);

    // Ambient street lights
    addPointLight(center + glm::vec3(-10.0f, 5.0f, -10.0f), glm::vec3(1.0f, 0.8f, 0.4f), 8.0f);
    addPointLight(center + glm::vec3(10.0f, 5.0f, -10.0f), glm::vec3(1.0f, 0.8f, 0.4f), 8.0f);
    addPointLight(center + glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(1.0f, 0.8f, 0.4f), 8.0f);
}

void LightManager::createGoldenHourLighting()
{
    removeAllLights();

    // Low angle sun
    addDirectionalLight(
        glm::normalize(glm::vec3(0.8f, -0.3f, 0.5f)),
        glm::vec3(1.0f, 0.7f, 0.3f),
        1.5f);

    // Sky fill
    addDirectionalLight(
        glm::normalize(glm::vec3(-0.2f, -0.1f, -0.3f)),
        glm::vec3(0.3f, 0.5f, 0.8f),
        0.4f);
}

void LightManager::removeAllLights()
{
    lights.clear();
}
