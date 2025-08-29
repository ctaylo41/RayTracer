#version 330 core
in vec3 fragPos;
in vec3 aColor;
out vec4 fragColor;

uniform sampler2D texture1;

void main() {
    fragColor = texture(texture1, fragPos.xy);
}