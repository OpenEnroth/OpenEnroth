layout (location = 0) in vec4 vaPos;
layout (location = 1) in vec3 vaTexUV;
layout (location = 2) in float vaScreenSpace;
layout (location = 3) in vec4 vaCol;

out vec4 colour;
out vec4 texuv;
out float screenspace;

uniform mat4 view;
uniform mat4 projection;

void main() {
    // vaPos.w - 0.000001 is needed for 6700XT under linux or sky will flicker
    vec4 forcepos = vec4(vaPos.x * vaPos.w, vaPos.y * vaPos.w, vaPos.z * vaPos.w - 0.000001,  vaPos.w);
    gl_Position = projection * view * forcepos;
    colour = vec4(vaCol);
    texuv = vec4(vaTexUV.x * vaTexUV.z, vaTexUV.y * vaTexUV.z, 0, vaTexUV.z);
    screenspace = vaScreenSpace;
}
