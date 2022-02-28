#version 410 core

in vec4 vertexColour;
in vec2 texuv;
flat in vec2 olayer;
in vec3 vsPos;
in vec3 vsNorm;

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

#define num_point_lights 20
uniform PointLight fspointlights[num_point_lights];

uniform sampler2DArray textureArray0;
uniform sampler2DArray textureArray1;
//layout (binding = 2) uniform sampler2DArray textureArray2;
//layout (binding = 3) uniform sampler2DArray textureArray3;
//layout (binding = 4) uniform sampler2DArray textureArray4;
//layout (binding = 5) uniform sampler2DArray textureArray5;
//layout (binding = 6) uniform sampler2DArray textureArray6;
//layout (binding = 7) uniform sampler2DArray textureArray7;



// funcs
vec3 CalcSunLight(Sunlight light, vec3 normal, vec3 viewDir, vec3 thisfragcol);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {

	vec3 fragnorm = normalize(vsNorm);
    vec3 fragviewdir = normalize(CameraPos - vsPos);

    vec4 watercol = texture(textureArray0, vec3(texuv.x,texuv.y,waterframe));

	vec4 fragcol = vec4(0);

//    if (olayer.x == 7) {
//        fragcol = texture(textureArray7, vec3(texuv.x,texuv.y,olayer.y));
//    }

//    if (olayer.x == 6) {
//        fragcol = texture(textureArray6, vec3(texuv.x,texuv.y,olayer.y));
//    }

//    if (olayer.x == 5) {
//        fragcol = texture(textureArray5, vec3(texuv.x,texuv.y,olayer.y));
//    }

//    if (olayer.x == 4) {
//        fragcol = texture(textureArray4, vec3(texuv.x,texuv.y,olayer.y));
//    }

//   if (olayer.x == 3) {
//        fragcol = texture(textureArray3, vec3(texuv.x,texuv.y,olayer.y));
//    }

//    if (olayer.x == 2) {
//        fragcol = texture(textureArray2, vec3(texuv.x,texuv.y,olayer.y));
//    }

//    if (olayer.x == 1) {
        fragcol = texture(textureArray1, vec3(texuv.x,texuv.y,olayer.y));
//    }

//    if (olayer.x == 0) {
//        fragcol = texture(textureArray0, vec3(texuv.x,texuv.y,olayer.y));
//        if (olayer.y == 0){
//            fragcol = watercol;
//        }
//    }

    if (fragcol.a == 0 || olayer.x == 0){
        fragcol = watercol;
    }




//	if (olayer.y == 0){
//		//FragColour = texture(textureArray, vec3(texuv.x,texuv.y,waterframe));
//		fragcol = texture(textureArray0, vec3(texuv.x,texuv.y,waterframe));
//	} else {
//		//FragColour = texture(textureArray, vec3(texuv.x,texuv.y,olayer));
//		fragcol = texture(textureArray0, vec3(texuv.x,texuv.y,olayer.y));
//		//if (FragColour.a == 0){
//		if (fragcol.a == 0){
//			//FragColour = texture(textureArray, vec3(texuv.x,texuv.y,waterframe));
//			fragcol = texture(textureArray0, vec3(texuv.x,texuv.y,waterframe));
//		}
//	}





	vec3 result = CalcSunLight(sun, fragnorm, fragviewdir, vec3(1)); //fragcol.rgb);
    result = clamp(result, 0, 0.85);

    result += CalcPointLight(fspointlights[0], fragnorm, vsPos, fragviewdir);

    // stack stationary
    for(int i = 1; i < num_point_lights; i++) {
        if (fspointlights[i].type == 1)
            result += CalcPointLight(fspointlights[i], fragnorm, vsPos, fragviewdir);
    }

    result *= fragcol.rgb;

    // stack mobile

    for(int i = 1; i < num_point_lights; i++) {
        if (fspointlights[i].type == 2)
            result += CalcPointLight(fspointlights[i], fragnorm, vsPos, fragviewdir);
    }

    vec3 clamps = result; // fragcol.rgb *  // clamp(result,0,1) * ; 

	FragColour = vec4(clamps,1); // result, 1.0);

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

	vec3 clamped = clamp((light.ambient + (light.diffuse * diff) + (light.specular * spec)), 0, 1) * thisfragcol;

    return clamped; //(ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (light.diffuse.r == 0 && light.diffuse.g == 0 && light.diffuse.b == 0) return vec3(0);    

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128); //material.shininess
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
