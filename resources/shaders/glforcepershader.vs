#version 410 core

layout (location = 0) in vec4 vaPos;
layout (location = 1) in vec3 vaTexUV;
layout (location = 2) in vec3 vaCol;


out vec4 colour;
out vec4 texuv;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 forcepos = vec4(vaPos.x * vaPos.w, vaPos.y * vaPos.w, vaPos.z * vaPos.w,  vaPos.w);
    gl_Position = projection * view * forcepos;
    colour = vec4(vaCol, 1.0);
    texuv = vec4(vaTexUV.x * vaTexUV.z, vaTexUV.y * vaTexUV.z, 0, vaTexUV.z);
} 