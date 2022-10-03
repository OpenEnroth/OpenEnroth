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

// Partially defined types:
#define _BYTE uint8_t
#define _WORD uint16_t
#define _DWORD uint32_t
#define _QWORD uint64_t

#define HEXRAYS_LOBYTE(x) (*((_BYTE *)&(x)))    // low byte
#define HEXRAYS_LOWORD(x) (*((_WORD *)&(x)))    // low word
#define HEXRAYS_LODWORD(x) (*((_DWORD *)&(x)))  // low dword
#define HEXRAYS_HIBYTE(x) (*((_BYTE *)&(x) + 1))
#define HEXRAYS_HIWORD(x) (*((_WORD *)&(x) + 1))
#define HEXRAYS_HIDWORD(x) (*((_DWORD *)&(x) + 1))
#define BYTEn(x, n) (*((_BYTE *)&(x) + n))
#define WORDn(x, n) (*((_WORD *)&(x) + n))
#define BYTE1(x) BYTEn(x, 1)  // byte 1 (counting from 0)
#define BYTE2(x) BYTEn(x, 2)
#define BYTE3(x) BYTEn(x, 3)
#define BYTE4(x) BYTEn(x, 4)
#define BYTE5(x) BYTEn(x, 5)
#define BYTE6(x) BYTEn(x, 6)
#define BYTE7(x) BYTEn(x, 7)
#define BYTE8(x) BYTEn(x, 8)
#define BYTE9(x) BYTEn(x, 9)
#define BYTE10(x) BYTEn(x, 10)
#define BYTE11(x) BYTEn(x, 11)
#define BYTE12(x) BYTEn(x, 12)
#define BYTE13(x) BYTEn(x, 13)
#define BYTE14(x) BYTEn(x, 14)
#define BYTE15(x) BYTEn(x, 15)
#define WORD1(x) WORDn(x, 1)
#define WORD2(x) WORDn(x, 2)  // third word of the object, unsigned
#define WORD3(x) WORDn(x, 3)
#define WORD4(x) WORDn(x, 4)
#define WORD5(x) WORDn(x, 5)
#define WORD6(x) WORDn(x, 6)
#define WORD7(x) WORDn(x, 7)

// now signed macros (the same but with sign extension)
#define HEXRAYS_SLOBYTE(x) (*((int8_t *)&(x)))
#define HEXRAYS_SLOWORD(x) (*((int16_t *)&(x)))
#define HEXRAYS_SLODWORD(x) (*((int32_t *)&(x)))
#define HEXRAYS_SHIBYTE(x) (*((int8_t *)&(x) + 1))
#define HEXRAYS_SHIWORD(x) (*((int16_t *)&(x) + 1))
#define HEXRAYS_SHIDWORD(x) (*((int32_t *)&(x) + 1))
#define SBYTEn(x, n) (*((int8_t *)&(x) + n))
#define SWORDn(x, n) (*((int16_t *)&(x) + n))
#define SBYTE1(x) SBYTEn(x, 1)
#define SBYTE2(x) SBYTEn(x, 2)
#define SBYTE3(x) SBYTEn(x, 3)
#define SBYTE4(x) SBYTEn(x, 4)
#define SBYTE5(x) SBYTEn(x, 5)
#define SBYTE6(x) SBYTEn(x, 6)
#define SBYTE7(x) SBYTEn(x, 7)
#define SBYTE8(x) SBYTEn(x, 8)
#define SBYTE9(x) SBYTEn(x, 9)
#define SBYTE10(x) SBYTEn(x, 10)
#define SBYTE11(x) SBYTEn(x, 11)
#define SBYTE12(x) SBYTEn(x, 12)
#define SBYTE13(x) SBYTEn(x, 13)
#define SBYTE14(x) SBYTEn(x, 14)
#define SBYTE15(x) SBYTEn(x, 15)
#define SWORD1(x) SWORDn(x, 1)
#define SWORD2(x) SWORDn(x, 2)
#define SWORD3(x) SWORDn(x, 3)
#define SWORD4(x) SWORDn(x, 4)
#define SWORD5(x) SWORDn(x, 5)
#define SWORD6(x) SWORDn(x, 6)
#define SWORD7(x) SWORDn(x, 7)


/*  297 */
enum SoundType {
    SOUND_EndTurnBasedMode = 0xCE,
    SOUND_StartTurnBasedMode = 0xCF,
    SOUND_FlipOnExit = 0x4E21,
};

/*  362 */
#pragma pack(push, 1)
struct TravelInfo {
    char uMapID;
    char pSchedule[7];
    char uDaysCount;
    char field_9[3];
    int x;
    int y;
    int z;
    int direction;
    int field_1C;
};
#pragma pack(pop)

/*  376 */
#pragma pack(push, 1)
struct stru336 {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int16_t field_18[480];
    int16_t field_3D8[480];
};
#pragma pack(pop)
