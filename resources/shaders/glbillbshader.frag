#include "precision.glsl"
#include "fog.glsl"

in vec4 colour;
in vec2 texuv;
in float screenspace;
flat in int paletteid;

out vec4 FragColour;

uniform sampler2D texture0;
uniform sampler2D paltex2D;
uniform FogParam fog;
uniform float gamma;

void main() {
    vec4 fragcol = texture(texture0, texuv);
    int index = int(fragcol.r * 255.0);
    vec4 newcol = vec4(texelFetch(paltex2D, ivec2(index, paletteid), 0));

    if (paletteid > 0)
        if (index > 0)
            fragcol = vec4(newcol.r, newcol.g, newcol.b, 1.0);

    fragcol *= colour;

    if (fog.fogstart == fog.fogend) {
        FragColour = fragcol;
        return;
    }

    float fograt = getFogRatio(fog, screenspace);
    if (fragcol.a < 0.004) fograt = 0.0;

    float alpha = 0.0;
    if (fog.fogmiddle > fog.fogstart) {
        alpha = smoothstep(fog.fogend, (fog.fogend + fog.fogmiddle) / 2.0, screenspace);
    }

    FragColour = mix(fragcol, vec4(fog.color, alpha), fograt);
    FragColour.rgb = pow(FragColour.rgb, vec3(1.0/gamma));
}
