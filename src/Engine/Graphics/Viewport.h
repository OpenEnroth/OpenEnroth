#pragma once

#include <cstdint>

#include "Library/Color/Color.h"

class GraphicsImage;

struct Viewport {
    void SetViewport(int topLeft_X, int topLeft_Y, int bottomRight_X, int bottomRight_Y);
    bool Contains(unsigned int x, unsigned int y);

    int viewportTL_X;
    int viewportTL_Y;
    int viewportBR_X;
    int viewportBR_Y;

    int viewportWidth;
    int viewportHeight;
    int viewportCenterX;
    int viewportCenterY;
};

extern Viewport *pViewport;

struct ViewingParams {
    inline ViewingParams() {
        draw_sw_outlines = false;
        draw_d3d_outlines = false;
    }

    void CenterOnPartyZoomIn();
    void InitGrayPalette();
    void MapViewUp();
    void MapViewLeft();
    void MapViewDown();
    void MapViewRight();
    void CenterOnPartyZoomOut();
    void ClampMapViewPosition();
    Sizei GetMapViewMinMaxOffset();
    Sizei GetMapViewMinMaxX();
	Sizei GetMapViewMinMaxY();
    void _443365(); // Sets indoor_center and minimap zoom based on level type.

    unsigned int uMinimapZoom = 0;
    unsigned int uMapBookMapZoom = 0;
    int sViewCenterX = 0;
    int sViewCenterY = 0;
    int16_t indoor_center_x = 0;
    int16_t indoor_center_y = 0;
    GraphicsImage *location_minimap = nullptr;  // unsigned int uTextureID_LocationMap; ::40
    int draw_sw_outlines = 0;
    int draw_d3d_outlines = 0;
    Color pPalette[256] {};
};

extern ViewingParams *viewparams;
