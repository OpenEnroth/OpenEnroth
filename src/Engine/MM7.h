#pragma once

#include <cstdint>

#define PID(type, id) (uint16_t)((((8 * (id))) | (std::to_underlying(type))) & 0xFFFF)  // packed id
#define PID_TYPE(pid) static_cast<ObjectType>((pid)&7)          // extract type
#define PID_ID(pid) (uint32_t)(((pid)&0xFFFF) >> 3)  // extract value
#define PID_INVALID (uint16_t)(-1)

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

/*  297 */
enum SoundType {
    SOUND_EndTurnBasedMode = 0xCE,
    SOUND_StartTurnBasedMode = 0xCF,
    SOUND_FlipOnExit = 0x4E21,
};
