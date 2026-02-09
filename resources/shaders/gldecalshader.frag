#include "precision.glsl"
#include "fog.glsl"

in vec4 vertexColour;
in vec2 texuv;
//flat in float olayer;
//in vec3 vsPos;
//in vec3 vsNorm;
//flat in int vsAttrib;
in vec4 viewspace;

out vec4 FragColour;

uniform sampler2D texture0;
uniform FogParam fog;

void main() {
    vec4 fragcol = texture(texture0, texuv) * vertexColour;

    float fograt = getFogRatio(fog, abs(viewspace.z/ viewspace.w));
    if (fragcol.a < 0.004) fograt = 0.0;
    
    FragColour = mix(fragcol, vec4(0.0), fograt);
}
