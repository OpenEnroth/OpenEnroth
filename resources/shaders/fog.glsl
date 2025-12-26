// Common fog structures and functions.

struct FogParam {
    vec3 color;
    float fogstart;
    float fogmiddle;
    float fogend;
};

float getFogRatio(FogParam fogpar, float dist) {
    float result = 0.0;
    if (fogpar.fogstart < fogpar.fogmiddle) {
        result = 0.25 + smoothstep(fogpar.fogstart, fogpar.fogmiddle, dist) * 0.60
               + smoothstep(fogpar.fogmiddle, fogpar.fogend, dist) * 0.15;
    } else {
        result = smoothstep(fogpar.fogstart, fogpar.fogend, dist);
    }
    return result;
}
