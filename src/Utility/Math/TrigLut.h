#pragma once

#include <array>

/**
 * Lookup table for trigonometric functions.
 */
class TrigTableLookup {
 public:
    static const int uIntegerPi = 1024;
    static const int uIntegerHalfPi = 512;
    static const int uIntegerQuarterPi = 256;
    static const int uIntegerDoublePi = 2048;
    static const int uDoublePiMask = 2047;
    static const int uPiMask = 1023;
    static const int uHalfPiMask = 511;

    /**
     * @offset 0x00452969
     */
    TrigTableLookup();

    /**
     * @offset 0x00402CAE
     *
     * @param angle                     Angle in 1/2048ths of a full circle.
     * @return                          Cosine of the provided angle.
     */
    float cos(int angle) const;

    /**
     * @offset 0x0042EBDB
     *
     * @param angle                     Angle in 1/2048ths of a full circle.
     * @return                          Sine of the provided angle.
     */
    float sin(int angle) const;

    /**
     * @offset 0x0045281E
     *
     * @return                          Angle in 1/2048ths of a full circle. Actual result is in range [0, 2047].
     */
    int atan2(int x, int y) const;

 private:
    std::array<float, uIntegerHalfPi + 1> _cosTable;
};

extern TrigTableLookup TrigLUT;
