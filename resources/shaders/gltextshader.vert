layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in mat4 vaColours; // One color per texture channel, in columns.

out mat4 colours;
out vec2 texuv;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(vaPos, 1.0);
    colours = vaColours;
    texuv = vaTexUV;
}
