#ifdef GL_ES
    precision highp int;
    precision highp float;
#endif

in vec4 vertexColour;
in vec2 texuv;
//flat in float olayer;
//in vec3 vsPos;
//in vec3 vsNorm;
//flat in int vsAttrib;
in vec4 viewspace;

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
    vec4 fragcol = texture(texture0, texuv) * vertexColour;
    if (fog.fogstart == fog.fogend) {
        FragColour = fragcol;
        return;
    }

    float fograt = getFogRatio(fog, abs(viewspace.z/ viewspace.w));
    if (fragcol.a < 0.004) fograt = 0.0;

    FragColour = mix(fragcol, vec4(0.0), fograt);

}

float getFogRatio(FogParam fogpar, float dist) {
   float result = 0.0;
    if (fogpar.fogstart < fogpar.fogmiddle) {
       result = 0.25 + smoothstep(fogpar.fogstart, fogpar.fogmiddle, dist) * 0.60 + smoothstep(fogpar.fogmiddle, fogpar.fogend, dist) * 0.15;
    } else {
        result = smoothstep(fogpar.fogstart, fogpar.fogend, dist);
    }
    return result;
}
