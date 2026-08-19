#pragma once

#include "Library/Geometry/Vec.h"

// TODO(captainurist): this is a facet plane plus a decal basis - rename the struct and its field_* members
//                     accordingly.
struct stru314 {  // facet normals face / wall / celings
    //----- (00489B60) --------------------------------------------------------
    stru314() {
        this->Normal.x = 0.0;
        this->Normal.y = 0.0;
        this->Normal.z = 0.0;

        this->field_10.x = 0.0;
        this->field_10.y = 0.0;
        this->field_10.z = 0.0;

        this->field_1C.x = 0.0;
        this->field_1C.y = 0.0;
        this->field_1C.z = 0.0;

        this->dist = 0;
    }

    //----- (00489B96) --------------------------------------------------------
    inline ~stru314() {}

    /**
     * Computes the facet-local u and v axes from `Normal` and stores them in `field_10` and `field_1C`.
     *
     * @offset 0x436932
     */
    void computeBasis();

    Vec3f Normal;
    Vec3f field_10; // For decal application: u vector, perpendicular to Normal
    Vec3f field_1C; // For decal application: v vector, perpendicular to Normal and u
    float dist = 0;
};
