#version 330 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Color;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;   // Need to add this
in vec3 Bitangent; // Need to add this

out vec4 fragColor;

// PBR texture uniforms
uniform sampler2D baseColorTexture;
uniform sampler2D normalTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D occlusionTexture;
uniform sampler2D emissiveTexture;

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

// Light structures (same as before)
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

// Lighting functions (same as before but using proper normal)
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness) {
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
    
    return (diffuse + specular) * light.color.rgb * light.color.w;
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
    
    // Start with ambient light
    vec3 ambient = 0.3 * baseColor.rgb;
    vec3 lighting = ambient;
    
    // Add directional lights
    for (int i = 0; i < int(numDirectionalLights) && i < 4; i++) {
        lighting += calculateDirectionalLight(directionalLights[i], norm, viewDir, baseColor.rgb, metallic, roughness);
    }
    
    // Add point lights
    for (int i = 0; i < int(numPointLights) && i < 32; i++) {
        lighting += calculatePointLight(pointLights[i], FragPos, norm, viewDir, baseColor.rgb, metallic, roughness);
    }
    
    // Add spot lights
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


    // Debug modes:
    // fragColor = vec4(norm * 0.5 + 0.5, 1.0);  // Visualize normal mapping
    // fragColor = vec4(baseColor.rgb, 1.0);

    // Debug metallic values (should be mostly black for non-metals)
    //fragColor = vec4(vec3(metallic), 1.0);

    // Debug roughness values
    //fragColor = vec4(vec3(roughness), 1.0);

    // Debug ambient occlusion
    //fragColor = vec4(vec3(aoFactor), 1.0);

    // Force minimum brightness to see geometry
    // vec3 finalColor = max(lighting + emissiveColor, vec3(0.05));
    // fragColor = vec4(finalColor, baseColor.a);
}