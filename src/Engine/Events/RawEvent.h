#pragma once

#include <cstdint>

#include "Engine/Events/EventEnums.h"

#include "Utility/Unaligned.h"

struct RawEvent {
    uint8_t _e_size; // Size of the data that follows.
    uint8_t v1; // Event id LSB.
    uint8_t v2; // Event id MSB.
    uint8_t v3; // Step.

    EventType _e_type;
    uint8_t v5;
    uint8_t v6;
    uint8_t v7;
    uint8_t v8;
    uint8_t v9;
    uint8_t v10;
    uint8_t v11;
    uint8_t v12;
    uint8_t v13;
    uint8_t v14;
    uint8_t v15;
    uint8_t v16;
    uint8_t v17;
    uint8_t v18;
    uint8_t v19;
    uint8_t v20;
    uint8_t v21;
    uint8_t v22;
    uint8_t v23;
    uint8_t v24;
    uint8_t v25;
    uint8_t v26;
    uint8_t v27;
    uint8_t v28;
    uint8_t v29;
    uint8_t v30;
    uint8_t v31;
    uint8_t v32;
    uint8_t tail[128];
};
static_assert(alignof(RawEvent) == 1);

#define EVT_BYTE(x) (x)
#define EVT_WORD(x) (readUnaligned<uint16_t>(x))
#define EVT_DWORD(x) (readUnaligned<uint32_t>(x))
