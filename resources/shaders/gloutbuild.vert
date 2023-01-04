layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in vec2 vaTexLayer;
layout (location = 3) in vec3 vaNormal;
layout (location = 4) in float vaAttrib;

out vec4 vertexColour;
out vec2 texuv;
flat out vec2 olayer;
out vec3 vsPos;
out vec3 vsNorm;
flat out int vsAttrib;
out vec4 viewspace;

uniform mat4 view;
uniform mat4 projection;

void main() {
    viewspace = view * vec4(vaPos, 1.0);
    gl_Position = projection * view * vec4(vaPos, 1.0);

    //if (gl_Position.z > 1.0)
    //    gl_Position = vec4(gl_Position.x, gl_Position.y, 0.999999999999, gl_Position.w);

    // rgb unused
    //float opacity = smoothstep(1.0 , 0.9999, gl_Position.z / gl_Position.w);
    //if (gl_Position.z < 0.0) opacity = 1.0;
    vertexColour = vec4(0.0, 0.0, 0.0, 1.0);

    texuv = vaTexUV;
    olayer = vaTexLayer;
    vsPos = vaPos;
    vsNorm = vaNormal;
    vsAttrib = int(vaAttrib);
}
