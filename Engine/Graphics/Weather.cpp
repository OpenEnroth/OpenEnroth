#include "Engine/Graphics/IRender.h"

#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/Viewport.h"




struct Weather *pWeather = new Weather;



//----- (004C2AA6) --------------------------------------------------------
int Weather::DrawSnow()
{
    for (unsigned int i = 0; i < 700; ++i)
    {
        int x = 2 * i;
        int y = 2 * i + 1;
        this->Screen_Coord[x] += rand() % 3 + 1;
        this->Screen_Coord[y] += rand() % 2 + 1;
        if (this->Screen_Coord[x] < (signed int)(viewparams->uScreen_BttmR_X - 1))//467
        {
            if (this->Screen_Coord[x] < (signed int)viewparams->uScreen_topL_X)//8
                this->Screen_Coord[x] = viewparams->uScreen_BttmR_X - rand() % 8;
        }
        else
            this->Screen_Coord[x] = viewparams->uScreen_topL_X + rand() % 8;
        if (this->Screen_Coord[y] < (signed int)viewparams->uScreen_topL_Y//8
            || this->Screen_Coord[y] >= (signed int)viewparams->uScreen_BttmR_Y)//351
        {
            this->Screen_Coord[y] = viewparams->uScreen_topL_Y;
            this->Screen_Coord[x] = viewparams->uScreen_topL_X + (rand() % (signed int)((viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X) - 2));
        }
        //v1[this->Screen_Coord[2 * i] + 640 * this->Screen_Coord[2 * i + 1]] = 0xFFFFu;
        render->WritePixel16(this->Screen_Coord[x], this->Screen_Coord[y], 0xFFFF);//snowflake - point(снежинка - точка)
    }

    for (unsigned int i = 700; i < 950; ++i)
    {
        int x = 2 * i;
        int y = 2 * i + 1;
        this->Screen_Coord[x] += rand() % 5 - 3;//x
        this->Screen_Coord[y] += 4;//y
        if (this->Screen_Coord[x] < (signed int)viewparams->uScreen_topL_X)
            this->Screen_Coord[x] = viewparams->uScreen_BttmR_X - rand() % 4 - 2;
        if (this->Screen_Coord[x] >= (signed int)(viewparams->uScreen_BttmR_X - 2))
            this->Screen_Coord[x] = rand() % 4 + viewparams->uScreen_topL_X + 2;
        if (this->Screen_Coord[y] < (signed int)viewparams->uScreen_topL_Y || this->Screen_Coord[y] >= (signed int)(viewparams->uScreen_BttmR_Y - 1))
        {
            this->Screen_Coord[y] = viewparams->uScreen_topL_Y;
            this->Screen_Coord[x] = viewparams->uScreen_topL_X + (rand() % (signed int)((viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X) - 2));
        }
        render->WritePixel16(this->Screen_Coord[x], this->Screen_Coord[y], 0xFFFF);//x, y  квадратная снежинка)
        render->WritePixel16(this->Screen_Coord[x] + 1, this->Screen_Coord[y], 0xFFFF);//x + 1, y
        render->WritePixel16(this->Screen_Coord[x], this->Screen_Coord[y] + 1, 0xFFFF);//x , y + 1
        render->WritePixel16(this->Screen_Coord[x] + 1, this->Screen_Coord[y] + 1, 0xFFFF);//x + 1, y + 1

        //v1[this->Screen_Coord[2 * i] + 640 * this->Screen_Coord[2 * i + 1]] = 0xFFFFu;
        //v1[this->Screen_Coord[2 * i] + 640 * this->Screen_Coord[2 * i + 1] + 1] = 0xFFFFu;
        //v1[this->Screen_Coord[2 * i] + 640 * (this->Screen_Coord[2 * i + 1] + 1)] = 0xFFFFu;
        //v1[this->Screen_Coord[2 * i] + 640 * this->Screen_Coord[2 * i + 1] + 641] = 0xFFFFu;
    }

    for (unsigned int i = 0; i < 50; i++)
    {
        this->Screen_Coord[1901 + (i * 2)] += 8;
        this->Screen_Coord[1901 + ((i * 2) - 1)] += rand() % 11 - 5;
        if (this->Screen_Coord[1901 + ((i * 2) - 1)] < (signed int)viewparams->uScreen_topL_X || this->Screen_Coord[1901 + ((i * 2) - 1)] >= (signed int)(viewparams->uScreen_BttmR_X - 5))
        {
            this->Screen_Coord[1901 + (i * 2)] = viewparams->uScreen_topL_Y;
            this->Screen_Coord[1901 + ((i * 2) - 1)] = viewparams->uScreen_topL_X + (rand() % (signed int)((viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X) - 5));
        }
        if (this->Screen_Coord[1901 + (i * 2)] < (signed int)viewparams->uScreen_topL_Y || this->Screen_Coord[1901 + (i * 2)] >= (signed int)(viewparams->uScreen_BttmR_Y - 5))
        {
            this->Screen_Coord[1901 + (i * 2)] = viewparams->uScreen_topL_Y;
            this->Screen_Coord[1901 + ((i * 2) - 1)] = viewparams->uScreen_topL_X + (rand() % (signed int)((viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X) - 5));
        }
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)], this->Screen_Coord[1901 + (i * 2)], 0xFFFF);//x, y
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)], this->Screen_Coord[1901 + (i * 2)] + 1, 0xFFFF);//x, y + 1
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)], this->Screen_Coord[1901 + (i * 2)] + 2, 0xFFFF);//x, y + 2
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)], this->Screen_Coord[1901 + (i * 2)] + 3, 0xFFFF);//x, y + 3

        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 1, this->Screen_Coord[1901 + (i * 2)], 0xFFFF);//x + 1, y
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 1, this->Screen_Coord[1901 + (i * 2)] + 1, 0xFFFF);//x + 1, y + 1
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 1, this->Screen_Coord[1901 + (i * 2)] + 2, 0xFFFF);//x + 1, y + 2
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 1, this->Screen_Coord[1901 + (i * 2)] + 3, 0xFFFF);//x + 1, y + 3

        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 2, this->Screen_Coord[1901 + (i * 2)], 0xFFFF);//x + 2, y
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 2, this->Screen_Coord[1901 + (i * 2)] + 1, 0xFFFF);//x + 2, y + 1
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 2, this->Screen_Coord[1901 + (i * 2)] + 2, 0xFFFF);//x + 2, y + 2
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 2, this->Screen_Coord[1901 + (i * 2)] + 3, 0xFFFF);//x + 2, y + 3

        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 3, this->Screen_Coord[1901 + (i * 2)], 0xFFFF);//x + 3, y
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 3, this->Screen_Coord[1901 + (i * 2)] + 1, 0xFFFF);//x + 3, y + 1
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 3, this->Screen_Coord[1901 + (i * 2)] + 2, 0xFFFF);//x + 3, y + 2
        render->WritePixel16(this->Screen_Coord[1901 + ((i * 2) - 1)] + 3, this->Screen_Coord[1901 + (i * 2)] + 3, 0xFFFF);//x + 3, y + 3
    }
    return 0;
}

//----- (004C2EA0) --------------------------------------------------------
int Weather::Initialize()
{
    int v3 = pViewport->uScreen_BR_X - pViewport->uScreen_TL_X - 4;
    int v4 = pViewport->uScreen_BR_Y - pViewport->uScreen_TL_Y - 4;
    for (unsigned int i = 0; i < 1000; i++)
    {
        this->Screen_Coord[2 * i] = (unsigned short)pViewport->uViewportTL_X + rand() % v3;
        this->Screen_Coord[(2 * (i + 1)) - 1] = (unsigned short)pViewport->uViewportTL_Y + rand() % v4;
    }
    return 0;
}

//----- (004C2EFA) --------------------------------------------------------
int Weather::Draw()
{
    extern bool bSnow;
    if (bRenderSnow || bSnow)
        DrawSnow();
    return 0;
}

//----- (004C2F0B) --------------------------------------------------------
bool Weather::OnPlayerTurn(__int16 a2)
{
    unsigned int screen_width; // esi@3

    if (this->bRenderSnow != true)
        return 0;
    screen_width = viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X;

    for (unsigned int i = 0; i < 1000; ++i)
    {
        this->Screen_Coord[2 * i] += a2;
        if (this->Screen_Coord[2 * i] < (signed int)viewparams->uScreen_BttmR_X - 4)
        {
            if (this->Screen_Coord[2 * i] >= (signed int)viewparams->uScreen_topL_X)
                continue;
            this->Screen_Coord[2 * i] += screen_width;
        }
        else
            this->Screen_Coord[2 * i] = this->Screen_Coord[2 * i] - screen_width + 4;
    }
    return 1;
}