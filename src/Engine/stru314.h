#pragma once

/*  179 */
#pragma pack(push, 1)
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

    Vec3f Normal;
    Vec3f field_10; // For decal application: u vector, perpendicular to Normal
    Vec3f field_1C; // For decal application: v vector, perpendicular to Normal and u
    float dist = 0;
};
#pragma pack(pop)
