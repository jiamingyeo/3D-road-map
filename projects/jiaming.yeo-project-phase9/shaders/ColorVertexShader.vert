#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

uniform mat4 uViewProj;

out vec3 outColor;
out vec3 outNormal;

void main() {
    gl_Position = uViewProj * vec4(position, 1.0);
    outColor = color;
    outNormal = normal;
}