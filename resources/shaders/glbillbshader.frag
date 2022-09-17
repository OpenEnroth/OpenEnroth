#version 410 core

in vec4 colour;
in vec2 texuv;
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
    vec4 fragcol = texture(texture0, texuv) * colour;
    if (fog.fogstart == fog.fogend) {
        FragColour = fragcol;
        return;
    }

    float fograt = getFogRatio(fog, screenspace);
    if (fragcol.a < 0.004) fograt = 0.0;

    float alpha = 0.0;
    if (fog.fogmiddle > fog.fogstart) {
        if (fog.fogmiddle / 2.0 > screenspace) {
            alpha = 1.0;
        } else {
            alpha = (fog.fogmiddle - screenspace) / (fog.fogmiddle / 2.0);
        }
    }

    FragColour = mix(fragcol, vec4(fog.color, alpha), fograt);
}

float getFogRatio(FogParam fogpar, float dist) {
    float result = 0.0;
    if (fogpar.fogstart < fogpar.fogmiddle) {
       result = smoothstep(fogpar.fogstart, fogpar.fogmiddle, dist);
    } else {
        result = smoothstep(fogpar.fogstart, fogpar.fogend, dist);
    }
    return result;
}