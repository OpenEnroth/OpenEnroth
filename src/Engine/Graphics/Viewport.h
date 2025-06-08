#pragma once

#include <cstdint>

#include "Library/Color/Color.h"

class GraphicsImage;

// TODO(pskelton): combined viewport and viewing params?? or at least not have multiple places
// where screen coords and viewport coords are held

struct Viewport {
    void SetViewport(int uX, int uY, int uZ, int uW);
    bool Contains(unsigned int x, unsigned int y);

    int uViewportTL_X;
    int uViewportTL_Y;
    int uViewportBR_X;
    int uViewportBR_Y;

    int uViewportWidth;
    int uViewportHeight;
    int uViewportCenterX;
    int uViewportCenterY;
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
