#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "Engine/Events/EventEnums.h"

#define EVT_ID_MARGARETH_DIALOGUE_LINE_1    7
#define EVT_ID_MARGARETH_DIALOGUE_LINE_2    9
#define EVT_ID_MARGARETH_DIALOGUE_LINE_3    43
#define EVT_ID_JOIN_AIR_GUILD_DIALOGUE      402


/*  167 */
#pragma pack(push, 1)
struct EventIndex {
    int event_id;
    int event_step;
    unsigned int uEventOffsetInEVT;
};
#pragma pack(pop)

// raw event struct
// header
//  size_event 1 header+data
//  event id  2 3
// event_sequence_num
// event data
#pragma pack(push, 1)
struct _evt_raw {
    unsigned char _e_size;
    unsigned char v1;
    unsigned char v2;
    unsigned char v3;

    EventType _e_type;
    unsigned char v5;
    unsigned char v6;
    unsigned char v7;
    unsigned char v8;
    unsigned char v9;
    unsigned char v10;
    unsigned char v11;
    unsigned char v12;
    unsigned char v13;
    unsigned char v14;
    unsigned char v15;
    unsigned char v16;
    unsigned char v17;
    unsigned char v18;
    unsigned char v19;
    unsigned char v20;
    unsigned char v21;
    unsigned char v22;
    unsigned char v23;
    unsigned char v24;
    unsigned char v25;
    unsigned char v26;
    unsigned char v27;
    unsigned char v28;
    unsigned char v29;
    unsigned char v30;
    unsigned char v31;
    unsigned char v32;
    unsigned char tail[128];
};
#pragma pack(pop)

#define EVT_WORD(x) *(uint16_t *)&x
#define EVT_DWORD(x) *(unsigned int *)&x
#define EVT_STRING(x) (unsigned char *)&x
#define EVT_BYTE(x) (unsigned char)x

extern std::array<unsigned int, 500> pLevelStrOffsets;
extern unsigned int uLevelStrNumStrings;
extern unsigned int uLevelStrFileSize;
extern std::array<char, 9216> pLevelStr;

extern int savedEventID;
extern int savedEventStep;
extern struct LevelDecoration *savedDecoration;

/*  312 */
#pragma pack(push, 1)
struct ByteArray {
    char f[65535];
};
#pragma pack(pop)

struct MapEventVariables {
    std::array<unsigned char, 75> mapVars;
    std::array<unsigned char, 125> decorVars;
};

extern MapEventVariables mapEventVariables;
