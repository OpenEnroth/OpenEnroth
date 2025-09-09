#ifdef GL_ES
    precision highp int;
    precision highp float;
    precision highp sampler2DArray;
#endif

in vec4 vertexColour;
in vec2 texuv;
flat in vec2 olayer;
in vec3 vsPos;
in vec3 vsNorm;
in vec4 viewspace;

out vec4 FragColour;

struct Sunlight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {

    float type;  // 0- off, 1- stat, 2 - mob

    vec3 position;

    // dont use these atm
    //float constant;
    //float linear;
    //float quadratic;

    float radius;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct FogParam {
    vec3 color;
    float fogstart;
    float fogmiddle;
    float fogend;
};

uniform int waterframe;
uniform Sunlight sun;
uniform vec3 CameraPos;
uniform float gamma;

#define num_point_lights 20
uniform PointLight fspointlights[num_point_lights];

uniform sampler2DArray textureArray0;
uniform sampler2DArray textureArray1;
uniform FogParam fog;


// funcs
vec3 CalcSunLight(Sunlight light, vec3 normal, vec3 viewDir, vec3 thisfragcol);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float getFogRatio(FogParam fogpar, float dist);

void main() {
    vec3 fragnorm = normalize(vsNorm);
    vec3 fragviewdir = normalize(CameraPos - vsPos);

    // get water textures at point
    vec4 watercol = texture(textureArray0, vec3(texuv.x,texuv.y,waterframe));

    vec4 fragcol = vec4(0);

    // get normal texture at point
    fragcol = texture(textureArray1, vec3(texuv.x,texuv.y,olayer.y));

    // replace texture with water if alpha or a water tile
    if (fragcol.a == 0.0 || olayer.x == 0.0){
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
    if (fog.fogstart == fog.fogend) {
        FragColour = vec4(clamps, vertexColour.a);
        return;
    }

    float dist = abs(viewspace.z / viewspace.w);
    float alpha = 0.0;
    if (fog.fogmiddle > fog.fogstart) {
        alpha = smoothstep(fog.fogend, (fog.fogend + fog.fogmiddle) / 2.0, dist);
    }

    float fograt = getFogRatio(fog, dist);
    FragColour = mix(vec4(clamps, vertexColour.a), vec4(fog.color, alpha), fograt);
    FragColour.rgb = pow(FragColour.rgb, vec3(1.0/gamma));
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

// calculates the color when using a directional light.
vec3 CalcSunLight(Sunlight light, vec3 normal, vec3 viewDir, vec3 thisfragcol) {
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0); //material.shininess
    // combine results
    vec3 ambient = light.ambient * thisfragcol;
    vec3 diffuse = light.diffuse * diff * thisfragcol;
    vec3 specular = light.specular * spec * thisfragcol;  // should be spec map

    vec3 clamped = clamp((light.ambient + (light.diffuse * diff) + (light.specular * spec)), 0.0, 1.0) * thisfragcol;

    return clamped; //(ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (light.diffuse.r == 0.0 && light.diffuse.g == 0.0 && light.diffuse.b == 0.0)
        return vec3(0);
    if (light.radius < 1.0)
        return vec3(0);

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0); //material.shininess
    // attenuation
    float distance = length(light.position - fragPos);

    //float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float attenuation = clamp(1.0 - ((distance * distance)/(light.radius * light.radius)), 0.0, 1.0);
    attenuation *= attenuation;



    // combine results
    vec3 ambient = light.ambient ;//* vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff ;//* vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec ;//* vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
