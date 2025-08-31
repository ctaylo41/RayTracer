#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 aPos;
out vec3 aColor;
out vec3 aNormal;
out vec2 aUV;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    aPos = vec3(model * vec4(position, 1.0));
    aColor = color;
    aNormal = normal;
    aUV = uv;
}