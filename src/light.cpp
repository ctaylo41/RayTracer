#include "light.h"
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

Light::Light(LightType type, const LightProperties& properties)
    : type(type), properties(properties) {
    switch(type) {
        case LightType::Directional:
            // Initialize directional light properties
            this->properties.direction = glm::normalize(this->properties.direction);
            this->properties.position = glm::vec3(0.0f); // Directional lights don't have a position
            break;
        case LightType::Point:
            // Initialize point light properties
            if (properties.constant == 0.0f && properties.linear == 0.0f && properties.quadratic == 0.0f) {
                this->properties.constant = 1.0f;
                this->properties.linear = 0.09f;
                this->properties.quadratic = 0.032f;
            }
            break;
        case LightType::Spot:
            // Initialize spot light properties
            this->properties.direction = glm::normalize(this->properties.direction);
            if(properties.constant == 0.0f && properties.linear == 0.0f && properties.quadratic == 0.0f) {
                this->properties.constant = 1.0f;
                this->properties.linear = 0.09f;
                this->properties.quadratic = 0.032f;
            }
            break;
    }
}

float Light::calculateRange(float threshold) const {
    if (type == LightType::Directional) {
        return std::numeric_limits<float>::infinity(); // Directional lights have infinite range
    }

    float a = properties.quadratic;
    float b = properties.linear;
    float c = properties.constant - (properties.intensity / threshold);

    if(a==0.0f) {
        if(b==0.0f) return properties.range;
        return std::max(0.0f, -c / b);
    }

    float discriminant = b * b - 4 * a * c;
    if(discriminant < 0) return properties.range;

    float range1 = (-b + std::sqrt(discriminant)) / (2 * a);
    float range2 = (-b - std::sqrt(discriminant)) / (2 * a);
    return std::max(0.0f, std::max(range1, range2));
}

void Light::transform(const glm::mat4& matrix) {
    if (type == LightType::Directional) {
        glm::vec4 pos = matrix * glm::vec4(properties.position, 1.0f);
        properties.position = glm::vec3(pos) / pos.w;
    } 

    if (type == LightType::Point || type == LightType::Spot) {
        glm::vec4 dir = matrix * glm::vec4(properties.direction, 0.0f);
        properties.direction = glm::normalize(glm::vec3(dir));
    }
}

std::string Light::getDebugInfo() const {
    std::stringstream ss;
    ss << "Light Type: ";
    switch (type) {
        case LightType::Directional:
            ss << "Directional";
            break;
        case LightType::Point:
            ss << "Point";
            break;
        case LightType::Spot:
            ss << "Spot";
            break;
    }
    ss << "\n";
    ss << "  Enabled: " << (properties.enabled ? "Yes" : "No") << "\n";
    ss << "  Color: (" << properties.color.r << ", " << properties.color.g << ", " << properties.color.b << ")\n";
    ss << "  Intensity: " << properties.intensity << "\n";
    
    if (type != LightType::Directional) {
        ss << "  Position: (" << properties.position.x << ", " << properties.position.y << ", " << properties.position.z << ")\n";
        ss << "  Attenuation: C=" << properties.constant << ", L=" << properties.linear << ", Q=" << properties.quadratic << "\n";
    }

    if (type == LightType::Directional || type == LightType::Spot) {
        ss << "  Direction: (" << properties.direction.x << ", " << properties.direction.y << ", " << properties.direction.z << ")\n";
    }

    if (type == LightType::Spot) {
        ss << "  Cutoff: Inner=" << properties.innerCutoff << "°, Outer=" << properties.outerCutoff << "°\n";
    }
    
    return ss.str();
}