#pragma once

#include <cstdint>
#include <array>

/*  282 */
#pragma pack(push, 1)
struct OtherOverlay {
    OtherOverlay();
    void Reset();

    int16_t field_0;
    int16_t field_2;
    int16_t sprite_frame_time;  // __int16 field_4;
    int16_t field_6;
    int16_t screen_space_x;  // __int16 field_8;
    int16_t screen_space_y;  // __int16 field_A;
    int16_t field_C;
    int16_t field_E;
    int field_10;
};
#pragma pack(pop)

/*   63 */
#pragma pack(push, 1)
struct OtherOverlayList {
    inline OtherOverlayList()
        :  //----- (0045848D)
          field_3E8(0),
          bRedraw(false) {}

    void Reset();
    int _4418B1(int a2, int a3, int a4, int a5);
    int _4418B6(int uOverlayID, int16_t a3, int a4, int a5, int16_t a6);
    void DrawTurnBasedIcon(int a2);

    std::array<OtherOverlay, 50> pOverlays;
    int field_3E8;
    int bRedraw;
};
#pragma pack(pop)

/*   52 */
#pragma pack(push, 1)
struct OverlayDesc {
    uint16_t uOverlayID;
    uint16_t uOverlayType;
    uint16_t uSpriteFramesetID;
    int16_t field_6;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct OverlayList {
    inline OverlayList()
        :  //----- (00458474)
          uNumOverlays(0),
          pOverlays(nullptr) {}

    void ToFile();
    void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
    bool FromFileTxt(const char *Args);
    void InitializeSprites();

    unsigned int uNumOverlays;
    struct OverlayDesc *pOverlays;
};
#pragma pack(pop)

extern struct OtherOverlayList *pOtherOverlayList;  // idb
extern struct OverlayList *pOverlayList;
