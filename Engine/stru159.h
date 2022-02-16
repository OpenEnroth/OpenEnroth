#pragma once

/*  152 */
#pragma pack(push, 1)
struct stru159 {
    const char *video_name;
    int field_4;
    int house_npc_id;
    unsigned __int8 uBuildingType;  // enum BuildingType
    unsigned __int8 uRoomSoundId;
    unsigned __int16 padding_e;
};
#pragma pack(pop)

extern std::array<const stru159, 196> pAnimatedRooms;
