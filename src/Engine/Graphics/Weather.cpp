#include "Engine/Graphics/IRender.h"

#include "Engine/Engine.h"

#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Weather.h"

Weather *pWeather = new Weather;

void Weather::DrawSnow() {
    for (unsigned int i = 0; i < Screen_Coord.size(); ++i) {
        int size = 1;
        int base = 3;
        if (i >= 950) {
            size = 4;
            base = 10;
        } else if (i >= 700) {
            size = 2;
            base = 5;
        }

        Screen_Coord[i].x += rand() % base - base / 2;
        Screen_Coord[i].y += rand() % size + size;
        if (Screen_Coord[i].x < pViewport->uViewportTL_X) {
            Screen_Coord[i].x = pViewport->uViewportTL_X + rand() % base;
        } else if (Screen_Coord[i].x >= (pViewport->uViewportBR_X - size)) {
            Screen_Coord[i].x = pViewport->uViewportBR_X - rand() % base;
        }
        if (Screen_Coord[i].y >= (pViewport->uViewportBR_Y - size)) {
            Screen_Coord[i].y = pViewport->uViewportTL_Y;
            Screen_Coord[i].x = pViewport->uViewportTL_X +
                                (rand() % (pViewport->uViewportBR_X -
                                    pViewport->uViewportTL_X - size));
        }

        render->FillRectFast(Screen_Coord[i].x, Screen_Coord[i].y, size, size, 0xFFFF);
    }
}

void Weather::Initialize() {
    int width = pViewport->uViewportBR_X - pViewport->uViewportTL_X;
    int height = pViewport->uViewportBR_Y - pViewport->uViewportTL_Y;
    for (Point &point : Screen_Coord) {
        point.x = pViewport->uViewportTL_X + rand() % width;
        point.y = pViewport->uViewportTL_Y + rand() % height;
    }
}

void Weather::Draw() {
    if (bRenderSnow && engine->config->graphics.Snow.Get()) {
        DrawSnow();
    }
}

bool Weather::OnPlayerTurn(int dangle) {
    if (!bRenderSnow) {
        return false;
    }

    unsigned int screen_width = pViewport->uViewportBR_X - pViewport->uViewportTL_X;

    for (Point &point : Screen_Coord) {
        point.x += dangle;
        if (point.x < pViewport->uViewportBR_X) {
            if (point.x >= pViewport->uViewportTL_X) {
                continue;
            }
            point.x += screen_width;
        } else {
            point.x -= screen_width;
        }
    }

    return true;
}
