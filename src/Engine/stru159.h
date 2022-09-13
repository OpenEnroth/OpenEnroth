#pragma once

#include "Engine/Events2D.h"

/*  152 */
// Originally was a packed struct.
struct stru159 {
    const char *video_name;
    int field_4;
    int house_npc_id;
    BuildingType uBuildingType; // Originally was 1 byte.
    unsigned __int8 uRoomSoundId;
    unsigned __int16 padding_e;
};

extern std::array<const stru159, 196> pAnimatedRooms;
