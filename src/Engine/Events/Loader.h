#pragma once

#include <stdint.h>
#include <string>
#include "Engine/Events/EventEnums.h"

enum class EventType : uint8_t;

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

#define EVT_WORD(x) *(uint16_t *)&x
#define EVT_DWORD(x) *(unsigned int *)&x
#define EVT_STRING(x) (unsigned char *)&x
#define EVT_BYTE(x) (unsigned char)x

/**
 * @offset 0x443DA1
 */
void initGlobalEvents();

/**
 * @offset 0x443EF8
 */
void initLocalEvents(const std::string &mapName);
