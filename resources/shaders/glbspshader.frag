#ifdef GL_ES
    precision highp float;
    precision highp sampler2DArray;
#endif

in vec4 vertexColour;
in vec2 texuv;
flat in vec2 olayer;
in vec3 vsPos;
in vec3 vsNorm;
flat in int vsAttrib;
flat in int vsSector;

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
    float sector;

    //float constant;
    //float linear;
    //float quadratic;


    float radius;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform int waterframe;
uniform Sunlight sun;
uniform vec3 CameraPos;
uniform int flowtimer;
uniform int flowtimerms; // TODO(Nik-RE-dev): use a single timer for everything
uniform int watertiles;
uniform float gamma;

#define num_point_lights 40
uniform PointLight fspointlights[num_point_lights];

uniform sampler2DArray textureArray0;


// funcs
vec3 CalcSunLight(Sunlight light, vec3 normal, vec3 viewDir, vec3 thisfragcol);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    vec3 fragnorm = normalize(vsNorm);
    vec3 fragviewdir = normalize(CameraPos - vsPos);

    vec4 fragcol = vec4(1.0);
    vec2 texcoords = vec2(0.0);
    vec2 texuvmod = vec2(0.0);
    vec2 deltas = vec2(0.0);
    ivec3 texsize = textureSize(textureArray0,0);

    // texture flow mods
    if (abs(vsNorm.z) >= 0.9) {
        if ((vsAttrib & 0x400) > 0) texuvmod.y = 1.0;
        if ((vsAttrib & 0x800) > 0) texuvmod.y = -1.0;
    } else {
        if ((vsAttrib & 0x400) > 0) texuvmod.y = -1.0;
        if ((vsAttrib & 0x800) > 0) texuvmod.y = 1.0;
    }

    if ((vsAttrib & 0x1000) > 0) {
        texuvmod.x = -1.0;
    } else if ((vsAttrib & 0x2000) > 0) {
        texuvmod.x = 1.0;
    }

    // lava movement
    if ((vsAttrib & 0x4000) > 0) {
        // Texture makes full circle in 8 seconds
        float lavaperiod = mod(float(flowtimerms), 8000.0);
        float lavaradians = lavaperiod * radians(360.0) / 8000.0;
        float curpos = sin(lavaradians);
        deltas.x = 0.0;
        deltas.y = float(texsize.y) * curpos;
    } else {
        deltas.x = texuvmod.x * mod(float(flowtimer), float(texsize.x));
        deltas.y = texuvmod.y * mod(float(flowtimer), float(texsize.y));
    }

    texcoords.x = (deltas.x + texuv.x) / float(texsize.x);
    texcoords.y = (deltas.y + texuv.y) / float(texsize.y);
    fragcol = texture(textureArray0, vec3(texcoords.x,texcoords.y,olayer.y));

    vec4 toplayer = texture(textureArray0, vec3(texcoords.x,texcoords.y,0));
    vec4 watercol = texture(textureArray0, vec3(texcoords.x,texcoords.y,waterframe));

    if ((watertiles == 1) && (olayer.y == 0.0)){
        if ((vsAttrib & 0x3C00) != 0){ // water anim disabled
            fragcol = toplayer;
        } else {
            fragcol = watercol;
        }
    }


    vec3 result = CalcSunLight(sun, fragnorm, fragviewdir, vec3(1)); //fragcol.rgb);
    result = clamp(result, 0.0, 0.85);

    result += CalcPointLight(fspointlights[0], fragnorm, vsPos, fragviewdir);

    // stack stationary
    for(int i = 1; i < num_point_lights; i++) {
        if (fspointlights[i].type == 1.0)
            result += CalcPointLight(fspointlights[i], fragnorm, vsPos, fragviewdir);
    }

    result *= fragcol.rgb;

    // stack mobile

    for(int i = 1; i < num_point_lights; i++) {
        if (fspointlights[i].type == 2.0)
            result += CalcPointLight(fspointlights[i], fragnorm, vsPos, fragviewdir);
    }

    vec3 clamps = result; // fragcol.rgb *  // clamp(result,0,1) * ; 

        vec3 dull;

    // percpetion red fade
    if ((vsAttrib & 0x10000) > 0) {
        float ss = (sin(float(flowtimer) / 30.0) + 1.0) / 2.0;
        dull = vec3(1, ss, ss);
    } else {
        dull = vec3(1,1,1);
    }

    FragColour = vec4(clamps,1)  * vec4(dull,1); // result, 1.0);

    FragColour.rgb = pow(FragColour.rgb, vec3(1.0/gamma));

}

// calculates the color when using a directional light.
vec3 CalcSunLight(Sunlight light, vec3 normal, vec3 viewDir, vec3 thisfragcol) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0 ); //material.shininess
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
    float distance = length(light.position - fragPos);
    if (distance > light.radius)
        return vec3(0);

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0); //material.shininess
    // attenuation

    //float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float attenuation = clamp(1.0 - ((distance * distance)/(light.radius * light.radius)), 0.0, 1.0);
    attenuation *= attenuation;



    // combine results
    vec3 ambient = light.ambient;//* vec3(texture(material.diffuse, TexCoords));

    // no ambient light if facing away from lightsource
    if (diff == 0.0) {
        ambient = vec3(0.0);
    }

    // stationary sources cant light outside their sector
    // if (light.sector > 0.0) {
    //     if (vsSector != int(light.sector)) {
    //         ambient = vec3(0);
    //         diff = 0.0;
    //     }
    // }

    vec3 diffuse = light.diffuse * diff ;//* vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec ;//* vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
