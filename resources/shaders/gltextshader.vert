layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in vec4 vaCol;
layout (location = 3) in float vaTexID;

out vec4 colour;
out vec2 texuv;
flat out float olayer;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(vaPos, 1.0);
    colour = vaCol;
    texuv = vaTexUV;
    olayer = vaTexID;
}