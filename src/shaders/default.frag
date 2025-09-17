#version 330 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Color;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;
in vec4 FragPosLightSpace; // Add this for shadow mapping

out vec4 fragColor;

// PBR texture uniforms
uniform sampler2D baseColorTexture;
uniform sampler2D normalTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D occlusionTexture;
uniform sampler2D emissiveTexture;

// Shadow mapping
uniform sampler2D shadowMap;
uniform bool enableShadows;

// Material factor uniforms
uniform float metallicFactor;
uniform float roughnessFactor;
uniform vec4 baseColorFactor;
uniform vec4 emissiveFactor;

// Alpha settings
uniform float alphaCutoff;
uniform bool useAlphaBlending;

// Camera position
uniform vec4 viewPos;

// Light structures
struct DirectionalLight {
    vec4 direction;
    vec4 color;
    bool enabled;
};

struct PointLight {
    vec4 position;
    vec4 color;
    vec4 attenuation;
    bool enabled;
};

struct SpotLight {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 attenuation;
    vec4 cutoff;
    bool enabled;
};

// Light uniforms
uniform float numDirectionalLights;
uniform float numPointLights;
uniform float numSpotLights;

uniform DirectionalLight directionalLights[4];
uniform PointLight pointLights[32];
uniform SpotLight spotLights[16];

// Shadow calculation function
float calculateShadow(vec4 fragPosLightSpace) {
    if (!enableShadows) return 0.0;
    
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if fragment is outside light's view frustum
    if (projCoords.z > 1.0) return 0.0;
    
    // Get current fragment depth
    float currentDepth = projCoords.z;
    
    // Sample shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Calculate bias to prevent shadow acne
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-directionalLights[0].direction.xyz);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    // Check if fragment is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    // PCF (Percentage Closer Filtering) for softer shadows
    shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

// Function to get proper normal (with normal mapping)
vec3 getNormalFromMap() {
    return normalize(Normal);
    vec4 normalMap = texture(normalTexture, TexCoord);
    
    // Check if normal map has actual data (not just default blue)
    if (length(normalMap.rgb - vec3(0.5, 0.5, 1.0)) < 0.01) {
        // No normal map data, use vertex normal
        return normalize(Normal);
    }
    
    // Convert from [0,1] to [-1,1] range
    vec3 tangentNormal = normalMap.rgb * 2.0 - 1.0;
    
    // Create TBN matrix to transform from tangent space to world space
    vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);
    
    // Gram-Schmidt process to ensure orthogonality
    T = normalize(T - dot(T, N) * N);
    B = cross(N, T);
    
    mat3 TBN = mat3(T, B, N);
    
    // Transform tangent space normal to world space
    return normalize(TBN * tangentNormal);
}

// Lighting functions with shadow support
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness, float shadow) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(-light.direction.xyz);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular (simplified)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (1.0 - roughness) * 128.0);
    
    // Combine
    vec3 diffuse = diff * baseColor.rgb * (1.0 - metallic);
    vec3 specular = spec * mix(vec3(0.04), baseColor.rgb, metallic);
    
    // Apply shadow (reduce diffuse and specular by shadow amount)
    return (diffuse + specular) * light.color.rgb * light.color.w * (1.0 - shadow);
}

vec3 calculatePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    float distance = length(light.position.xyz - fragPos);
    
    // Attenuation
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * distance * distance);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (1.0 - roughness) * 128.0);
    
    // Combine
    vec3 diffuse = diff * baseColor.rgb * (1.0 - metallic);
    vec3 specular = spec * mix(vec3(0.04), baseColor.rgb, metallic);
    
    return (diffuse + specular) * light.color.rgb * light.color.w * attenuation;
}

vec3 calculateSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    float distance = length(light.position.xyz - fragPos);
    
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.cutoff.x - light.cutoff.y;
    float intensity = clamp((theta - light.cutoff.y) / epsilon, 0.0, 1.0);
    
    // Attenuation
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * distance * distance);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (1.0 - roughness) * 128.0);
    
    // Combine
    vec3 diffuse = diff * baseColor.rgb * (1.0 - metallic);
    vec3 specular = spec * mix(vec3(0.04), baseColor.rgb, metallic);
    
    return (diffuse + specular) * light.color.rgb * light.color.w * attenuation * intensity;
}

void main() {
    // Sample base color texture
    vec4 baseColor = texture(baseColorTexture, TexCoord);
    
    // Apply base color factor
    baseColor.rgb *= baseColorFactor.rgb;
    
    // Use default color if base color is black
    if (baseColor.rgb == vec3(0.0, 0.0, 0.0)) {
        baseColor.rgb = vec3(0.2, 0.2, 0.2);
    }
    
    // Handle alpha testing/blending
    if (!useAlphaBlending && baseColor.a < alphaCutoff) {
        discard;
    }
    
    // Sample metallic-roughness texture
    vec4 metallicRoughness = texture(metallicRoughnessTexture, TexCoord);
    float metallic = metallicRoughness.b * metallicFactor;
    float roughness = metallicRoughness.g * roughnessFactor;
    
    // Sample ambient occlusion
    vec4 occlusion = texture(occlusionTexture, TexCoord);
    float aoFactor = (occlusion.r > 0.0) ? occlusion.r : 1.0;
    
    // Sample emissive texture
    vec4 emissive = texture(emissiveTexture, TexCoord);
    vec3 emissiveColor = emissive.rgb * emissiveFactor.rgb;
    
    // Get proper normal (with normal mapping)
    vec3 norm = getNormalFromMap();
    vec3 viewDir = normalize(viewPos.xyz - FragPos);
    
    // Calculate shadow
    float shadow = calculateShadow(FragPosLightSpace);
    
    // Start with ambient light
    vec3 ambient = 0.3 * baseColor.rgb;
    vec3 lighting = ambient;
    
    // Add directional lights (only first directional light casts shadows for now)
    for (int i = 0; i < int(numDirectionalLights) && i < 4; i++) {
        float lightShadow = (i == 0) ? shadow : 0.0; // Only first light casts shadows
        lighting += calculateDirectionalLight(directionalLights[i], norm, viewDir, baseColor.rgb, metallic, roughness, lightShadow);
    }
    
    // Add point lights (no shadows for now)
    for (int i = 0; i < int(numPointLights) && i < 32; i++) {
        lighting += calculatePointLight(pointLights[i], FragPos, norm, viewDir, baseColor.rgb, metallic, roughness);
    }
    
    // Add spot lights (no shadows for now)
    for (int i = 0; i < int(numSpotLights) && i < 16; i++) {
        lighting += calculateSpotLight(spotLights[i], FragPos, norm, viewDir, baseColor.rgb, metallic, roughness);
    }
    
    // Apply ambient occlusion
    lighting *= aoFactor;
    
    // Add emissive
    vec3 finalColor = lighting + emissiveColor;
    finalColor = max(finalColor, vec3(0.05));
    
    // Output final color
    fragColor = vec4(finalColor, baseColor.a);
}