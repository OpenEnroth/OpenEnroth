#pragma once

#include <cstdint>

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

// TODO(captainurist): #enum class
enum SoundType {
    SOUND_EndTurnBasedMode = 0xCE,
    SOUND_StartTurnBasedMode = 0xCF,
    SOUND_FlipOnExit = 0x4E21,
};
