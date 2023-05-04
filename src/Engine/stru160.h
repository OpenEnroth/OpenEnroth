#pragma once

struct stru160 {
    inline stru160() {}
    inline stru160(float a, float b, float c)
        : field_0(a), field_4(b), field_8(c) {}

    float field_0 = 0;
    float field_4 = 0;
    float field_8 = 0;
};

struct stru16x : public stru160 {
    float field_C = 0;
};

extern std::array<stru160, 66> array_4EB8B8;
extern std::array<stru160, 128> array_4EBBD0;
extern std::array<int32_t, 128 * 3> array_4EBBD0_x;
