#pragma once
#include <array>

typedef unsigned int uint;

#define PID(type, id) (uint32_t)((((8 * (id))) | (type)) & 0xFFFF)  // packed id
#define PID_TYPE(pid) (ObjectType)((pid)&7)          // extract type
#define PID_ID(pid) (uint32_t)(((pid)&0xFFFF) >> 3)  // extract value
#define PID_INVALID (-1)

// typedef char _UNKNOWN;
typedef unsigned int uint;

inline void memset32(void *ptr, uint32_t value, int count) {
    uint32_t *p = (uint32_t *)ptr;
    for (int i = 0; i < count; i++) {
        *p++ = value;
    }
}

#if defined(__GNUC__)
typedef long long ll;
typedef unsigned long long ull;
#define __int64 long long
#define __int32 int
#define __int16 short
#define __int8 char
#define MAKELL(num) num##LL
#define FMT_64 "ll"
#elif defined(_MSC_VER)
typedef __int64 ll;
typedef unsigned __int64 ull;
#define MAKELL(num) num##i64
#define FMT_64 "I64"
#elif defined(__BORLANDC__)
typedef __int64 ll;
typedef unsigned __int64 ull;
#define MAKELL(num) num##i64
#define FMT_64 "L"
#else
#error "unknown compiler"
#endif
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef char int8;
typedef signed char sint8;
typedef unsigned char uint8;
typedef short int16;
typedef signed short sint16;
typedef unsigned short uint16;
typedef int int32;
typedef signed int sint32;
typedef unsigned int uint32;
typedef ll int64;
typedef ll sint64;
typedef ull uint64;

// Partially defined types:
#define _BYTE uint8
#define _WORD uint16
#define _DWORD uint32
#define _QWORD uint64
#if !defined(_MSC_VER)
#define _LONGLONG __int128
#endif

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
#define HEXRAYS_SLOBYTE(x) (*((int8 *)&(x)))
#define HEXRAYS_SLOWORD(x) (*((int16 *)&(x)))
#define HEXRAYS_SLODWORD(x) (*((int32 *)&(x)))
#define HEXRAYS_SHIBYTE(x) (*((int8 *)&(x) + 1))
#define HEXRAYS_SHIWORD(x) (*((int16 *)&(x) + 1))
#define HEXRAYS_SHIDWORD(x) (*((int32 *)&(x) + 1))
#define SBYTEn(x, n) (*((int8 *)&(x) + n))
#define SWORDn(x, n) (*((int16 *)&(x) + n))
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

// Generate a reference to pair of operands
template <class T>
int16 __PAIR__(int8 high, T low) {
    return (((int16)high) << sizeof(high) * 8) | uint8(low);
}
template <class T>
int32 __PAIR__(int16 high, T low) {
    return (((int32)high) << sizeof(high) * 8) | uint16(low);
}
template <class T>
int64 __PAIR__(int32 high, T low) {
    return (((int64)high) << sizeof(high) * 8) | uint32(low);
}
template <class T>
uint16 __PAIR__(uint8 high, T low) {
    return (((uint16)high) << sizeof(high) * 8) | uint8(low);
}
template <class T>
uint32 __PAIR__(uint16 high, T low) {
    return (((uint32)high) << sizeof(high) * 8) | uint16(low);
}
template <class T>
uint64 __PAIR__(uint32 high, T low) {
    return (((uint64)high) << sizeof(high) * 8) | uint32(low);
}

// rotate left
template <class T>
T __ROL__(T value, uint count) {
    const uint nbits = sizeof(T) * 8;
    count %= nbits;

    T high = value >> (nbits - count);
    value <<= count;
    value |= high;
    return value;
}

// rotate right
template <class T>
T __ROR__(T value, uint count) {
    const uint nbits = sizeof(T) * 8;
    count %= nbits;

    T low = value << (nbits - count);
    value >>= count;
    value |= low;
    return value;
}

// carry flag of left shift
template <class T>
int8 __MKCSHL__(T value, uint count) {
    const uint nbits = sizeof(T) * 8;
    count %= nbits;

    return (value >> (nbits - count)) & 1;
}

// carry flag of right shift
template <class T>
int8 __MKCSHR__(T value, uint count) {
    return (value >> (count - 1)) & 1;
}

// sign flag
template <class T>
int8 __SETS__(T x) {
    if (sizeof(T) == 1) return int8(x) < 0;
    if (sizeof(T) == 2) return int16(x) < 0;
    if (sizeof(T) == 4) return int32(x) < 0;
    return int64(x) < 0;
}

// overflow flag of subtraction (x-y)
template <class T, class U>
int8 __OFSUB__(T x, U y) {
    if (sizeof(T) < sizeof(U)) {
        U x2 = x;
        int8 sx = __SETS__(x2);
        return (sx ^ __SETS__(y)) & (sx ^ __SETS__(x2 - y));
    } else {
        T y2 = y;
        int8 sx = __SETS__(x);
        return (sx ^ __SETS__(y2)) & (sx ^ __SETS__(x - y2));
    }
}

// overflow flag of addition (x+y)
template <class T, class U>
int8 __OFADD__(T x, U y) {
    if (sizeof(T) < sizeof(U)) {
        U x2 = x;
        int8 sx = __SETS__(x2);
        return ((1 ^ sx) ^ __SETS__(y)) & (sx ^ __SETS__(x2 + y));
    } else {
        T y2 = y;
        int8 sx = __SETS__(x);
        return ((1 ^ sx) ^ __SETS__(y2)) & (sx ^ __SETS__(x + y2));
    }
}

// carry flag of subtraction (x-y)
template <class T, class U>
int8 __CFSUB__(T x, U y) {
    int size = sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U);
    if (size == 1) return uint8(x) < uint8(y);
    if (size == 2) return uint16(x) < uint16(y);
    if (size == 4) return uint32(x) < uint32(y);
    return uint64(x) < uint64(y);
}

// carry flag of addition (x+y)
template <class T, class U>
int8 __CFADD__(T x, U y) {
    int size = sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U);
    if (size == 1) return uint8(x) > uint8(x + y);
    if (size == 2) return uint16(x) > uint16(x + y);
    if (size == 4) return uint32(x) > uint32(x + y);
    return uint64(x) > uint64(x + y);
}

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

/*  374 */
#pragma pack(push, 1)
struct stat_coord {
    __int16 x;
    __int16 y;
    __int16 width;
    __int16 height;
};
#pragma pack(pop)
extern std::array<stat_coord, 26> stat_string_coord;

/*  376 */
#pragma pack(push, 1)
struct stru336 {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    __int16 field_18[480];
    __int16 field_3D8[480];
};
#pragma pack(pop)
