#pragma once
#include <array>

#include "Engine/OurMath.h"

/*  152 */
#pragma pack(push, 1)
struct stru367 {
    bool direction;
    std::array<int, 13> field_4;
    std::array<int, 60> _view_transformed_x;
    std::array<int, 60> _view_transformed_y;
    std::array<int, 48> _view_transformed_z;
    std::array<int, 60> _screen_space_y;
    std::array<int, 60> _screen_space_x;
};
#pragma pack(pop)

// extern  const stru367 stru_367;
