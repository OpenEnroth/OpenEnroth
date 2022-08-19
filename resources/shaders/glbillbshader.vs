#version 410 core

layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in vec4 vaCol;


out vec4 colour;
out vec2 texuv;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(vaPos, 1.0);
    float opacity = smoothstep(1.0 , 0.9999, vaPos.z);
    colour = vec4(vaCol.r, vaCol.g, vaCol.b, opacity);
    texuv = vaTexUV;
} 