#version 330 core

// Input vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUV;

// Output to fragment shader
out vec3 FragPos;
out vec3 Color;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Tangent;
out vec3 Bitangent;

// Uniforms for transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Transform vertex position
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Pass through color and texture coordinates
    Color = aColor;
    TexCoord = aUV;
    
    // Transform normal properly (using normal matrix)
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * aNormal);
    
    // Calculate tangent and bitangent for normal mapping
    // This is a simplified approach - ideally tangents should be calculated per-vertex
    // and passed as vertex attributes, but this works for basic normal mapping
    
    vec3 c1 = cross(Normal, vec3(0.0, 0.0, 1.0));
    vec3 c2 = cross(Normal, vec3(0.0, 1.0, 0.0));
    
    // Choose the cross product that gives the larger result
    if (length(c1) > length(c2)) {
        Tangent = normalize(c1);
    } else {
        Tangent = normalize(c2);
    }
    
    // Bitangent is perpendicular to both normal and tangent
    Bitangent = normalize(cross(Normal, Tangent));
    
    // Final position for OpenGL
    gl_Position = projection * view * vec4(FragPos, 1.0);
}