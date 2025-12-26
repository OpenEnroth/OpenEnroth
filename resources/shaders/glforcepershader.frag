#include "precision.glsl"
#include "fog.glsl"

in vec4 colour;
in vec4 texuv;
in float screenspace;

out vec4 FragColour;

uniform sampler2D texture0;
uniform FogParam fog;

void main() {
    vec4 fragcol = textureProj(texture0, texuv) * colour;

    // no fog early break
    if (fog.fogstart == fog.fogend) {
        FragColour = fragcol;
        return;
    }

    float fograt = getFogRatio(fog, screenspace);
    if (fragcol.a < 0.004) fograt = 0.0;
    //if (fragcol.r < 0.04 && fragcol.g < 0.04 && fragcol.b < 0.04) fograt = 0.0;

    float alpha = 0.0;
    // day fog
    if (fog.fogmiddle > fog.fogstart) alpha = 1.0;

    FragColour = mix(fragcol, vec4(fog.color, alpha), fograt);
}
