layout (location = 0) in vec3 vaPos;
layout (location = 1) in float vaW;
layout (location = 2) in vec2 vaTexUV;
layout (location = 3) in float vaTexW;
layout (location = 4) in float vaScreenSpace;
layout (location = 5) in vec4 vaCol;

out vec4 colour;
out vec4 texuv;
out float screenspace;

uniform mat4 view;
uniform mat4 projection;

void main() {
    // vaW contains depth information needed for perspective-correct interpolation.
    //
    // The GPU uses gl_Position.w to interpolate ALL varyings (texuv, colour, etc.),
    // not just positions. During rasterization, for each varying V at vertices 0, 1, 2:
    //   1. Compute V0/w0, V1/w1, V2/w2
    //   2. Interpolate linearly: V_linear = a*(V0/w0) + b*(V1/w1) + c*(V2/w2)
    //   3. Interpolate 1/w linearly: inv_w = a/w0 + b/w1 + c/w2
    //   4. Final value: V_final = V_linear / inv_w
    //
    // If w=1.0 for all vertices, this simplifies to plain linear interpolation,
    // which is not what we need.
    //
    // The z offset of 0.000001 is needed for 6700XT under linux or sky will flicker.
    vec4 forcepos = vec4(vaPos.x * vaW, vaPos.y * vaW, vaPos.z * vaW - 0.000001, vaW);
    gl_Position = projection * view * forcepos;
    colour = vec4(vaCol);
    texuv = vec4(vaTexUV.x * vaTexW, vaTexUV.y * vaTexW, 0, vaTexW);
    screenspace = vaScreenSpace;
}
