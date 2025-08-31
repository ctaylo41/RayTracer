#version 330 core
in vec3 aPos;
in vec3 aColor;
in vec3 aNormal;
in vec2 aUV;

out vec4 fragColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;


void main() {
    fragColor = texture(diffuseMap, aUV);
}