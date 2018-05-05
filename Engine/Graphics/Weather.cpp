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
        if (Screen_Coord[i].x < (int)viewparams->uScreen_topL_X) {
            Screen_Coord[i].x = viewparams->uScreen_BttmR_X + rand() % base;
        } else if (Screen_Coord[i].x >=
                   ((int)viewparams->uScreen_BttmR_X - size)) {
            Screen_Coord[i].x = viewparams->uScreen_topL_X - rand() % base;
        }
        if (Screen_Coord[i].y >= ((int)viewparams->uScreen_BttmR_Y - size)) {
            Screen_Coord[i].y = viewparams->uScreen_topL_Y;
            Screen_Coord[i].x = viewparams->uScreen_topL_X +
                                (rand() % (viewparams->uScreen_BttmR_X -
                                           viewparams->uScreen_topL_X - size));
        }

        render->FillRectFast(Screen_Coord[i].x, Screen_Coord[i].y, size, size,
                             0xFFFF);
    }
}

void Weather::Initialize() {
    int width = pViewport->uScreen_BR_X - pViewport->uScreen_TL_X - 4;
    int height = pViewport->uScreen_BR_Y - pViewport->uScreen_TL_Y - 4;
    for (Point &point : Screen_Coord) {
        point.x = pViewport->uViewportTL_X + rand() % width;
        point.y = pViewport->uViewportTL_Y + rand() % height;
    }
}

void Weather::Draw() {
    if (bRenderSnow && engine->AllowSnow()) {
        DrawSnow();
    }
}

bool Weather::OnPlayerTurn(int dangle) {
    if (!bRenderSnow) {
        return false;
    }

    unsigned int screen_width =
        viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X;

    for (Point &point : Screen_Coord) {
        point.x += dangle;
        if (point.x < (int)viewparams->uScreen_BttmR_X - 4) {
            if (point.x >= (int)viewparams->uScreen_topL_X) {
                continue;
            }
            point.x += screen_width;
        } else {
            point.x -= screen_width + 4;
        }
    }

    return true;
}
