#include "Engine/VectorTypes.h"

#include <utility>

#include "OurMath.h"
#include "Engine/mm7_data.h"

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z) {
    if (x < y) {
        std::swap(x, y);
    }
    if (x < z) {
        std::swap(x, z);
    }
    if (y < z) {
        std::swap(y, z);
    }

    return x + (11 * y >> 5) + (z >> 2);
}

void Vec3_float_::Normalize() {
    float denom = (1.0 / sqrt(this->x * this->x + this->y * this->y + this->z * this->z));
    this->x = denom * this->x;
    this->y = denom * this->y;
    this->z = denom * this->z;
}
