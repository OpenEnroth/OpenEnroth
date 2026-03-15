// Common fog structures and functions.

struct FogParam {
    vec3 color;
    float weakDensity;
    float strongDensity;
    float weakDistance; // Density = weakDensity, alpha = 1.0.
    float strongDistance; // Density = strongDensity, alpha = 1.0.
    float clipDistance; // Density = 1.0, alpha = 0.0.
};

// smoothstep with undefined behavior (edge0 >= edge1) replaced by a clamp: returns 0.0 when edge0 >= edge1,
// meaning x is treated as "before the transition starts". This matches the no-fog semantics where all
// distances are equal to the far clip — the transition never starts so fog contribution is zero.
float safeSmooth(float edge0, float edge1, float x) {
    if (edge0 >= edge1)
        return 0.0;
    return smoothstep(edge0, edge1, x);
}

float getFogRatio(FogParam fog, float dist) {
    return
        fog.weakDensity +
        (fog.strongDensity - fog.weakDensity) * safeSmooth(fog.weakDistance, fog.strongDistance, dist) +
        (1.0 - fog.strongDensity) * safeSmooth(fog.strongDistance, fog.clipDistance, dist);
}

float getFogAlpha(FogParam fog, float dist) {
    return 1.0 - safeSmooth(fog.strongDistance, fog.clipDistance, dist);
}
