// Common fog structures and functions.

struct FogParam {
    vec3 color;
    float weakDensity;
    float strongDensity;
    float weakDistance; // Density = weakDensity, alpha = 1.0.
    float strongDistance; // Density = strongDensity, alpha = 1.0.
    float clipDistance; // Density = 1.0, alpha = 0.0.
};

float getFogRatio(FogParam fog, float dist) {
    return
        fog.weakDensity +
        (fog.strongDensity - fog.weakDensity) * smoothstep(fog.weakDistance, fog.strongDistance, dist) +
        (1.0 - fog.strongDensity) * smoothstep(fog.strongDistance, fog.clipDistance, dist);
}

float getFogAlpha(FogParam fog, float dist) {
    return 1.0 - smoothstep(fog.strongDistance, fog.clipDistance, dist);
}
