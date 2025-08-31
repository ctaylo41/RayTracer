#version 330 core

// Input vertex attributes (from your VAO)
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUV;

// Output to fragment shader
out vec3 FragPos;
out vec3 Color;
out vec3 Normal;
out vec2 TexCoord;

// Uniforms for transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Transform vertex position
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Pass through color and texture coordinates
    Color = aColor;
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normal properly
    TexCoord = aUV;
    
    // Final position for OpenGL
    gl_Position = projection * view * vec4(FragPos, 1.0);
}