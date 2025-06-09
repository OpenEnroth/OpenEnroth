#include "Engine/Graphics/Renderer/Renderer.h"

#include "Engine/Engine.h"

#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Random/Random.h"

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

        Screen_Coord[i].x += vrng->random(base) - base / 2;
        Screen_Coord[i].y += vrng->random(size) + size;
        if (Screen_Coord[i].x < pViewport->viewportTL_X) {
            Screen_Coord[i].x = pViewport->viewportTL_X + vrng->random(base);
        } else if (Screen_Coord[i].x >= (pViewport->viewportBR_X - size)) {
            Screen_Coord[i].x = pViewport->viewportBR_X - vrng->random(base);
        }
        if (Screen_Coord[i].y >= (pViewport->viewportBR_Y - size)) {
            Screen_Coord[i].y = pViewport->viewportTL_Y;
            Screen_Coord[i].x = pViewport->viewportTL_X + vrng->random(pViewport->viewportBR_X - pViewport->viewportTL_X - size);
        }

        render->FillRectFast(Screen_Coord[i].x, Screen_Coord[i].y, size, size, colorTable.White);
    }
}

void Weather::Initialize() {
    int width = pViewport->viewportBR_X - pViewport->viewportTL_X;
    int height = pViewport->viewportBR_Y - pViewport->viewportTL_Y;
    for (Pointi &point : Screen_Coord) {
        point.x = pViewport->viewportTL_X + vrng->random(width);
        point.y = pViewport->viewportTL_Y + vrng->random(height);
    }
}

void Weather::Draw() {
    if (bRenderSnow && engine->config->graphics.Snow.value()) {
        DrawSnow();
    }
}

bool Weather::OnPlayerTurn(int dangle) {
    if (!bRenderSnow) {
        return false;
    }

    unsigned int screen_width = pViewport->viewportBR_X - pViewport->viewportTL_X;

    for (Pointi &point : Screen_Coord) {
        point.x += dangle;
        if (point.x < pViewport->viewportBR_X) {
            if (point.x >= pViewport->viewportTL_X) {
                continue;
            }
            point.x += screen_width;
        } else {
            point.x -= screen_width;
        }
    }

    return true;
}
