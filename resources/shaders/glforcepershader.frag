#ifdef GL_ES
    precision highp int;
    precision highp float;
#endif

in vec4 colour;
in vec4 texuv;
in float screenspace;

out vec4 FragColour;

struct FogParam {
    vec3 color;
    float fogstart;
    float fogmiddle;
    float fogend;
};

uniform sampler2D texture0;
uniform FogParam fog;

float getFogRatio(FogParam fogpar, float dist);

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

float getFogRatio(FogParam fogpar, float dist) {
   float result = 0.0;
    if (fog.fogstart > fog.fogmiddle) {
        result = smoothstep(fogpar.fogstart, fogpar.fogend, dist);
    } else {
        result = 0.25 + smoothstep(fogpar.fogstart, fogpar.fogmiddle, dist) * 0.60 + smoothstep(fogpar.fogmiddle, fogpar.fogend, dist) * 0.15;
    }
    return result;
}
