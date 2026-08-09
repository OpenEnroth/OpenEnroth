#include "Engine/stru314.h"

#include <cmath>

void stru314::computeBasis() {
    Vec3f *outU = &field_10;
    Vec3f *outV = &field_1C;
    const Vec3f &normal = Normal;
    if (fabsf(normal.z) < 1e-6f) {
        // Vertical wall.
        outV->x = -normal.y;
        outV->y = normal.x;
        outV->z = 0.0;

        outU->x = 0.0;
        outU->y = 0.0;
        outU->z = 1.0f;
    } else if (fabsf(normal.x) < 1e-6f && fabsf(normal.y) < 1e-6f) {
        // Floor.
        outV->x = 1.0;
        outV->y = 0.0;
        outV->z = 0.0;

        outU->x = 0.0;
        outU->y = 1.0;
        outU->z = 0.0;
    } else {
        // Other.
        if (fabs(normal.z) < 0.70811361) {
            outV->x = -normal.y;
            outV->y = normal.x;
            outV->z = 0.0;
            outV->normalize();

            outU->x = 0.0;
            outU->y = 0.0;
            outU->z = 1.0;
        } else {
            outV->x = 1.0;
            outV->y = 0.0;
            outV->z = 0.0;

            outU->x = 0.0;
            outU->y = 1.0;
            outU->z = 0.0;
        }
    }
}
