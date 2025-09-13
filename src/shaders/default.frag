#version 330 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Color;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;

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
uniform vec3 cameraPos;

// Shadow mapping uniforms
struct ShadowMap {
    float textureUnit;
    float lightIndex;
    mat4 lightSpaceMatrix;
};

uniform ShadowMap shadowMaps[4];
uniform float numShadowMaps;
uniform float shadowBias;
uniform float shadowSoftness;

uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;
uniform sampler2D shadowMap3;

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

// Function to get proper normal (with normal mapping)
vec3 getNormalFromMap() {
    vec4 normalMap = texture(normalTexture, TexCoord);
    
    if (length(normalMap.rgb) < 0.01) {
        return normalize(Normal);
    }
    
    vec3 tangentNormal = normalMap.rgb * 2.0 - 1.0;
    
    vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);
    
    T = normalize(T - dot(T, N) * N);
    B = cross(N, T);
    
    mat3 TBN = mat3(T, B, N);
    
    return normalize(TBN * tangentNormal);
}

// Shadow calculation function - now takes sampler2D as parameter
float calculateShadow(vec4 fragPosLightSpace, sampler2D shadowMapTexture, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if we're outside the shadow map bounds
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0 || 
       projCoords.z > 1.0) {
        return 0.0; // No shadow if outside bounds
    }
    
    // Sample the shadow map
    float closestDepth = texture(shadowMapTexture, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    // Simple shadow test with minimal bias
    return currentDepth > closestDepth + 0.001 ? 0.7 : 0.0;

    // Perform perspective divide
    // vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // // Transform to [0,1] range
    // projCoords = projCoords * 0.5 + 0.5;
    
    // // Get closest depth value from light's perspective
    // float closestDepth = texture(shadowMapTexture, projCoords.xy).r;
    
    // // Get depth of current fragment from light's perspective
    // float currentDepth = projCoords.z;
    
    // // Calculate bias to prevent shadow acne
    // float bias = max(shadowBias * (1.0 - dot(normal, lightDir)), shadowBias * 0.1);
    
    // // Check whether current frag pos is in shadow
    // // PCF (Percentage Closer Filtering) for soft shadows
    // float shadow = 0.0;
    // vec2 texelSize = 1.0 / textureSize(shadowMapTexture, 0);
    // int pcfSamples = int(shadowSoftness);
    
    // for(int x = -pcfSamples; x <= pcfSamples; ++x) {
    //     for(int y = -pcfSamples; y <= pcfSamples; ++y) {
    //         float pcfDepth = texture(shadowMapTexture, projCoords.xy + vec2(x, y) * texelSize).r;
    //         shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    //     }
    // }
    
    // shadow /= pow(2.0 * pcfSamples + 1.0, 2.0);
    
    // // Keep shadow at 0.0 when outside the far plane region of the light's frustum
    // if(projCoords.z > 1.0)
    //     shadow = 0.0;
    
    // return shadow;
}

// Get shadow factor for a light - fixed to pass correct sampler
float getShadowFactor(int lightIndex, vec3 normal, vec3 lightDir) {
    for(int i = 0; i < int(numShadowMaps) && i < 4; i++) {
        if(int(shadowMaps[i].lightIndex) == lightIndex) {
            vec4 fragPosLightSpace = shadowMaps[i].lightSpaceMatrix * vec4(FragPos, 1.0);
            
            // Use the correct shadow map based on index
            if(i == 0) return calculateShadow(fragPosLightSpace, shadowMap0, normal, lightDir);
            else if(i == 1) return calculateShadow(fragPosLightSpace, shadowMap1, normal, lightDir);
            else if(i == 2) return calculateShadow(fragPosLightSpace, shadowMap2, normal, lightDir);
            else if(i == 3) return calculateShadow(fragPosLightSpace, shadowMap3, normal, lightDir);
        }
    }
    return 0.0; // No shadow map found for this light
}

// Updated lighting functions with shadow support
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness, int lightIndex) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(-light.direction.xyz);
    
    // Calculate shadow
    float shadow = getShadowFactor(lightIndex, normal, lightDir);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (1.0 - roughness) * 128.0);
    
    // Combine
    vec3 diffuse = diff * baseColor.rgb * (1.0 - metallic);
    vec3 specular = spec * mix(vec3(0.04), baseColor.rgb, metallic);
    
    vec3 lighting = (diffuse + specular) * light.color.rgb * light.color.w;
    
    // Apply shadow (keep some ambient)
    return lighting * (1.0 - shadow * 0.8);
}

vec3 calculatePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness, int lightIndex) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    float distance = length(light.position.xyz - fragPos);
    
    // Attenuation
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * distance * distance);
    
    // Calculate shadow (point lights would need cube shadow mapping - simplified for now)
    float shadow = 0.0; // getShadowFactor(lightIndex, normal, lightDir);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (1.0 - roughness) * 128.0);
    
    // Combine
    vec3 diffuse = diff * baseColor.rgb * (1.0 - metallic);
    vec3 specular = spec * mix(vec3(0.04), baseColor.rgb, metallic);
    
    vec3 lighting = (diffuse + specular) * light.color.rgb * light.color.w * attenuation;
    
    return lighting * (1.0 - shadow * 0.8);
}

vec3 calculateSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness, int lightIndex) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    float distance = length(light.position.xyz - fragPos);
    
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.cutoff.x - light.cutoff.y;
    float intensity = clamp((theta - light.cutoff.y) / epsilon, 0.0, 1.0);
    
    // Attenuation
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * distance * distance);
    
    // Calculate shadow
    float shadow = getShadowFactor(lightIndex, normal, lightDir);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (1.0 - roughness) * 128.0);
    
    // Combine
    vec3 diffuse = diff * baseColor.rgb * (1.0 - metallic);
    vec3 specular = spec * mix(vec3(0.04), baseColor.rgb, metallic);
    
    vec3 lighting = (diffuse + specular) * light.color.rgb * light.color.w * attenuation * intensity;
    
    return lighting * (1.0 - shadow * 0.8);
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
    vec3 viewDir = normalize(cameraPos - FragPos);
    
    // Start with ambient light
    vec3 ambient = 0.3 * baseColor.rgb;
    vec3 lighting = ambient;
    
    // Add directional lights with shadows
    for (int i = 0; i < int(numDirectionalLights) && i < 4; i++) {
        lighting += calculateDirectionalLight(directionalLights[i], norm, viewDir, baseColor.rgb, metallic, roughness, i);
    }
    
    // Add point lights with shadows
    for (int i = 0; i < int(numPointLights) && i < 32; i++) {
        lighting += calculatePointLight(pointLights[i], FragPos, norm, viewDir, baseColor.rgb, metallic, roughness, i);
    }
    
    // Add spot lights with shadows
    for (int i = 0; i < int(numSpotLights) && i < 16; i++) {
        lighting += calculateSpotLight(spotLights[i], FragPos, norm, viewDir, baseColor.rgb, metallic, roughness, i);
    }
    
    // Apply ambient occlusion
    lighting *= aoFactor;
    
    // Add emissive
    vec3 finalColor = lighting + emissiveColor;
    finalColor = max(finalColor, vec3(0.05));
    
    // Output final color
    fragColor = vec4(finalColor, baseColor.a);
}