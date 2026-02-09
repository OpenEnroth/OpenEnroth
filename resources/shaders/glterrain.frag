#include "precision.glsl"
#include "fog.glsl"
#include "lighting.glsl"

in vec4 vertexColour;
in vec2 texuv;
flat in float olayer;
in vec3 vsPos;
in vec3 vsNorm;
flat in int vsAttrib;
in vec4 viewspace;

out vec4 FragColour;

uniform int waterframe;
uniform Sunlight sun;
uniform vec3 CameraPos;
uniform float gamma;

#define num_point_lights 20
uniform PointLight fspointlights[num_point_lights];

uniform sampler2DArray textureArray0;
uniform sampler2DArray textureArray1;
uniform FogParam fog;

void main() {
    vec3 fragnorm = normalize(vsNorm);
    vec3 fragviewdir = normalize(CameraPos - vsPos);

    // get water textures at point
    vec4 watercol = texture(textureArray0, vec3(texuv.x,texuv.y,waterframe));

    vec4 fragcol = vec4(0);

    // get normal texture at point
    fragcol = texture(textureArray1, vec3(texuv.x,texuv.y,olayer));

    // replace texture with water if alpha or a water tile (bit 0x1 in attribs)
    if (fragcol.a == 0.0 || (vsAttrib & 0x1) > 0){
        fragcol = watercol;
    }

    // apply sun
    vec3 result = CalcSunLight(sun, fragnorm, fragviewdir, vec3(1));
    result = clamp(result, 0.0, 0.85);

    // stack torchlight if any
    result += CalcPointLight(fspointlights[0], fragnorm, vsPos, fragviewdir);

    // stack stationary lights
    for(int i = 1; i < num_point_lights; i++) {
        if (fspointlights[i].type == 1.0)
            result += CalcPointLight(fspointlights[i], fragnorm, vsPos, fragviewdir);
    }

    result *= fragcol.rgb;

    // stack mobile lights
    for(int i = 1; i < num_point_lights; i++) {
        if (fspointlights[i].type == 2.0)
            result += CalcPointLight(fspointlights[i], fragnorm, vsPos, fragviewdir);
    }

    vec3 clamps = result;

    float dist = length(viewspace);
    float alpha = getFogAlpha(fog, dist);

    float fograt = getFogRatio(fog, dist);
    FragColour = mix(vec4(clamps, vertexColour.a), vec4(fog.color, alpha), fograt);
    FragColour.rgb = pow(FragColour.rgb, vec3(1.0/gamma));
}
