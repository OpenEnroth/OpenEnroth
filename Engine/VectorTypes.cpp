#include "Engine/VectorTypes.h"

#include <utility>

#include "OurMath.h"
#include "Engine/mm7_data.h"

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z) { // approx distance
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
