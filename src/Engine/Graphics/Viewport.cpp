#include "Viewport.h"

#include <algorithm>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Party.h"

Recti pViewport;
std::unique_ptr<ViewingParams> viewparams;

//----- (00443219) --------------------------------------------------------
void ViewingParams::MapViewUp() {
    this->sViewCenterY += 512;
    ClampMapViewPosition();
}

//----- (00443225) --------------------------------------------------------
void ViewingParams::MapViewLeft() {
    this->sViewCenterX -= 512;
    ClampMapViewPosition();
}

//----- (00443231) --------------------------------------------------------
void ViewingParams::MapViewDown() {
    this->sViewCenterY -= 512;
    ClampMapViewPosition();
}

//----- (0044323D) --------------------------------------------------------
void ViewingParams::MapViewRight() {
    this->sViewCenterX += 512;
    ClampMapViewPosition();
}

//----- (00443249) --------------------------------------------------------
void ViewingParams::CenterOnPartyZoomOut() {
    this->uMapBookMapZoom /= 2;
    if (this->uMapBookMapZoom < 384) this->uMapBookMapZoom = 384;

    this->sViewCenterX = pParty->pos.x;
    this->sViewCenterY = pParty->pos.y;
    ClampMapViewPosition();
}

//----- (00443291) --------------------------------------------------------
void ViewingParams::CenterOnPartyZoomIn() {
    int MaxZoom;

    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        MaxZoom = 1536;
    } else {
        assert(uCurrentlyLoadedLevelType == LEVEL_INDOOR);
        MaxZoom = 3072;
    }

    this->uMapBookMapZoom *= 2;
    if (this->uMapBookMapZoom > MaxZoom) this->uMapBookMapZoom = MaxZoom;

    this->sViewCenterX = pParty->pos.x;
    this->sViewCenterY = pParty->pos.y;
    ClampMapViewPosition();
}

//----- (004432E7) --------------------------------------------------------
void ViewingParams::ClampMapViewPosition() {
    auto [xMin, xMax] = GetMapViewMinMaxX();
    auto [yMin, yMax] = GetMapViewMinMaxY();
    this->sViewCenterX = std::clamp(this->sViewCenterX, xMin, xMax);
    this->sViewCenterY = std::clamp(this->sViewCenterY, yMin, yMax);
}

Sizei ViewingParams::GetMapViewMinMaxOffset() {
    int mapScale = 88 >> (this->uMapBookMapZoom / 384);
    int minOffset = (mapScale - 44) * 512;
    int maxOffset = (44 - mapScale) * 512;
    return { minOffset, maxOffset };
}

Sizei ViewingParams::GetMapViewMinMaxX() {
    auto [minOffset, maxOffset] = GetMapViewMinMaxOffset();
    return { this->indoor_center_x + minOffset, this->indoor_center_x + maxOffset };
}

Sizei ViewingParams::GetMapViewMinMaxY() {
    auto [minOffset, maxOffset] = GetMapViewMinMaxOffset();
    return { this->indoor_center_y + minOffset, this->indoor_center_y + maxOffset };
}

//----- (00443343) --------------------------------------------------------
void ViewingParams::InitGrayPalette() {
    for (unsigned short i = 0; i < 256; ++i) pPalette[i] = Color(i, i, i);
}

//----- (00443365) --------------------------------------------------------
void ViewingParams::_443365() {
    Vec3f *v3;  // eax@4
    Vec3f *v6;  // eax@12
    int minimum_y;    // [sp+10h] [bp-10h]@2
    int maximum_y;    // [sp+14h] [bp-Ch]@2
    int minimum_x;    // [sp+18h] [bp-8h]@2
    int maximum_x;    // [sp+1Ch] [bp-4h]@2

    InitGrayPalette();
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        minimum_x = 0x40000000;
        minimum_y = 0x40000000;

        maximum_x = -0x40000000;
        maximum_y = -0x40000000;
        for (int i = 0; i < pIndoor->mapOutlines.size(); ++i) {
            v3 = &pIndoor->vertices[pIndoor->mapOutlines[i].uVertex1ID];

            if (v3->x < minimum_x) minimum_x = v3->x;
            if (v3->x > maximum_x) maximum_x = v3->x;
            if (v3->y < minimum_y) minimum_y = v3->x;
            if (v3->y > maximum_y) maximum_y = v3->x;

            v6 = &pIndoor->vertices[pIndoor->mapOutlines[i].uVertex2ID];

            if (v6->x < minimum_x) minimum_x = v3->x;
            if (v6->x > maximum_x) maximum_x = v3->x;

            if (v6->y < minimum_y) minimum_y = v3->y;
            if (v6->y > maximum_y) maximum_y = v3->y;
        }

        uMinimapZoom = engine->config->settings.MinimapZoomIndoor.value();
        indoor_center_x = (signed int)(minimum_x + maximum_x) / 2;
        indoor_center_y = (signed int)(minimum_y + maximum_y) / 2;
    } else {
        indoor_center_x = 0;
        indoor_center_y = 0;
        uMinimapZoom = engine->config->settings.MinimapZoomOutdoor.value();
    }
    uMapBookMapZoom = 384;
}
