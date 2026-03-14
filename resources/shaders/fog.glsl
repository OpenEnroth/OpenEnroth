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
    // Guard against degenerate smoothstep (GLSL undefined behavior when edge0 >= edge1).
    // Check fog params directly (not dist) to avoid floating-point interpolation precision issues.
    if (fog.weakDistance >= fog.clipDistance)
        return fog.weakDensity;
    return
        fog.weakDensity +
        (fog.strongDensity - fog.weakDensity) * smoothstep(fog.weakDistance, fog.strongDistance, dist) +
        (1.0 - fog.strongDensity) * smoothstep(fog.strongDistance, fog.clipDistance, dist);
}

float getFogAlpha(FogParam fog, float dist) {
    if (fog.strongDistance >= fog.clipDistance)
        return 1.0;
    return 1.0 - smoothstep(fog.strongDistance, fog.clipDistance, dist);
}
