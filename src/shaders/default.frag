#version 330 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Color;
in vec3 Normal;
in vec2 TexCoord;

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

void main() {
    // Sample base color texture
    vec4 baseColor = texture(baseColorTexture, TexCoord);
    
    // Apply base color factor (allows tinting)
    baseColor.rgb *= baseColorFactor.rgb;
    
    // Use vertex color if base color is black/empty (safer check)
    if (baseColor.rgb == vec3(0.0, 0.0, 0.0)) {
        baseColor.rgb = vec3(0.2,0.2,0.2);
    }
    
    // Handle alpha testing/blending
    if (!useAlphaBlending && baseColor.a < alphaCutoff) {
        discard; // Alpha test - discard transparent pixels
    }
    
    // Sample metallic-roughness texture
    // Red channel = occlusion (if no separate AO map)
    // Green channel = roughness
    // Blue channel = metallic
    vec4 metallicRoughness = texture(metallicRoughnessTexture, TexCoord);
    float metallic = metallicRoughness.b * metallicFactor;
    float roughness = metallicRoughness.g * roughnessFactor;
    
    // Sample ambient occlusion
    vec4 occlusion = texture(occlusionTexture, TexCoord);
    float aoFactor = (occlusion.r > 0.0) ? occlusion.r : 1.0;
    
    // Sample emissive texture
    vec4 emissive = texture(emissiveTexture, TexCoord);
    vec3 emissiveColor = emissive.rgb * emissiveFactor.rgb;
    
    // Sample normal map (for future lighting calculations)
    vec4 normalMap = texture(normalTexture, TexCoord);
    // Convert from [0,1] to [-1,1] range for normal mapping
    vec3 tangentNormal = normalize(normalMap.rgb * 2.0 - 1.0);
    
    // Simple material visualization without lighting
    // This gives you a preview of how materials will look
    
    // Simulate basic material response
    vec3 dielectricColor = baseColor.rgb * (1.0 - metallic);
    vec3 metallicColor = mix(vec3(0.04), baseColor.rgb, metallic); // F0 for metals
    
    // Combine dielectric and metallic response
    vec3 materialColor = mix(dielectricColor, metallicColor, metallic);
    
    // Apply ambient occlusion
    materialColor *= aoFactor;
    
    // Add emissive (self-illumination)
    vec3 finalColor = materialColor + emissiveColor;
    
    // Output final color with alpha
    fragColor = vec4(finalColor, baseColor.a);
    

    // Debug modes (uncomment one to debug specific aspects):
    //fragColor = texture(baseColorTexture, TexCoord);
    //fragColor = vec4(baseColor.rgb, 1.0);                    // Base color only
    // fragColor = vec4(vec3(metallic), 1.0);                   // Metallic values
    // fragColor = vec4(vec3(roughness), 1.0);                  // Roughness values
    // fragColor = vec4(vec3(aoFactor), 1.0);                   // Ambient occlusion
    // fragColor = vec4(emissiveColor, 1.0);                    // Emissive only
    // fragColor = vec4(tangentNormal * 0.5 + 0.5, 1.0);       // Normal map visualization
    // fragColor = vec4(abs(Normal), 1.0);                      // Vertex normals
    // fragColor = vec4(vec3(baseColor.a), 1.0);                // Alpha channel visualization
}