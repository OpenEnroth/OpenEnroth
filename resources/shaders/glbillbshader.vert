#version 410 core

layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in vec4 vaCol;
layout (location = 3) in float vaScreenSpace;

layout (location = 5) in float palid; 


out vec4 colour;
out vec2 texuv;
out float screenspace;
flat out uint paletteid;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(vaPos, 1.0);
    // float opacity = smoothstep(1.0 , 0.9999, vaPos.z);
    colour = vec4(vaCol.r, vaCol.g, vaCol.b, 1.0);
    texuv = vaTexUV;
    screenspace = vaScreenSpace;
    paletteid = int(palid);
} 