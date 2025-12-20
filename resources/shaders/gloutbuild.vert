layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in float vaTexLayer;
layout (location = 3) in vec3 vaNormal;
layout (location = 4) in float vaAttrib;

out vec4 vertexColour;
out vec2 texuv;
flat out float olayer;
out vec3 vsPos;
out vec3 vsNorm;
flat out int vsAttrib;
out vec4 viewspace;

uniform mat4 view;
uniform mat4 projection;

void main() {
    viewspace = view * vec4(vaPos, 1.0);
    gl_Position = projection * view * vec4(vaPos, 1.0);

    // rgb unused
    vertexColour = vec4(0.0, 0.0, 0.0, 1.0);

    texuv = vaTexUV;
    olayer = vaTexLayer;
    vsPos = vaPos;
    vsNorm = vaNormal;
    vsAttrib = int(vaAttrib);
}
