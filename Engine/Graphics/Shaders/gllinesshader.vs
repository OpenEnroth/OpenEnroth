#version 410 core

layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec3 vaCol;

out vec3 colour;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(vaPos, 1.0);
    colour = vaCol;
} 