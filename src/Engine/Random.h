#pragma once

/*  304 */
#pragma pack(push, 1)
struct Random {
    //----- (004BE365) --------------------------------------------------------
    inline Random() {
        Initialize(0x5CCE1E47u);
        min = 0;
        range = 99;
    }

    void SetRange(int min, int max);
    double GetRandom();
    int GetInRange();
    void Initialize(int seed);

    double field_0[5];
    int min;
    int range;
};
#pragma pack(pop)

extern struct Random *pRnd;  // idb
