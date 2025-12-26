// Common lighting structures and functions.

struct Sunlight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    float type;  // 0 = off, 1 = stationary, 2 = mobile
    vec3 position;
    float radius;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Calculates the color when using a directional light.
vec3 CalcSunLight(Sunlight light, vec3 normal, vec3 viewDir, vec3 thisfragcol) {
    vec3 lightDir = normalize(light.direction);
    // Diffuse shading.
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading.
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0);
    // Combine results.
    vec3 clamped = clamp((light.ambient + (light.diffuse * diff) + (light.specular * spec)), 0.0, 1.0) * thisfragcol;
    return clamped;
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (light.diffuse.r == 0.0 && light.diffuse.g == 0.0 && light.diffuse.b == 0.0)
        return vec3(0);
    if (light.radius < 1.0)
        return vec3(0);

    float distance = length(light.position - fragPos);

    if (distance > light.radius)
        return vec3(0);

    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading.
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading.
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0);
    // Attenuation.
    float attenuation = clamp(1.0 - ((distance * distance) / (light.radius * light.radius)), 0.0, 1.0);
    attenuation *= attenuation;

    // Combine results.
    // No ambient light if facing away from light source.
    vec3 ambient = (diff > 0.0) ? light.ambient : vec3(0.0);
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
