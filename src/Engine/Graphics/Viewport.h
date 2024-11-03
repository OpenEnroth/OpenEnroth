#pragma once

#include <cstdint>

#include "Library/Color/Color.h"

class GraphicsImage;

// TODO(pskelton): combined viewport and viewing params?? or at least not have multiple places
// where screen coords and viewport coords are held

struct Viewport {
    inline Viewport() {
        SetScreen(0, 0, 639, 479);
    }

    void ResetScreen();
    void SetScreen(int uX, int uY, int uZ, int uW);
    void SetViewport(int uX, int uY, int uZ, int uW);
    bool Contains(unsigned int x, unsigned int y);

    int uScreen_TL_X;  // 0
    int uScreen_TL_Y;  // 0
    int uScreen_BR_X;  // 639
    int uScreen_BR_Y;  // 479

    int uViewportTL_X;
    int uViewportTL_Y;
    int uViewportBR_X;
    int uViewportBR_Y;

    int uScreenWidth;
    int uScreenHeight;
    int uScreenCenterX;
    int uScreenCenterY;
};

extern Viewport *pViewport;

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

    int uSomeX = 0;  // game screen co ords
    int uSomeY = 0;
    int uSomeZ = 0;
    int uSomeW = 0;

    // viewport coords
    unsigned int uScreen_topL_X = 0; // 8
    unsigned int uScreen_topL_Y = 0; // 8
    unsigned int uScreen_BttmR_X = 0; // 468
    unsigned int uScreen_BttmR_Y = 0;  // 352

    int field_20 = 0;
    unsigned int uMinimapZoom = 0;
    unsigned int uMapBookMapZoom = 0;
    int sViewCenterX = 0;
    int sViewCenterY = 0;
    int16_t indoor_center_x = 0;
    int16_t indoor_center_y = 0;
    int field_3C = 0;
    GraphicsImage *location_minimap = nullptr;  // unsigned int uTextureID_LocationMap; ::40
    int field_4C = 0;
    int draw_sw_outlines = 0;
    int draw_d3d_outlines = 0;
    int field_58 = 0;
    int field_5C = 0;
    int field_60 = 0;
    int field_64 = 0;
    int field_68 = 0;
    Color pPalette[256] {};
};

extern ViewingParams *viewparams;
