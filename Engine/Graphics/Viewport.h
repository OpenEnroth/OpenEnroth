#pragma once

#include <cstdint>

/*  149 */
#pragma pack(push, 1)
struct Viewport {
    inline Viewport() {
        field_of_view = 65536 / 2;
        SetScreen(0, 0, 639, 479);
    }

    void SetScreen(int uX, int uY, int uZ, int uW);
    void SetFOV(float fov);
    void SetViewport(int uX, int uY, int uZ, int uW);
    bool Contains(unsigned int x, unsigned int y);

    int uScreen_TL_X;
    int uScreen_TL_Y;
    int uScreen_BR_X;
    int uScreen_BR_Y;
    int uViewportTL_X;
    int uViewportTL_Y;
    int uViewportBR_X;
    int uViewportBR_Y;
    int uScreenWidth;
    int uScreenHeight;
    int uScreenCenterX;
    int uScreenCenterY;
    int field_of_view;
};
#pragma pack(pop)

extern struct Viewport *pViewport;

/*  201 */
#pragma pack(push, 1)
struct ViewingParams {
    inline ViewingParams() {
        draw_sw_outlines = false;
        draw_d3d_outlines = false;
        field_4C = 0;
    }

    void CenterOnPartyZoomIn();
    void InitGrayPalette();
    void MapViewUp();
    void MapViewLeft();
    void MapViewDown();
    void MapViewRight();
    void CenterOnPartyZoomOut();
    void AdjustPosition();
    void _443365();

    int uSomeX;  // game viewport co ords
    int uSomeY;
    int uSomeZ;
    int uSomeW;

    unsigned int uScreen_topL_X;
    unsigned int uScreen_topL_Y;
    unsigned int uScreen_BttmR_X;
    unsigned int uScreen_BttmR_Y;
    int field_20;
    unsigned int uMinimapZoom;
    int field_28;
    unsigned int uMapBookMapZoom;
    int sViewCenterX;
    int sViewCenterY;
    int16_t indoor_center_x;
    int16_t indoor_center_y;
    int field_3C;
    class Image *location_minimap;  // unsigned int uTextureID_LocationMap; ::40
    int bRedrawGameUI;
    int field_48;
    int field_4C;
    int draw_sw_outlines;
    int draw_d3d_outlines;
    int field_58;
    int field_5C;
    int field_60;
    int field_64;
    int field_68;
    uint16_t pPalette[256];
};
#pragma pack(pop)

extern struct ViewingParams *viewparams;
