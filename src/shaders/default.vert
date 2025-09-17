#version 330 core

// Input vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUV;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBitangent;

// Output to fragment shader
out vec3 FragPos;
out vec3 Color;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Tangent;
out vec3 Bitangent;
out vec4 FragPosLightSpace; // Add this for shadow mapping

// Uniforms for transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // Add this for shadow mapping

void main() {
    // Transform vertex position
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Pass through color and texture coordinates
    Color = aColor;
    TexCoord = aUV;
    
    // Transform normal properly (using normal matrix)
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * aNormal);
    
    // Transform tangent and bitangent properly
    Tangent = normalize(normalMatrix * aTangent);
    Bitangent = normalize(normalMatrix * aBitangent);
    
    // Calculate position in light space for shadow mapping
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    
    // Final position for OpenGL
    gl_Position = projection * view * vec4(FragPos, 1.0);
}