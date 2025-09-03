#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include "shader.h"

enum class LightType {
    Directional,
    Point,
    Spot
};

struct LightProperties {
    bool enabled = true;
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    float innerCutoff = 12.5f;
    float outerCutoff = 17.5f;

    bool castsShadows = false;
    float range = 50.0f;
};

class Light {
public:
    Light(LightType type, const LightProperties& properties = LightProperties());
    LightType getType() const { return type; }
    const LightProperties& getProperties() const { return properties; }
    LightProperties& getProperties() { return properties; }
    void setPosition(const glm::vec3& pos) { properties.position = pos; }
    void setDirection(const glm::vec3& dir) { properties.direction = glm::normalize(dir); }
    void setColor(const glm::vec3& color) { properties.color = color; }
    void setIntensity(float intensity) { properties.intensity = intensity; }
    void setEnabled(bool enabled) { properties.enabled = enabled; }

    void setSpotAngles(float inner, float outer) {
        properties.innerCutoff = inner;
        properties.outerCutoff = outer;
    }

    void setAttenuation(float constant, float linear, float quadratic) {
        properties.constant = constant;
        properties.linear = linear;
        properties.quadratic = quadratic;
    }

    float calculateRange(float threshold = 0.01f) const;

    void transform(const glm::mat4& matrix);

    std::string getDebugInfo() const;

private:
    LightType type;
    LightProperties properties;
};
#endif // LIGHT_H