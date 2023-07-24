#ifdef GL_ES
    precision highp float;
    precision highp usamplerBuffer;
#endif

in vec4 colour;
in vec2 texuv;
in float screenspace;
flat in int paletteid;

out vec4 FragColour;

struct FogParam {
    vec3 color;
    float fogstart;
    float fogmiddle;
    float fogend;
};

uniform sampler2D texture0;
uniform usamplerBuffer palbuf;
uniform FogParam fog;
uniform float gamma;

float getFogRatio(FogParam fogpar, float dist);

void main() {
    vec4 fragcol = texture(texture0, texuv);
    int index = int(fragcol.r * 255.0);
    vec4 newcol = vec4(texelFetch(palbuf, int(256 * paletteid + index)));

    if (paletteid > 0)
        if (index > 0)
            fragcol = vec4(newcol.r / 255.0, newcol.g / 255.0, newcol.b / 255.0, 1.0);

    fragcol *= colour;

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
    FragColour.rgb = pow(FragColour.rgb, vec3(1.0/gamma));
}

float getFogRatio(FogParam fogpar, float dist) {
    float result = 0.0;
    if (fogpar.fogstart < fogpar.fogmiddle) {
       result = 0.25 + smoothstep(fogpar.fogstart, fogpar.fogmiddle, dist) * 0.75;
    } else {
        result = smoothstep(fogpar.fogstart, fogpar.fogend, dist);
    }
    return result;
}
