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

// Simple shadow calculation for debugging
float calculateShadowSimple(vec4 fragPosLightSpace, sampler2D shadowMapTexture) {
    // Perform perspective divide
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
    
    // Simple shadow test with bias
    float bias = 0.005;
    return currentDepth - bias > closestDepth ? 0.7 : 0.0;
}

// Debug version - visualize shadow maps
float getShadowFactorDebug(int lightIndex) {
    // Only check first shadow map for debugging
    if(numShadowMaps > 0.0 && int(shadowMaps[0].lightIndex) == lightIndex) {
        vec4 fragPosLightSpace = shadowMaps[0].lightSpaceMatrix * vec4(FragPos, 1.0);
        
        // For debugging - return different values to see if shadows are working
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
        
        // Debug: return red tint if in shadow map bounds
        if(projCoords.x >= 0.0 && projCoords.x <= 1.0 && 
           projCoords.y >= 0.0 && projCoords.y <= 1.0 && 
           projCoords.z <= 1.0) {
            return calculateShadowSimple(fragPosLightSpace, shadowMap0);
        }
    }
    return 0.0;
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
    
    // Get normal
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(cameraPos - FragPos);
    
    // Start with ambient light
    vec3 ambient = 0.3 * baseColor.rgb;
    vec3 lighting = ambient;
    
    // Simple directional light calculation with shadows
    for (int i = 0; i < int(numDirectionalLights) && i < 4; i++) {
        if (!directionalLights[i].enabled) continue;
        
        vec3 lightDir = normalize(-directionalLights[i].direction.xyz);
        float diff = max(dot(norm, lightDir), 0.0);
        
        // Calculate shadow
        float shadow = getShadowFactorDebug(i);
        
        // Apply lighting with shadow
        vec3 diffuse = diff * baseColor.rgb * directionalLights[i].color.rgb * directionalLights[i].color.w;
        lighting += diffuse * (1.0 - shadow * 0.8);
        
        // Debug: Add slight red tint where shadows should be
        if (shadow > 0.0) {
            lighting += vec3(0.1, 0.0, 0.0); // Slight red tint in shadow areas
        }
    }
    
    // Debug: Show if we have shadow maps
    if (numShadowMaps > 0.0) {
        // Add slight blue tint to show shadow mapping is active
        lighting += vec3(0.0, 0.0, 0.05);
    }
    
    // Output final color
    fragColor = vec4(lighting, baseColor.a);
}